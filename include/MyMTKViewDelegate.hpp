#ifndef MYMTKVIEWDELEGATE_HPP
#define MYMTKVIEWDELEGATE_HPP

#include <MetalKit/MetalKit.hpp>

#include "Renderer.hpp"

class MyMTKViewDelegate : public MTK::ViewDelegate {
 public:
  MyMTKViewDelegate(MTL::Device* pDevice);
  virtual ~MyMTKViewDelegate() override;

  virtual void drawInMTKView(MTK::View* pView) override;

 private:
  Renderer* _pRenderer;
};

#endif  // MYMTKVIEWDELEGATE_HPP
