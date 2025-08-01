// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "glm/ext/matrix_float3x4.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "vk_descriptors.h"
#include <camera.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_loader.h>
#include <vk_types.h>
#include <vulkan/vulkan_core.h>

constexpr unsigned int FRAME_OVERLAP = 2;

struct RenderObject {
  uint32_t indexCount;
  uint32_t firstIndex;
  VkBuffer indexBuffer;

  MaterialInstance *material;
  Bounds bounds;
  glm::mat4 transform;
  VkDeviceAddress vertexBufferAddress;
};

struct EngineStats {
  float frametime;
  int triangle_count;
  int drawcall_count;
  float scene_update_time;
  float mesh_draw_time;
};

struct GLTFMetallic_Roughness {
  MaterialPipeline opaquePipeline;
  MaterialPipeline transparentPipeline;

  VkDescriptorSetLayout materialLayout;

  struct MaterialConstants {
    glm::vec4 colorFactors;
    glm::vec4 metal_rough_factors;
    glm::vec4 extra1[14];
  };

  struct MaterialResources {
    AllocatedImage colorImage;
    VkSampler colorSampler;
    AllocatedImage metalRoughImage;
    VkSampler metalRoughSampler;
    VkBuffer dataBuffer;
    uint32_t dataBufferOffset;
  };

  DescriptorWriter writer;

  void build_pipelines(VulkanEngine *engine);
  void clear_resources(VkDevice device);

  MaterialInstance
  write_material(VkDevice device, MaterialPass pass,
                 const MaterialResources &resources,
                 DescriptorAllocatorGrowable &descriptorAllocator);
};

struct DrawContext {
  std::vector<RenderObject> OpaqueSurfaces;
  std::vector<RenderObject> TransparentSurfaces;
};

struct MeshNode : public Node {
  std::shared_ptr<MeshAsset> mesh;

  virtual void Draw(const glm::mat4 &topMatrix, DrawContext &ctx) override;
};

class VulkanEngine {
public:
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  bool resize_requested{false};
  VkExtent2D _windowExtent{1700, 900};
  DeletionQueue _mainDeletionQueue;
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  float renderScale = 1.f;
  VmaAllocator _allocator;

  EngineStats stats;
  Camera mainCamera;

  AllocatedImage _whiteImage;
  AllocatedImage _blackImage;
  AllocatedImage _greyImage;
  AllocatedImage _errorCheckerboardImage;

  MaterialInstance defaultData;
  GLTFMetallic_Roughness metalRoughMaterial;

  VkSampler _defaultSamplerLinear;
  VkSampler _defaultSamplerNearest;

  VkFence _immFence;
  VkCommandBuffer _immCommandBuffer;
  VkCommandPool _immCommandPool;

public:
  VkInstance _instance;
  VkDebugUtilsMessengerEXT _debug_messenger;
  VkPhysicalDevice _chosenGPU;
  VkDevice _device;
  VkSurfaceKHR _surface;

  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

public:
  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;

  struct SDL_Window *_window{nullptr};

  static VulkanEngine &Get();

public:
  FrameData _frames[FRAME_OVERLAP];

  FrameData &get_current_frame() {
    return _frames[_frameNumber % FRAME_OVERLAP];
  }

public:
  DescriptorAllocatorGrowable globalDescriptorAllocator;
  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  DrawContext mainDrawContext;
  std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;

  GPUSceneData sceneData;
  VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;

  VkDescriptorSetLayout _singleImageDescriptorLayout;

  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;

  VkPipelineLayout _meshPipelineLayout;
  VkPipeline _meshPipeline;

  std::vector<ComputeEffect> backgroundEffects;
  int currentBackgroundEffect{0};

  std::vector<std::shared_ptr<MeshAsset>> testMeshes;

  std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;

public:
  void init();
  void cleanup();
  void draw();
  void draw_background(VkCommandBuffer cmd);
  void draw_geometry(VkCommandBuffer cmd);
  void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
  void update_scene();
  void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);
  GPUMeshBuffers uploadMesh(std::span<uint32_t> indices,
                            std::span<Vertex> vertices);
  void run();

  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage);
  AllocatedImage create_image(void *data, VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usage, bool mipmapped = false);
  AllocatedImage create_image(VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usage, bool mipmapped = false);
  void destroy_buffer(const AllocatedBuffer &buffer);
  void destroy_image(const AllocatedImage &img);

private:
  void create_swapchain(uint32_t width, uint32_t height);

  void destroy_swapchain();

private:
  void init_imgui();
  void init_vulkan();
  void init_swapchain();
  void init_commands();
  void init_sync_structures();
  void init_descriptors();
  void init_pipelines();
  void init_background_pipelines();
  void init_triangle_pipeline();
  void init_mesh_pipeline();
  void init_default_data();
  void resize_swapchain();
};

bool is_visible(const RenderObject &obj, const glm::mat4 &viewproj);
