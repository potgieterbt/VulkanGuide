// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "vk_descriptors.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <stddef.h>
#include <vector>
#include <vk_loader.h>
#include <vk_types.h>
#include <vulkan/vulkan_core.h>

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  VkExtent2D _windowExtent{1700, 900};
  DeletionQueue _mainDeletionQueue;
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  VmaAllocator _allocator;

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
  DescriptorAllocator globalDescriptorAllocator;
  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;

  VkPipelineLayout _meshPipelineLayout;
  VkPipeline _meshPipeline;

  std::vector<ComputeEffect> backgroundEffects;
  int currentBackgroundEffect{0};

  std::vector<std::shared_ptr<MeshAsset>> testMeshes;

public:
  void init();
  void cleanup();
  void draw();
  void draw_background(VkCommandBuffer cmd);
  void draw_geometry(VkCommandBuffer cmd);
  void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
  void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);
  GPUMeshBuffers uploadMesh(std::span<uint32_t> indices,
                            std::span<Vertex> vertices);
  void run();

private:
  void create_swapchain(uint32_t width, uint32_t height);
  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage);
  void destroy_swapchain();
  void destroy_buffer(const AllocatedBuffer &buffer);

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
};
