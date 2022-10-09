#include "vk_shaders.h"
#include "../mfly_vk.hpp"

uint32_t mfly::vk:: AddShader(VkShader_InitInfo shader_info, uint32_t logical_dvc, uint32_t stage) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_info.code_size;
    create_info.pCode = shader_info.bytecode;
    
    if(shader_info.existing_shader == UINT32_MAX) {
        vkapp.shaders.push_back(VkShaderModuleWrap());
        shader_info.existing_shader = vkapp.shaders.size()-1;
    }
    VkShaderModule& shader = vkapp.shaders[shader_info.existing_shader].shader;
    VkShaderModule tmp;
    VkResult res = vkCreateShaderModule(vkapp.logical_dvcs[logical_dvc], &create_info, nullptr, &tmp);
    if(res != VK_SUCCESS){
        return UINT32_MAX;
    }
    else {
        shader = tmp;
        return shader_info.existing_shader;
    }
}

uint32_t* mfly::vk::AddShaders(const VkShaderBulk& shader_bulk) {
    uint32_t* handles = new uint32_t[shader_bulk.shader_infos.size()];
    uint32_t shader_num = vkapp.shaders.size();
    vkapp.shaders.reserve(vkapp.shaders.size() + shader_bulk.shader_infos.size());
    for(auto group : shader_bulk.declares) {
        for(int i = group.start; i < group.end; ++i){
            uint32_t ret = AddShader(shader_bulk.shader_infos[i], group.logical_dvc, group.stage);
            if(ret != UINT32_MAX) {
                VkShaderModuleWrap& shader = vkapp.shaders[ret];
                shader.stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader.stage_info.pName = group.name;
                shader.stage_info.stage = (VkShaderStageFlagBits)group.stage;
                shader.stage_info.module = shader.shader;
            }
            handles[i] = ret;
        }
    }
    return handles;
}
