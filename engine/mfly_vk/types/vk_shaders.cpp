#include "vk_shaders.h"
#include "../mfly_vk.hpp"

using namespace mfly;

bool mfly::vk::AddShader(VkShader_InitInfo& shader_info, sm_key& logical_dvc) {
    bool ret = true;
    
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_info.code_size;
    create_info.pCode = shader_info.bytecode;
    
    vkapp.shaders.insert(shader_info.existing_shader, VkShaderModuleWrap());
    VkShaderModule& shader = vkapp.shaders[shader_info.existing_shader].shader;
    VkResult res = vkCreateShaderModule(vkapp.logical_dvcs[logical_dvc], &create_info, nullptr, &shader);
    if(res != VK_SUCCESS) {
        // TODO: Hookup logging callback!
        ret = false;
    }
    return ret;
}

void mfly::vk::AddShaders(VkShaderBulk& shader_bulk) {
    for(auto group : shader_bulk.declares) {
        for(int i = group.start; i < group.end; ++i){
            VkShader_InitInfo& shader_info = shader_bulk.shader_infos[i];
            bool success = AddShader(shader_info, group.logical_dvc);
            if(success) {
                VkShaderModuleWrap& shader = vkapp.shaders[shader_info.existing_shader];
                shader.stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader.stage_info.pName = group.name;
                shader.stage_info.stage = (VkShaderStageFlagBits)group.stage;
                shader.stage_info.module = shader.shader;
            }
        }
    }
}
