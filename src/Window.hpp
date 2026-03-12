#ifndef WINDOW_HPP
#define WINDOW_HPP
#include "vulkan/vulkan.hpp"
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
class Window {

public:
  explicit Window(uint32_t, uint32_t, const char *);

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  ~Window();

  [[nodiscard]] bool shouldClose() const;
  [[nodiscard]] bool wasResized();
  vk::raii::SurfaceKHR createSurface(const vk::raii::Instance &instance);
  [[nodiscard]] vk::Extent2D getExtent() const;
  static void framebufferResizeCallback(GLFWwindow *, int, int);
  [[nodiscard]] GLFWwindow *getGLFWWindow() const { return window; }

private:
  GLFWwindow *window = nullptr;
  uint32_t width;
  uint32_t height;
  bool framebufferResized = false;
};

#endif // !WINDOW_HPP
