#include "VulkanContext.hpp"

#include <iostream>
#include <limits>
#include <set>

#include "Window.hpp"

namespace {
const std::vector<const char*> deviceExtensions = {
    vk::KHRSwapchainExtensionName};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    std::cerr << "validation layer: type " << to_string(type)
              << " msg: " << pCallbackData->pMessage << std::endl;

    return vk::False;
}

}  // namespace

VulkanContext::VulkanContext(Window& window)
    : m_context(), m_instance(nullptr), m_debugMessenger(nullptr), m_surface(nullptr), m_physicalDevice(nullptr), m_device(nullptr), m_graphicsQueue(nullptr), m_presentQueue(nullptr), m_swapChain(nullptr) {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Scop",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14};

    auto requiredExtensions = getRequiredExtensions();

    vk::InstanceCreateInfo instanceCreateInfo{
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()};

#ifndef NDEBUG
    if (checkValidationLayerSupport()) {
        instanceCreateInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        throw std::runtime_error(
            "Validation layers requested, but not available!");
    }
#endif

    m_instance = vk::raii::Instance(m_context, instanceCreateInfo);

#ifndef NDEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debugInfo{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = debugCallback};

    m_debugMessenger = vk::raii::DebugUtilsMessengerEXT(m_instance, debugInfo);
#endif

    m_surface = window.createSurface(m_instance);

    auto physicalDevices = m_instance.enumeratePhysicalDevices();
    for (const auto& device : physicalDevices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (!*m_physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({.queueFamilyIndex = queueFamily,
                                    .queueCount = 1,
                                    .pQueuePriorities = &queuePriority});
    }

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{
        .dynamicRendering = VK_TRUE};

    vk::PhysicalDeviceFeatures deviceFeatures{.samplerAnisotropy = VK_TRUE};

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &dynamicRenderingFeatures,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures};

    m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);

    m_graphicsQueue =
        vk::raii::Queue(m_device, indices.graphicsFamily.value(), 0);
    m_presentQueue = vk::raii::Queue(m_device, indices.presentFamily.value(), 0);

    m_graphicsQueueIndex = indices.graphicsFamily.value();
    m_presentQueueIndex = indices.presentFamily.value();
    createSwapChain(window);
    createImageViews();
}

VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(
    vk::raii::PhysicalDevice const& physicalDevice) const {
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
    vk::raii::PhysicalDevice const& device) const {
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
    vk::raii::PhysicalDevice const& device) const {
    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanContext::checkValidationLayerSupport() const {
    auto availableLayers = m_context.enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
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

std::vector<const char*> VulkanContext::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifndef NDEBUG
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif
    return extensions;
}

vk::raii::Device& VulkanContext::getDevice() {
    return m_device;
}
vk::raii::PhysicalDevice& VulkanContext::getPhysicalDevice() {
    return m_physicalDevice;
}
vk::raii::Queue& VulkanContext::getGraphicsQueue() {
    return m_graphicsQueue;
}
vk::raii::SurfaceKHR& VulkanContext::getSurface() {
    return m_surface;
}

VulkanContext::~VulkanContext() {
    if (*m_device) {
        m_device.waitIdle();
    }
}

VulkanContext::SwapchainSupportDetails VulkanContext::querySwapchainSupport(vk::raii::PhysicalDevice const& device) const {
    return {.capabilities = device.getSurfaceCapabilitiesKHR(*m_surface),
            .formats = device.getSurfaceFormatsKHR(*m_surface),
            .presentModes = device.getSurfacePresentModesKHR(*m_surface)};
}

vk::SurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanContext::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, Window& window) const {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = window.getExtent();

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanContext::createSwapChain(Window& window) {
    SwapchainSupportDetails swapChainSupport =
        querySwapchainSupport(m_physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = *m_surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE};

    uint32_t queueFamilyIndices[] = {m_graphicsQueueIndex, m_presentQueueIndex};

    if (m_graphicsQueueIndex != m_presentQueueIndex) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    m_swapChain = vk::raii::SwapchainKHR(m_device, createInfo);
    m_swapChainImages = m_swapChain.getImages();
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void VulkanContext::createImageViews() {
    m_swapChainImageViews.clear();
    m_swapChainImageViews.reserve(m_swapChainImages.size());

    for (const auto& image : m_swapChainImages) {
        vk::ImageViewCreateInfo viewInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = m_swapChainImageFormat,
            .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1}};

        m_swapChainImageViews.emplace_back(m_device, viewInfo);
    }
}
