#include "Window.hpp"

Window::Window(uint32_t width, uint32_t height, const char *title) {
  if (glfwInit() == GLFW_FALSE)
    throw std::runtime_error("failed to init window!");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
};

Window::~Window() {
  if (window)
    glfwDestroyWindow(window);
  glfwTerminate();
};

vk::raii::SurfaceKHR Window::createSurface(const vk::raii::Instance &instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(*instance, window, nullptr, &surface) != 0)
    throw std::runtime_error("failed to create window surface!");
  return vk::raii::SurfaceKHR(instance, surface);
};

void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
  auto app = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window) != 0; };
bool Window::wasResized() {
  bool temp = framebufferResized;
  if (framebufferResized)
    framebufferResized = false;
  return temp;
};

vk::Extent2D Window::getExtent() const {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
};
