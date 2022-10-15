#ifndef MFLY_VK_SHADERS
#define MFLY_VK_SHADERS

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>
#include <stdint.h>

namespace mfly::vk {
    struct VkShader_InitInfo{
        uint32_t* bytecode;
        uint64_t code_size;
        sm_key existing_shader;
    };
    
    struct VkShaderStage_InitInfo {
        uint32_t start, end; // Uses 2 values for start and end when used in a bulk
        // Recreating a pipeline should be asking the previously created pipeline again...
        
        sm_key logical_dvc;
        uint32_t stage;
        const char* name;
    };

    struct VkShaderBulk {
        std::vector<VkShader_InitInfo> shader_infos;
        std::vector<VkShaderStage_InitInfo> declares;
    };

    bool AddShader(VkShader_InitInfo& shader_info, sm_key& logical_dvc);
    void AddShaders(VkShaderBulk& shader_bulk);

    struct VkShaderModuleWrap {
        VkShaderModule shader;
        VkPipelineShaderStageCreateInfo stage_info = {};
    };
};

#endif