#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class VulkanContext;

class Renderer {
   public:
    explicit Renderer(VulkanContext& context);
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

   private:
    void createGraphicsPipeline();
    void createPipelineLayout();

    std::vector<char> readFile(const std::string& filename);
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);
    vk::Format findDepthFormat();

    VulkanContext& m_context;

    vk::raii::PipelineLayout m_pipelineLayout;
    vk::raii::Pipeline m_graphicsPipeline;
};
