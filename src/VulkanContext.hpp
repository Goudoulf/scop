#ifndef VULKANCONTEXT_HPP
#define VULKANCONTEXT_HPP
#include <optional>
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Window;

class VulkanContext {

public:
  explicit VulkanContext(Window &);
  VulkanContext(const VulkanContext &) = delete;
  VulkanContext &operator=(const VulkanContext &) = delete;

  ~VulkanContext();

  std::vector<const char *> getRequiredExtensions();

  [[nodiscard]] vk::raii::Device &getDevice();
  [[nodiscard]] vk::raii::PhysicalDevice &getPhysicalDevice();
  [[nodiscard]] vk::raii::Queue &getGraphicsQueue();
  [[nodiscard]] vk::raii::SurfaceKHR &getSurface();

private:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  QueueFamilyIndices
  findQueueFamilies(vk::raii::PhysicalDevice const &device) const;

  bool checkValidationLayerSupport() const;
  [[nodiscard]] bool
  isDeviceSuitable(vk::raii::PhysicalDevice const &device) const;
  [[nodiscard]] bool
  checkDeviceExtensionSupport(vk::raii::PhysicalDevice const &device) const;

  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  SwapchainSupportDetails
  querySwapchainSupport(vk::raii::PhysicalDevice const &device) const;

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats) const;
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes) const;
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                Window &window) const;

  void createSwapChain(Window &window);
  void createImageViews();

  vk::raii::Context m_context;
  vk::raii::Instance m_instance;
  vk::raii::DebugUtilsMessengerEXT m_debugMessenger;
  vk::raii::SurfaceKHR m_surface;
  vk::raii::PhysicalDevice m_physicalDevice;
  vk::raii::Device m_device;

  vk::raii::Queue m_graphicsQueue;
  vk::raii::Queue m_presentQueue;
  uint32_t m_graphicsQueueIndex;
  uint32_t m_presentQueueIndex;

  vk::raii::SwapchainKHR m_swapChain;
  std::vector<vk::Image> m_swapChainImages;
  vk::Format m_swapChainImageFormat;
  vk::Extent2D m_swapChainExtent;

  std::vector<vk::raii::ImageView> m_swapChainImageViews;
};

#endif // !VULKANCONTEXT_HPP
