#include "Renderer.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Math.hpp"
#include "Mesh.hpp"

const int Renderer::kMaxFramesInFlight = 3;

Renderer::Renderer(MTL::Device* pDevice)
    : _pDevice(pDevice->retain()), _angle(0.f), _frame(0) {
  _pCommandQueue = _pDevice->newCommandQueue();
  buildShaders();
  buildDepthStencilStates();
  buildBuffers();

  _semaphore = dispatch_semaphore_create(Renderer::kMaxFramesInFlight);
}

Renderer::~Renderer() {
  _pShaderLibrary->release();
  _pDepthStencilState->release();
  _pVertexDataBuffer->release();
  for (int i = 0; i < kMaxFramesInFlight; ++i) {
    _pInstanceDataBuffer[i]->release();
  }
  for (int i = 0; i < kMaxFramesInFlight; ++i) {
    _pCameraDataBuffer[i]->release();
  }
  _pIndexBuffer->release();
  _pPSO->release();
  _pCommandQueue->release();
  _pDevice->release();
}

void Renderer::buildDepthStencilStates() {
  MTL::DepthStencilDescriptor* pDsDesc =
      MTL::DepthStencilDescriptor::alloc()->init();
  pDsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
  pDsDesc->setDepthWriteEnabled(true);

  _pDepthStencilState = _pDevice->newDepthStencilState(pDsDesc);

  pDsDesc->release();
}

void Renderer::buildShaders() {
  using NS::StringEncoding::UTF8StringEncoding;

  std::filesystem::path shaderPath =
      std::filesystem::current_path().parent_path() / "src" / "shader.metal";

  std::string   shaderPathStr = shaderPath.string();
  std::ifstream shaderFile(shaderPathStr);
  if (!shaderFile.is_open()) {
    __builtin_printf(
        "Failed to open Shader.metal at path: %s\n", shaderPathStr.c_str());
    assert(false);
  }

  std::stringstream shaderSrcStream;
  shaderSrcStream << shaderFile.rdbuf();
  shaderFile.close();
  std::string shaderSrcStr = shaderSrcStream.str();
  NS::String* shaderSrc    = NS::String::string(
      shaderSrcStr.c_str(), UTF8StringEncoding);
  if (!shaderSrc) {
    __builtin_printf("Failed to create NS::String from shader source\n");
    assert(false);
  }

  NS::Error*    pError   = nullptr;
  MTL::Library* pLibrary = _pDevice->newLibrary(shaderSrc, nullptr, &pError);
  if (!pLibrary) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  MTL::Function* pVertexFn = pLibrary->newFunction(
      NS::String::string("vertexMain", UTF8StringEncoding));
  MTL::Function* pFragFn = pLibrary->newFunction(
      NS::String::string("fragmentMain", UTF8StringEncoding));

  MTL::RenderPipelineDescriptor* pDesc =
      MTL::RenderPipelineDescriptor::alloc()->init();
  pDesc->setVertexFunction(pVertexFn);
  pDesc->setFragmentFunction(pFragFn);
  pDesc->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  pDesc->setDepthAttachmentPixelFormat(
      MTL::PixelFormat::PixelFormatDepth16Unorm);

  _pPSO = _pDevice->newRenderPipelineState(pDesc, &pError);
  if (!_pPSO) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  _pShaderLibrary = pLibrary;
}

void Renderer::buildBuffers() {
  auto mesh = createMesh(MeshType::Sphere);

  auto vertices = mesh->getVertices();
  auto indices  = mesh->getIndices();

  _numIndices                 = indices.size();
  const size_t vertexDataSize = vertices.size() *
                                sizeof(shader_types::VertexData);
  const size_t indexDataSize = indices.size() * sizeof(uint16_t);

  _pVertexDataBuffer = _pDevice->newBuffer(
      vertexDataSize, MTL::ResourceStorageModeManaged);
  _pIndexBuffer = _pDevice->newBuffer(
      indexDataSize, MTL::ResourceStorageModeManaged);

  memcpy(_pVertexDataBuffer->contents(), vertices.data(), vertexDataSize);
  memcpy(_pIndexBuffer->contents(), indices.data(), indexDataSize);

  _pVertexDataBuffer->didModifyRange(NS::Range::Make(0, vertexDataSize));
  _pIndexBuffer->didModifyRange(NS::Range::Make(0, indexDataSize));

  const size_t instanceDataSize = kMaxFramesInFlight * kNumInstances *
                                  sizeof(shader_types::InstanceData);
  for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
    _pInstanceDataBuffer[i] = _pDevice->newBuffer(
        instanceDataSize, MTL::ResourceStorageModeManaged);
  }

  const size_t cameraDataSize = kMaxFramesInFlight *
                                sizeof(shader_types::CameraData);
  for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
    _pCameraDataBuffer[i] = _pDevice->newBuffer(
        cameraDataSize, MTL::ResourceStorageModeManaged);
  }

  const size_t lightDataSize = kMaxFramesInFlight *
                               sizeof(shader_types::LightData);
  for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
    _pLightDataBuffer[i] = _pDevice->newBuffer(
        lightDataSize, MTL::ResourceStorageModeManaged);
  }
}

