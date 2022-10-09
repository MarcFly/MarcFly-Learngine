#include "vk_renderpass.h"
#include "../mfly_vk.hpp"

uint32_t mfly::vk::AddAttachmentDesc(VkAttachmentInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.attachment_descs, existing);
    VkAttachmentDescription& desc = vkapp.attachment_descs[existing];
    
    desc.loadOp = (VkAttachmentLoadOp)info.ops.load;
    desc.storeOp = (VkAttachmentStoreOp)info.ops.store;
    desc.stencilLoadOp = (VkAttachmentLoadOp)info.ops.stencil_load;
    desc.stencilStoreOp = (VkAttachmentStoreOp)info.ops.stencil_store;

    desc.samples = (VkSampleCountFlagBits)info.samples;
    desc.format = (VkFormat)info.format;

    // TODO: Research VkAttachment Description Flags
    
    desc.initialLayout = info.input_layout;
    desc.finalLayout = info.output_layout;

    return existing;
}

uint32_t mfly::vk::AddSubPass(VkSubPassInfoWrap& info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.subpasses, existing);
    
    VkSubpassDescWrap& sp_wrap = vkapp.subpasses[existing];
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
    
    return existing;
}

uint32_t mfly::vk::CreateRenderPass(VkRenderPassInfoWrap info, uint32_t existing) {
    bool destroy_pass = existing != UINT32_MAX;
    PushNonInvalid(vkapp.render_passes, existing);
    existing = PushNonInvalid(vkapp.render_pass_infos, existing);
    VkRenderPass* pass = &vkapp.render_passes[existing];
    VkRenderPassInfoWrap& pass_info = vkapp.render_pass_infos[existing];

    if(destroy_pass) {
        // TODO: Select proper device
        vkDestroyRenderPass(vkapp.logical_dvcs[0], *pass, nullptr);
    }

    pass_info = info;
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = pass_info.attachment_handles.size();
    std::vector<VkAttachmentDescription> attachments;
    VecFromHandles(pass_info.attachment_handles, vkapp.attachment_descs, attachments);
    create_info.pAttachments = attachments.data();
    
    std::vector<VkSubpassDescWrap> subpasses;
    VecFromHandles(pass_info.subpass_handles, vkapp.subpasses, subpasses);
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


    VkResult res = vkCreateRenderPass(vkapp.logical_dvcs[0], &create_info, nullptr, pass);

    if(res != VK_SUCCESS)
        printf("Failed to create rende pass");

    return vkapp.render_passes.size()-1; // Always return, if it failed, you have to repair it or will crash somewhere else
}

//=============================================


uint32_t mfly::vk::AddRenderPassBegin(VkBeginRenderPassInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.begin_renderpass_infos, existing);
    VkRenderPassBeginInfoWrap& bri_wrap  = vkapp.begin_renderpass_infos[existing];
    info.clear_colors.swap(bri_wrap.colors);
    bri_wrap.create_info = {};
    bri_wrap.create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    bri_wrap.create_info.renderPass = vkapp.render_passes[info.render_pass_handle];
    bri_wrap.create_info.framebuffer = vkapp.framebuffers[info.framebuffer_handle].framebuffer;
    bri_wrap.create_info.renderArea.extent = info.extent;
    bri_wrap.create_info.renderArea.offset = info.offset;
    bri_wrap.create_info.pClearValues = bri_wrap.colors.data();
    bri_wrap.create_info.clearValueCount = bri_wrap.colors.size();

    return existing;
}

uint32_t mfly::vk::BeginRenderPass(uint32_t begin_renderpass_handle, VkCommandBuffer cmd_buf) {
    VkRenderPassBeginInfoWrap& info = vkapp.begin_renderpass_infos[begin_renderpass_handle];

    // TODO: Change the last parameter based on required subpasses or not (currently only primary passes)
    info.create_info.framebuffer = vkapp.framebuffers[0].framebuffer;
    info.create_info.renderArea.extent = vkapp.swapchains[0].area;
    vkCmdBeginRenderPass(cmd_buf, &info.create_info, VK_SUBPASS_CONTENTS_INLINE);
    return 0;
}
