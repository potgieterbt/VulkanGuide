#pragma once

#include "fastgltf/types.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "vk_descriptors.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_types.h>

class VulkanEngine;

struct Bounds {
  glm::vec3 origin;
  float sphereRadius;
  glm::vec3 extents;
};

struct GLTFMaterial {
  MaterialInstance data;
};

struct GeoSurface {
  uint32_t startIndex;
  uint32_t count;
  Bounds bounds;
  std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset {
  std::string name;

  std::vector<GeoSurface> surfaces;
  GPUMeshBuffers meshBuffers;
};

struct LoadedGLTF : public IRenderable {
  std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
  std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
  std::unordered_map<std::string, AllocatedImage> images;
  std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

  std::vector<std::shared_ptr<Node>> topNodes;

  std::vector<VkSampler> samplers;

  DescriptorAllocatorGrowable descriptorPool;

  AllocatedBuffer materialDataBuffer;

  VulkanEngine *creator;

  ~LoadedGLTF() { clearAll(); };

  virtual void Draw(const glm::mat4 &topMatrix, DrawContext &ctx);

private:
  void clearAll();
};

std::optional<AllocatedImage> load_image(VulkanEngine *engine,
                                         fastgltf::Asset &asset,
                                         fastgltf::Image &image);
std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanEngine *engine,
                                                    std::string_view filePath);
