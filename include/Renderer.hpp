#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

static constexpr size_t kInstanceRows    = 10;
static constexpr size_t kInstanceColumns = 10;
static constexpr size_t kInstanceDepth   = 10;
static constexpr size_t kNumInstances =
    (kInstanceRows * kInstanceColumns * kInstanceDepth);
static constexpr size_t kMaxFramesInFlight = 3;

class Renderer {
 public:
  Renderer(MTL::Device* pDevice);
  ~Renderer();
  void buildShaders();
  void buildDepthStencilStates();
  void buildBuffers();
  void draw(MTK::View* pView);

 private:
  MTL::Device*              _pDevice;
  MTL::CommandQueue*        _pCommandQueue;
  MTL::Library*             _pShaderLibrary;
  MTL::RenderPipelineState* _pPSO;
  MTL::DepthStencilState*   _pDepthStencilState;
  MTL::Buffer*              _pVertexDataBuffer;
  MTL::Buffer*              _pInstanceDataBuffer[kMaxFramesInFlight];
  MTL::Buffer*              _pCameraDataBuffer[kMaxFramesInFlight];
  MTL::Buffer*              _pIndexBuffer;
  MTL::Buffer*              _pLightDataBuffer[kMaxFramesInFlight];
  float                     _angle;
  int                       _frame;
  float                     _currentTime;
  dispatch_semaphore_t      _semaphore;
  static const int          kMaxFramesInFlight;
  size_t                    _numIndices;

  void updateLightData(MTL::Buffer* pLightBuffer);
};

#endif  // RENDERER_HPP
