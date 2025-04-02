#include <cassert>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <AppKit/AppKit.hpp>

#include "AppDelegate.hpp"

int main(int argc, char* argv[]) {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  MyAppDelegate    appDelegate;
  NS::Application* pApp = NS::Application::sharedApplication();
  pApp->setDelegate(&appDelegate);
  pApp->run();

  pAutoreleasePool->release();
  return 0;
}
