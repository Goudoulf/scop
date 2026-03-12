#include "Renderer.hpp"

#include <fstream>

#include "VulkanContext.hpp"

void Renderer::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo layoutInfo{};
    m_pipelineLayout = vk::raii::PipelineLayout(m_context.getDevice(), layoutInfo);
}

std::vector<char> Renderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
