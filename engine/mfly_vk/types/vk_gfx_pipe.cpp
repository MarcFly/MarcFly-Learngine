#include "vk_gfx_pipe.h"
#include "vk_renderpass.h"
#include "../mfly_vk.hpp"
#include "vk_images.h"

using namespace mfly;

sm_key mfly::vk::CreateGraphicsPipeline(sm_key& dvc_handle, VkGraphicsPipeStateInfoWrap pipe_info) {
    VkPipelineVertexInputStateCreateInfo vtx_info = {};
    vtx_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Function to set descriptors from binding descriptors and attribute descriptors
    vtx_info.vertexBindingDescriptionCount = vtx_info.vertexAttributeDescriptionCount = 0;
    vtx_info.pVertexBindingDescriptions = nullptr;
    vtx_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_info = {};
    input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_info.topology = (VkPrimitiveTopology)pipe_info.input_info.topology;
    input_info.primitiveRestartEnable = pipe_info.input_info.restart_on_indexed;

    VkPipelineDynamicStateCreateInfo dyn_state = {};
    dyn_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_state.dynamicStateCount = pipe_info.dyn_states.size();
    dyn_state.pDynamicStates = pipe_info.dyn_states.data();

    VkPipelineViewportStateCreateInfo vp_state = {};
    vp_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp_state.viewportCount = pipe_info.viewports.size();
    vp_state.pViewports = pipe_info.viewports.data();
    vp_state.scissorCount = pipe_info.scissors.size();
    vp_state.pScissors = pipe_info.scissors.data();

    VkPipelineRasterizationStateCreateInfo raster_state = {};
    raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_state.depthClampEnable = pipe_info.raster_info.clamp_depth;
    raster_state.rasterizerDiscardEnable = pipe_info.raster_info.disable_all;
    
    raster_state.polygonMode = (VkPolygonMode)pipe_info.raster_info.poly_mode;
    raster_state.lineWidth = 1.0f; // TODO: Add to raster info
    
    raster_state.cullMode = pipe_info.raster_info.cull_mode;
    raster_state.frontFace = (VkFrontFace)pipe_info.raster_info.front_face;
    
    raster_state.depthBiasEnable = pipe_info.raster_info.bias;
    raster_state.depthBiasConstantFactor = pipe_info.raster_info.bias_constant;
    raster_state.depthBiasClamp = pipe_info.raster_info.bias_clamp;
    raster_state.depthBiasSlopeFactor = pipe_info.raster_info.bias_slope;

    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.sampleShadingEnable = pipe_info.multisample_info.per_sample;
    multisample_state.rasterizationSamples = (VkSampleCountFlagBits)pipe_info.multisample_info.num_samples;
    multisample_state.minSampleShading = pipe_info.multisample_info.min_per_samples;
    multisample_state.pSampleMask = nullptr; // TODO: Add holder of masks
    multisample_state.alphaToCoverageEnable = pipe_info.multisample_info.alpha_coverage;
    multisample_state.alphaToOneEnable = pipe_info.multisample_info.set_alpha_to_one;

    // TODO: Depth and Stencil Testing

    VkPipelineColorBlendStateCreateInfo blend_state = {};
    blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_state.logicOpEnable = pipe_info.blend_info.logic_blend;
    blend_state.logicOp = (VkLogicOp)pipe_info.blend_info.logic_op;

    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    for(auto attachment : pipe_info.blend_info.attachments) {
        attachments.push_back(VkPipelineColorBlendAttachmentState());
        VkPipelineColorBlendAttachmentState& curr = attachments.back();
        curr.colorWriteMask = attachment.channel_mask;
        curr.blendEnable = attachment.blend;
        curr.srcColorBlendFactor = (VkBlendFactor)attachment.src_color;
        curr.dstColorBlendFactor = (VkBlendFactor)attachment.dst_color;
        curr.colorBlendOp = (VkBlendOp)attachment.color_op;
        curr.srcAlphaBlendFactor = (VkBlendFactor)attachment.src_alpha;
        curr.dstAlphaBlendFactor = (VkBlendFactor)attachment.dst_alpha;
        curr.alphaBlendOp = (VkBlendOp)attachment.alpha_op;
    }
    blend_state.attachmentCount = pipe_info.blend_info.attachments.size();
    blend_state.pAttachments = attachments.data();
    for(int i = 0; i < pipe_info.blend_info.constants.size(); ++i) {
        blend_state.blendConstants[i] = pipe_info.blend_info.constants[i];
    }

    VkPipelineLayoutCreateInfo layout_state = {};
    layout_state.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // TODO: Get layout sets from handles and way to store them
    layout_state.setLayoutCount = pipe_info.layout_info.count;
    layout_state.pSetLayouts = nullptr;
    // TODO: Get constants from handles and way to store them
    layout_state.pushConstantRangeCount = pipe_info.layout_info.push_constants_handles.size();
    layout_state.pPushConstantRanges = nullptr;

    sm_key ret = vkapp.graphic_pipes.push_back(VkGraphicsPipelineWrap());
    VkGraphicsPipelineWrap& pipe_wrap = vkapp.graphic_pipes[ret];
    VkPipelineLayout* pipe = &pipe_wrap.layout;
    // TODO: Way to select proper device
    VkResult res = vkCreatePipelineLayout(vkapp.logical_dvcs[dvc_handle], &layout_state, nullptr, pipe);
    if(res != VK_SUCCESS)
        printf("Failed to create pipeline layout\n"); 

    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = pipe_info.num_stages;
    std::vector<VkShaderModuleWrap> shaders;
    vkapp.shaders.from_handles(pipe_info.shaders, shaders);
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
    for(auto shader : shaders) {
        shader_stage_infos.push_back(shader.stage_info);
        VkPipelineShaderStageCreateInfo& sh_info = shader_stage_infos.back();
        sh_info.pName = pipe_info.name;
    }   
    create_info.pStages = shader_stage_infos.data();

    create_info.stageCount = shader_stage_infos.size();
    create_info.pStages = shader_stage_infos.data();

    create_info.pVertexInputState = &vtx_info;
    create_info.pInputAssemblyState = &input_info;
    create_info.pViewportState = &vp_state;
    create_info.pRasterizationState = &raster_state;
    create_info.pMultisampleState = &multisample_state;
    create_info.pDepthStencilState = nullptr; // TODO
    create_info.pColorBlendState = &blend_state;
    create_info.pDynamicState = &dyn_state;
    create_info.layout = *pipe;
    
    create_info.renderPass = vkapp.render_passes[pipe_info.render_pass_handle];
    create_info.subpass = 0; // TODO: Properly set the subpass
    
    create_info.basePipelineHandle = VK_NULL_HANDLE; // TODO: Deriving from previously created / Requires flag VK_PIPELINE_CRETE_DERIVATIVE_BIT
    create_info.basePipelineIndex = -1; // TODO: index from the one that are about to be created... prefer handle maybe

    res = vkCreateGraphicsPipelines(vkapp.logical_dvcs[dvc_handle], VK_NULL_HANDLE, 1, &create_info, nullptr, &pipe_wrap.pipe);
    // TODO: Allow multiple pipelines to be created at the same time
    if(res != VK_SUCCESS)
        printf("Failed to create graphics pipeline");

    return ret;
};


//=================================================


void mfly::vk::SetDynState(sm_key& swapchain_handle, VkCommandBuffer cmd_buf) {
    VkSwapchainWrap& swc = vkapp.swapchains[swapchain_handle];

    // Temp
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swc.area.width);
    viewport.height = static_cast<float>(swc.area.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swc.area;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
    // *Temp  
}
