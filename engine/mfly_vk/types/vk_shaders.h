#ifndef MFLY_VK_SHADERS
#define MFLY_VK_SHADERS

#include <vulkan/vulkan.hpp>
#include <stdint.h>

namespace mfly::vk {
    struct VkShader_InitInfo{
        uint32_t* bytecode;
        uint64_t code_size;
        uint32_t existing_shader = UINT32_MAX;
    };
    
    struct VkShaderStage_InitInfo {
        uint32_t start, end; // Uses 2 values for start and end when used in a bulk
        // Recreating a pipeline should be asking the previously created pipeline again...
        
        uint32_t logical_dvc;
        uint32_t stage;
        const char* name;
    };

     struct VkShaderBulk {
        std::vector<VkShader_InitInfo> shader_infos;
        std::vector<VkShaderStage_InitInfo> declares;
    };

    uint32_t AddShader(VkShader_InitInfo shader_info, uint32_t logical_dvc, uint32_t stage);
    uint32_t* AddShaders(const VkShaderBulk& shader_bulk);

    struct VkShaderModuleWrap {
        VkShaderModule shader;
        VkPipelineShaderStageCreateInfo stage_info = {};
    };
};

#endif