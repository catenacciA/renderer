#ifndef MESH_HPP
#define MESH_HPP

#include <cmath>
#include <memory>
#include <vector>

#include "Math.hpp"

namespace shader_types {
struct VertexData {
  simd::float3 position;
  simd::float3 normal;
};

struct InstanceData {
  simd::float4x4 instanceTransform;
  simd::float3x3 instanceNormalTransform;
  simd::float4   instanceColor;
};

struct CameraData {
  simd::float4x4 perspectiveTransform;
  simd::float4x4 worldTransform;
  simd::float3x3 worldNormalTransform;
};
struct LightData {
  simd::float3 position;
  simd::half3  color;
  float        intensity;
  float        range;
  float        pulseSpeed;
  float        time;
};
}  // namespace shader_types

class Mesh {
 public:
  virtual ~Mesh() = default;

  virtual std::vector<shader_types::VertexData> getVertices() const = 0;
  virtual std::vector<uint16_t>                 getIndices() const  = 0;
};

class SphereMesh : public Mesh {
 public:
  SphereMesh(float radius, unsigned int stacks, unsigned int slices)
      : radius_(radius), stacks_(stacks), slices_(slices) {}

  std::vector<shader_types::VertexData> getVertices() const override {
    std::vector<shader_types::VertexData> vertices;
    for (unsigned int i = 0; i <= stacks_; ++i) {
      float V   = static_cast<float>(i) / static_cast<float>(stacks_);
      float phi = V * M_PI;
      for (unsigned int j = 0; j <= slices_; ++j) {
        float U     = static_cast<float>(j) / static_cast<float>(slices_);
        float theta = U * (M_PI * 2);

        float x = radius_ * sinf(phi) * cosf(theta);
        float y = radius_ * cosf(phi);
        float z = radius_ * sinf(phi) * sinf(theta);

        float nx = x / radius_;
        float ny = y / radius_;
        float nz = z / radius_;

        vertices.push_back({{x, y, z}, {nx, ny, nz}});
      }
    }
    return vertices;
  }

  std::vector<uint16_t> getIndices() const override {
    std::vector<uint16_t> indices;
    for (unsigned int i = 0; i < stacks_; ++i) {
      for (unsigned int j = 0; j < slices_; ++j) {
        uint16_t first  = (i * (slices_ + 1)) + j;
        uint16_t second = first + slices_ + 1;

        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);

        indices.push_back(second);
        indices.push_back(second + 1);
        indices.push_back(first + 1);
      }
    }
    return indices;
  }

 private:
  float        radius_;
  unsigned int stacks_;
  unsigned int slices_;
};

class CubeMesh : public Mesh {
 public:
  CubeMesh(float sideLength) : s(sideLength) {}

  std::vector<shader_types::VertexData> getVertices() const override {
    std::vector<shader_types::VertexData> vertices = {
        //   Positions          Normals
        {{-s, -s, +s}, {0.f, 0.f, 1.f}},
        {{+s, -s, +s}, {0.f, 0.f, 1.f}},
        {{+s, +s, +s}, {0.f, 0.f, 1.f}},
        {{-s, +s, +s}, {0.f, 0.f, 1.f}},

        {{+s, -s, +s}, {1.f, 0.f, 0.f}},
        {{+s, -s, -s}, {1.f, 0.f, 0.f}},
        {{+s, +s, -s}, {1.f, 0.f, 0.f}},
        {{+s, +s, +s}, {1.f, 0.f, 0.f}},

        {{+s, -s, -s}, {0.f, 0.f, -1.f}},
        {{-s, -s, -s}, {0.f, 0.f, -1.f}},
        {{-s, +s, -s}, {0.f, 0.f, -1.f}},
        {{+s, +s, -s}, {0.f, 0.f, -1.f}},

        {{-s, -s, -s}, {-1.f, 0.f, 0.f}},
        {{-s, -s, +s}, {-1.f, 0.f, 0.f}},
        {{-s, +s, +s}, {-1.f, 0.f, 0.f}},
        {{-s, +s, -s}, {-1.f, 0.f, 0.f}},

        {{-s, +s, +s}, {0.f, 1.f, 0.f}},
        {{+s, +s, +s}, {0.f, 1.f, 0.f}},
        {{+s, +s, -s}, {0.f, 1.f, 0.f}},
        {{-s, +s, -s}, {0.f, 1.f, 0.f}},

        {{-s, -s, -s}, {0.f, -1.f, 0.f}},
        {{+s, -s, -s}, {0.f, -1.f, 0.f}},
        {{+s, -s, +s}, {0.f, -1.f, 0.f}},
        {{-s, -s, +s}, {0.f, -1.f, 0.f}},
    };
    return vertices;
  }

  std::vector<uint16_t> getIndices() const override {
    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,        // front
        4, 5, 6, 6, 7, 4,        // right
        8, 9, 10, 10, 11, 8,     // back
        12, 13, 14, 14, 15, 12,  // left
        16, 17, 18, 18, 19, 16,  // top
        20, 21, 22, 22, 23, 20,  // bottom
    };
    return indices;
  }

 private:
  float s;
};

enum class MeshType { Sphere, Cube };

std::unique_ptr<Mesh> createMesh(MeshType type) {
  switch (type) {
    case MeshType::Sphere: return std::make_unique<SphereMesh>(0.5f, 20, 20);
    case MeshType::Cube: return std::make_unique<CubeMesh>(0.5f);
    default: return nullptr;
  }
}

#endif  // MESH_HPP