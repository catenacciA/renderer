#ifndef APPDELEGATE_HPP
#define APPDELEGATE_HPP

#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include "MyMTKViewDelegate.hpp"

class MyAppDelegate : public NS::ApplicationDelegate {
 public:
  MyAppDelegate();
  virtual ~MyAppDelegate();

  NS::Menu* createMenuBar();

  virtual void applicationWillFinishLaunching(
      NS::Notification* pNotification) override;
  virtual void applicationDidFinishLaunching(
      NS::Notification* pNotification) override;
  virtual bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* pSender) override;

 private:
  NS::Window*        _pWindow;
  MTK::View*         _pMtkView;
  MTL::Device*       _pDevice;
  MyMTKViewDelegate* _pViewDelegate;
};

#endif  // APPDELEGATE_HPP