void Renderer::updateLightData(MTL::Buffer* pLightBuffer) {
  shader_types::LightData* pLightData =
      reinterpret_cast<shader_types::LightData*>(pLightBuffer->contents());

  pLightData->position = {
      5.0f * sinf(_currentTime), 5.0f, 5.0f * cosf(_currentTime)};
  pLightData->color      = {1.0f, 0.9f, 0.8f};
  pLightData->intensity  = 2.0f;
  pLightData->range      = 30.0f;
  pLightData->pulseSpeed = 2.0f;
  pLightData->time       = _currentTime;

  pLightBuffer->didModifyRange(
      NS::Range::Make(0, sizeof(shader_types::LightData)));
}

void Renderer::draw(MTK::View* pView) {
  using simd::float3;
  using simd::float4;
  using simd::float4x4;

  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  _frame = (_frame + 1) % Renderer::kMaxFramesInFlight;
  MTL::Buffer* pInstanceDataBuffer = _pInstanceDataBuffer[_frame];
  MTL::Buffer* pLightDataBuffer    = _pLightDataBuffer[_frame];

  MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
  Renderer* pRenderer = this;
  pCmd->addCompletedHandler(^void(MTL::CommandBuffer* pCmd) {
    dispatch_semaphore_signal(pRenderer->_semaphore);
  });

  _currentTime += 0.016f;
  _angle += 0.002f;

  const float                 scl = 0.2f;
  shader_types::InstanceData* pInstanceData =
      reinterpret_cast<shader_types::InstanceData*>(
          pInstanceDataBuffer->contents());

  float3 objectPosition = {0.f, 0.f, -10.f};

  float4x4 rt    = Math::makeTranslate(objectPosition);
  float4x4 rr1   = Math::makeYRotate(-_angle);
  float4x4 rr0   = Math::makeXRotate(_angle * 0.5);
  float4x4 rtInv = Math::makeTranslate(
      {-objectPosition.x, -objectPosition.y, -objectPosition.z});
  float4x4 fullObjectRot = rt * rr1 * rr0 * rtInv;

  size_t ix = 0;
  size_t iy = 0;
  size_t iz = 0;
  for (size_t i = 0; i < kNumInstances; ++i) {
    if (ix == kInstanceRows) {
      ix = 0;
      iy += 1;
    }
    if (iy == kInstanceRows) {
      iy = 0;
      iz += 1;
    }

    float4x4 scale = Math::makeScale((float3){scl, scl, scl});
    float4x4 zrot  = Math::makeZRotate(_angle * sinf((float)ix));
    float4x4 yrot  = Math::makeYRotate(_angle * cosf((float)iy));

    float x = ((float)ix - (float)kInstanceRows / 2.f) * (2.f * scl) + scl;
    float y = ((float)iy - (float)kInstanceColumns / 2.f) * (2.f * scl) + scl;
    float z = ((float)iz - (float)kInstanceDepth / 2.f) * (2.f * scl);
    float4x4 translate = Math::makeTranslate(
        Math::add(objectPosition, {x, y, z}));

    pInstanceData[i].instanceTransform = fullObjectRot * translate * yrot *
                                         zrot * scale;
    pInstanceData[i].instanceNormalTransform = Math::discardTranslation(
        pInstanceData[i].instanceTransform);

    float iDivNumInstances         = i / (float)kNumInstances;
    float r                        = iDivNumInstances;
    float g                        = 1.0f - r;
    float b                        = sinf(M_PI * 2.0f * iDivNumInstances);
    pInstanceData[i].instanceColor = (float4){r, g, b, 1.0f};

    ix += 1;
  }
  pInstanceDataBuffer->didModifyRange(
      NS::Range::Make(0, pInstanceDataBuffer->length()));

  updateLightData(pLightDataBuffer);

  MTL::Buffer*              pCameraDataBuffer = _pCameraDataBuffer[_frame];
  shader_types::CameraData* pCameraData =
      reinterpret_cast<shader_types::CameraData*>(
          pCameraDataBuffer->contents());
  pCameraData->perspectiveTransform = Math::makePerspective(
      45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f);
  pCameraData->worldTransform       = Math::makeIdentity();
  pCameraData->worldNormalTransform = Math::discardTranslation(
      pCameraData->worldTransform);
  pCameraDataBuffer->didModifyRange(
      NS::Range::Make(0, sizeof(shader_types::CameraData)));

  MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(_pPSO);
  pEnc->setDepthStencilState(_pDepthStencilState);

  pEnc->setVertexBuffer(_pVertexDataBuffer, 0, 0);
  pEnc->setVertexBuffer(pInstanceDataBuffer, 0, 1);
  pEnc->setVertexBuffer(pCameraDataBuffer, 0, 2);

  pEnc->setFragmentBuffer(pCameraDataBuffer, 0, 0);
  pEnc->setFragmentBuffer(pLightDataBuffer, 0, 1);

  pEnc->setCullMode(MTL::CullModeBack);
  pEnc->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);

  pEnc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
      _numIndices, MTL::IndexType::IndexTypeUInt16, _pIndexBuffer, 0,
      kNumInstances);

  pEnc->endEncoding();
  pCmd->presentDrawable(pView->currentDrawable());
  pCmd->commit();

  pPool->release();
}
