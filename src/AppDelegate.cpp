#include "AppDelegate.hpp"

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

MyAppDelegate::MyAppDelegate()
    : _pWindow(nullptr)
    , _pMtkView(nullptr)
    , _pDevice(nullptr)
    , _pViewDelegate(nullptr) {}

MyAppDelegate::~MyAppDelegate() {
  if (_pMtkView) _pMtkView->release();
  if (_pWindow) _pWindow->release();
  if (_pDevice) _pDevice->release();
  delete _pViewDelegate;
}

NS::Menu* MyAppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu*     pMainMenu    = NS::Menu::alloc()->init();
  NS::MenuItem* pAppMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu*     pAppMenu     = NS::Menu::alloc()->init(
      NS::String::string("Appname", UTF8StringEncoding));

  NS::String* appName =
      NS::RunningApplication::currentApplication()->localizedName();
  NS::String* quitItemName = NS::String::string("Quit ", UTF8StringEncoding)
                                 ->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback(
      "appQuit", [](void*, SEL, const NS::Object* pSender) {
        NS::Application::sharedApplication()->terminate(pSender);
      });
  NS::MenuItem* pAppQuitItem = pAppMenu->addItem(
      quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
  pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pAppMenuItem->setSubmenu(pAppMenu);

  NS::MenuItem* pWindowMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu*     pWindowMenu     = NS::Menu::alloc()->init(
      NS::String::string("Window", UTF8StringEncoding));
  SEL closeWindowCb = NS::MenuItem::registerActionCallback(
      "windowClose", [](void*, SEL, const NS::Object*) {
        NS::Application::sharedApplication()
            ->windows()
            ->object<NS::Window>(0)
            ->close();
      });
  NS::MenuItem* pCloseWindowItem = pWindowMenu->addItem(
      NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb,
      NS::String::string("w", UTF8StringEncoding));
  pCloseWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pWindowMenuItem->setSubmenu(pWindowMenu);

  pMainMenu->addItem(pAppMenuItem);
  pMainMenu->addItem(pWindowMenuItem);

  pAppMenuItem->release();
  pWindowMenuItem->release();
  pAppMenu->release();
  pWindowMenu->release();

  return pMainMenu->autorelease();
}

void MyAppDelegate::applicationWillFinishLaunching(
    NS::Notification* pNotification) {
  NS::Menu*        pMenu = createMenuBar();
  NS::Application* pApp  = reinterpret_cast<NS::Application*>(
      pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void MyAppDelegate::applicationDidFinishLaunching(
    NS::Notification* pNotification) {
  CGRect frame = {{100.0, 100.0}, {1024.0, 1024.0}};

  _pWindow = NS::Window::alloc()->init(frame,
      NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
      NS::BackingStoreBuffered, false);

  _pDevice = MTL::CreateSystemDefaultDevice();

  _pMtkView = MTK::View::alloc()->init(frame, _pDevice);
  _pMtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  _pMtkView->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
  _pMtkView->setDepthStencilPixelFormat(
      MTL::PixelFormat::PixelFormatDepth16Unorm);
  _pMtkView->setClearDepth(1.0f);

  _pViewDelegate = new MyMTKViewDelegate(_pDevice);
  _pMtkView->setDelegate(_pViewDelegate);

  _pWindow->setContentView(_pMtkView);
  _pWindow->setTitle(
      NS::String::string("raytracer", NS::StringEncoding::UTF8StringEncoding));
  _pWindow->makeKeyAndOrderFront(nullptr);

  NS::Application* pApp = reinterpret_cast<NS::Application*>(
      pNotification->object());
  pApp->activateIgnoringOtherApps(true);
}

bool MyAppDelegate::applicationShouldTerminateAfterLastWindowClosed(
    NS::Application* /*pSender*/) {
  return true;
}
