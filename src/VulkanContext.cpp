
#include "VulkanContext.hpp"
#include "Window.hpp"
#include <set>

namespace {
const std::vector<const char *> deviceExtensions = {
    vk::KHRSwapchainExtensionName};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

} // namespace

VulkanContext::VulkanContext(Window &window)
    : m_context(),
      m_instance(m_context,
                 [this]() {
                   auto extensions = getRequiredExtensions();
                   std::vector<const char *> layers;
#ifndef NDEBUG
                   if (checkValidationLayerSupport()) {
                     layers.insert(layers.end(), validationLayers.begin(),
                                   validationLayers.end());
                   }
#endif

                   vk::ApplicationInfo appInfo{.pApplicationName = "Scop",
                                               .apiVersion = vk::ApiVersion13};

                   return vk::InstanceCreateInfo{
                       .pApplicationInfo = &appInfo,
                       .enabledLayerCount =
                           static_cast<uint32_t>(layers.size()),
                       .ppEnabledLayerNames = layers.data(),
                       .enabledExtensionCount =
                           static_cast<uint32_t>(extensions.size()),
                       .ppEnabledExtensionNames = extensions.data()};
                 }()),
      m_debugMessenger(nullptr), m_surface(window.createSurface(m_instance)),
      m_physicalDevice(nullptr), m_device(nullptr), m_graphicsQueue(nullptr),
      m_presentQueue(nullptr) {
  auto physicalDevices = m_instance.enumeratePhysicalDevices();
  for (const auto &device : physicalDevices) {
    if (isDeviceSuitable(device)) {
      m_physicalDevice = device;
      break;
    }
  }
}

VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(
    vk::raii::PhysicalDevice const &physicalDevice) const {

  QueueFamilyIndices indices;

  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();

  for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {

    if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
      indices.graphicsFamily = i;
    if (*m_surface && physicalDevice.getSurfaceSupportKHR(i, *m_surface))
      indices.presentFamily = i;
    if (indices.isComplete())
      break;
  };
  return indices;
}

bool VulkanContext::isDeviceSuitable(
    vk::raii::PhysicalDevice const &device) const {

  QueueFamilyIndices indices = findQueueFamilies(device);
  if (!indices.isComplete()) {
    return false;
  }

  bool extensionsSupported = checkDeviceExtensionSupport(device);
  if (!extensionsSupported) {
    return false;
  }

  auto formats = device.getSurfaceFormatsKHR(*m_surface);
  auto presentModes = device.getSurfacePresentModesKHR(*m_surface);

  if (formats.empty() || presentModes.empty()) {
    return false;
  }

  auto supportedFeatures = device.getFeatures();
  if (!supportedFeatures.samplerAnisotropy) {
    return false;
  }

  return true;
}

bool VulkanContext::checkDeviceExtensionSupport(
    vk::raii::PhysicalDevice const &device) const {

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  auto availableExtensions = device.enumerateDeviceExtensionProperties();

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

bool VulkanContext::checkValidationLayerSupport() const {
  auto availableLayers = m_context.enumerateInstanceLayerProperties();

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (std::strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }
  return true;
};
