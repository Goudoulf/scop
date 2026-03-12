#include <iostream>

#include "VulkanContext.hpp"
#include "Window.hpp"

int main() {
    Window window(800, 600, "Scop - Vulkan Test");

    try {
        VulkanContext context(window);

        std::cout << "Vulkan Context initialized successfully!" << std::endl;
        std::cout << "GPU: "
                  << context.getPhysicalDevice().getProperties().deviceName
                  << std::endl;

        while (!window.shouldClose()) {
            glfwPollEvents();
        }

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
