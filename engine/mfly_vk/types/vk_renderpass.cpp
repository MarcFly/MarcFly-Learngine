#include "vk_renderpass.h"
#include "../mfly_vk.hpp"

using namespace mfly;

void  mfly::vk::AddAttachmentDesc(sm_key& attachment_desc_handle, VkAttachmentInfoWrap info) {
    VkAttachmentDescription& desc = vkapp.attachment_descs.insert(attachment_desc_handle, VkAttachmentDescription());
    
    desc.loadOp = (VkAttachmentLoadOp)info.ops.load;
    desc.storeOp = (VkAttachmentStoreOp)info.ops.store;
    desc.stencilLoadOp = (VkAttachmentLoadOp)info.ops.stencil_load;
    desc.stencilStoreOp = (VkAttachmentStoreOp)info.ops.stencil_store;

    desc.samples = (VkSampleCountFlagBits)info.samples;
    desc.format = (VkFormat)info.format;

    // TODO: Research VkAttachment Description Flags
    
    desc.initialLayout = info.input_layout;
    desc.finalLayout = info.output_layout;
}

void mfly::vk::AddSubPass(sm_key& subpass_handle, VkSubPassInfoWrap& info) {
    
    VkSubpassDescWrap& sp_wrap = vkapp.subpasses.insert(subpass_handle, VkSubpassDescWrap());
    VkSubpassDescription& subpass = sp_wrap.desc;
    VkSubPassInfoWrap& subpass_info = sp_wrap.info;
    subpass_info.framebuffers.swap(info.framebuffers);
    subpass_info.depth_stencil->layout = info.depth_stencil->layout;
    subpass_info.depth_stencil->attachment = info.depth_stencil->attachment;
    subpass_info.inputs.swap(info.inputs);
    subpass_info.preserve.swap(info.preserve);
    subpass_info.sampling_resolves.swap(info.sampling_resolves);

    subpass = {};
    // TODO: Research https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescriptionFlagBits.html
    //subpass.flags
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // TODO: bindpoint to info
    subpass.colorAttachmentCount = subpass_info.framebuffers.size();
    subpass.pColorAttachments = (subpass.colorAttachmentCount == 0) ? nullptr : subpass_info.framebuffers.data();
    subpass.inputAttachmentCount = subpass_info.inputs.size();
    subpass.pInputAttachments = (subpass.inputAttachmentCount == 0) ? nullptr : subpass_info.inputs.data();
    //subpass.pDepthStencilAttachment = subpass_info.depth_stencil; // Not used for now
    subpass.preserveAttachmentCount = subpass_info.preserve.size();
    subpass.pPreserveAttachments = (subpass.preserveAttachmentCount == 0) ? nullptr : subpass_info.preserve.data();
    subpass.pResolveAttachments = (subpass_info.sampling_resolves.size() == 0) ? nullptr : subpass_info.sampling_resolves.data();
}

void mfly::vk::CreateRenderPass(sm_key& renderpass_handle, sm_key& renderpass_info_handle, VkRenderPassInfoWrap info) {
    VkRenderPass& pass = vkapp.render_passes.insert(renderpass_handle, VkRenderPass());
    VkRenderPassInfoWrap& pass_info = vkapp.render_pass_infos.insert(renderpass_info_handle, info);

    pass_info = info;
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = pass_info.attachment_handles.size();
    std::vector<VkAttachmentDescription> attachments;
    vkapp.attachment_descs.from_handles(pass_info.attachment_handles, attachments);
    create_info.pAttachments = attachments.data();
    
    std::vector<VkSubpassDescWrap> subpasses;
    vkapp.subpasses.from_handles(pass_info.subpass_handles, subpasses);

    std::vector<VkSubpassDescription> sp_descs;
    for(auto sp : subpasses) {sp_descs.push_back(sp.desc);}
    create_info.subpassCount = sp_descs.size();
    create_info.pSubpasses = sp_descs.data();
    // TODO: Research RenderPass Flags, Dependencies and pNext

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Wether they are internal subpasses or from other renderpasses
    dependency.dstSubpass = 0; // Index of the subpass?, per subpass dependency creation...
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What to wait on
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What waits on this
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Define the dependencies
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;


    VkResult res = vkCreateRenderPass(vkapp.logical_dvcs[0], &create_info, nullptr, &pass);

    if(res != VK_SUCCESS)
        printf("Failed to create rende pass");
        
     // Always return, if it failed, you have to repair it or will crash somewhere else
}

//=============================================


void mfly::vk::AddRenderPassBegin(sm_key& begin_renderpass_handle, VkBeginRenderPassInfoWrap info) {
    VkRenderPassBeginInfoWrap& bri_wrap = vkapp.begin_renderpass_infos.insert(begin_renderpass_handle, VkRenderPassBeginInfoWrap());
    info.clear_colors.swap(bri_wrap.colors);
    bri_wrap.create_info = {};
    bri_wrap.create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    bri_wrap.create_info.renderPass = vkapp.render_passes[info.render_pass_handle];
    bri_wrap.create_info.framebuffer = vkapp.framebuffers[info.framebuffer_handle].framebuffer;
    bri_wrap.create_info.renderArea.extent = info.extent;
    bri_wrap.create_info.renderArea.offset = info.offset;
    bri_wrap.create_info.pClearValues = bri_wrap.colors.data();
    bri_wrap.create_info.clearValueCount = bri_wrap.colors.size();
}

void mfly::vk::BeginRenderPass(sm_key& begin_renderpass_handle, VkCommandBuffer cmd_buf) {
    VkRenderPassBeginInfoWrap& info = vkapp.begin_renderpass_infos[begin_renderpass_handle];

    // TODO: Change the last parameter based on required subpasses or not (currently only primary passes)
    info.create_info.framebuffer = vkapp.framebuffers[0].framebuffer;
    info.create_info.renderArea.extent = vkapp.swapchains[0].area;
    vkCmdBeginRenderPass(cmd_buf, &info.create_info, VK_SUBPASS_CONTENTS_INLINE);
}
