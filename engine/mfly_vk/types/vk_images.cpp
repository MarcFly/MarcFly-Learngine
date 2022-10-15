#include "vk_images.h"
#include "../mfly_vk.hpp"

using namespace mfly;

sm_key mfly::vk::CreateImage(VkImage_InitInfo info)
{
    // Example Image
    info.img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    sm_key img_key = vkapp.images.push_back(VkImageMemWrap());
    VkImageMemWrap &img = vkapp.images[img_key];
    // TODO: Select logical device properly
    VkDevice dvc = vkapp.logical_dvcs[info.handles.logical_dvc_handle];
    vkCreateImage(dvc, &info.img, nullptr, &img.img);

    VkMem_InitInfo mem_info = {};
    vkGetImageMemoryRequirements(dvc, img.img, &mem_info.mem);

    uint64_t use_offset = AllocMem(info.handles, mem_info);
    img.mem_handle = info.handles.mem_handle;

    vkBindImageMemory(dvc, img.img, vkapp.mem[info.handles.mem_handle].mem, use_offset);

    for (auto view_info : info.views)
    {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = img.img;

        img.view_handles.push_back(vkapp.img_views.push_back(VkImageView()));
        
        vkCreateImageView(dvc, &view_info, nullptr, &vkapp.img_views[img.view_handles.back()]);
    }

    // DEBUG INFO
    vkapp_info.imgs.push_back(info);

    return img_key;
}

//========================================================


sm_key mfly::vk::AddFramebuffer(sm_key& dvc_handle, VkFramebufferInfoWrap info) {
    sm_key fb_key = vkapp.framebuffers.push(VkFramebufferWrap());
    VkFramebufferWrap& fb_wrap = vkapp.framebuffers[fb_key];

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    std::vector<VkImageView> img_views;
    vkapp.img_views.from_handles(info.img_view_handles, img_views);
    create_info.attachmentCount = img_views.size();
    create_info.pAttachments = img_views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[dvc_handle], &create_info, nullptr, &fb_wrap.framebuffer);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    return fb_key;
}

sm_key mfly::vk::AddSWCFramebuffer(sm_key& dvc_handle, VkFramebufferInfoWrap info, sm_key& swapchain_handle) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    
    sm_key fb_key = vkapp.framebuffers.push(VkFramebufferWrap());
    VkFramebuffer& fb = vkapp.framebuffers[fb_key].framebuffer;
    swc_wrap.framebuffers.push_back(fb_key);
    // TODO: ...
    //if(prev_val == existing) vkDestroyFramebuffer(vkapp.logical_dvcs[0], fb,nullptr);

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    create_info.attachmentCount = swc_wrap.img_views.size();
    std::vector<VkImageView> views;
    vkapp.img_views.from_handles(swc_wrap.img_views, views);
    create_info.pAttachments = views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[dvc_handle], &create_info, nullptr, &fb);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    sm_key fb_info_key = fb_key;
    vkapp_info.framebuffers.insert(fb_info_key, info);
    swc_wrap.fb_infos.push_back(fb_info_key);
    VkFramebufferInfoWrap& fb_info = vkapp_info.framebuffers[fb_info_key];
    fb_info.extent = info.extent;
    for(int i = 0; i < swc_wrap.img_views.size(); ++i)
        info.img_view_handles.push_back(swc_wrap.img_views[i]);
    fb_info.img_view_handles.swap(info.img_view_handles);
    fb_info.num_layers = info.num_layers;
    fb_info.render_pass_handle;

    return fb_key;
}

//========================================================

void mfly::vk::CreateSwapchain(sm_key& swapchain_handle, VkSwapchainCreateInfoKHR info, sm_key surface_handle, sm_key logical_dvc_handle)
{
    VkSwapchainWrap &scw = vkapp.swapchains.insert(swapchain_handle, VkSwapchainWrap());

    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = vkapp.surfaces[surface_handle];
    // There are many more settings, research

    info.minImageCount = 1;

    scw.logical_dvc_handle = logical_dvc_handle;
    VkDevice &dvc = vkapp.logical_dvcs[logical_dvc_handle];
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkResult res = vkCreateSwapchainKHR(dvc, &info, nullptr, &scw.swapchain);

    scw.area = info.imageExtent;
    scw.surface_handle = surface_handle;

    uint32_t img_count;
    //vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, nullptr);
    img_count = 1;
    std::vector<VkImage> swc_images;
    swc_images.resize(img_count);
    vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, swc_images.data());
    // TODO: Add them later

    // TODO: Proper info for the image views
    VkImageViewCreateInfo view_info = {};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT if it is different format than image is specified
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    // This is when knowing if a texture is not in RGBA order... old textures or other formats
    // Most of the cases this is still irrelevant as for internal formats you will most probably implement your own buffer...

    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    // Image info data...

    for(int i = 0; i < swc_images.size(); ++i) {
        VkImageMemWrap img_mem;

        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        img_mem.img = view_info.image = swc_images[i];

        sm_key img_view_h = vkapp.img_views.push_back(VkImageView());
        img_mem.view_handles.push_back(img_view_h);
        scw.img_views.push_back(img_view_h);
        vkCreateImageView(dvc, &view_info, nullptr, &vkapp.img_views[img_mem.view_handles.back()]);

        scw.images.push_back(vkapp.images.push(img_mem));
    }

    AddSemaphore(logical_dvc_handle, scw.img_semaphore_h);
    scw.img_semaphore = vkapp.semaphores[scw.img_semaphore_h];
    AddFence(logical_dvc_handle, scw.img_fence_h);
    scw.img_fence = vkapp.fences[scw.img_fence_h];

    // DEBUG INFO
    sm_key scw_info_k = swapchain_handle;
    vkapp_info.swapchains.insert(scw_info_k, info);

}

void mfly::vk::TriggerResizeSwapchain(sm_key swapchain_handle, VkExtent2D area) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    swc_wrap.need_resize = true;
    swc_wrap.area = area;
    vkapp_info.swapchains[swapchain_handle].imageExtent = area;
    for(int i = 0; i < swc_wrap.fb_infos.size(); ++i) {
        vkapp_info.framebuffers[i].extent = area;
    }
}

void mfly::vk::RecreateSwapchain(sm_key& swapchain_handle) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    std::vector<VkFramebufferInfoWrap> fb_infos;
    for(sm_key& fb_info_h : swc_wrap.fb_infos)
        fb_infos.push_back(vkapp_info.framebuffers[fb_info_h]);
    DestroySwapchain(swc_wrap);
    VkDevice dvc = vkapp.logical_dvcs[swc_wrap.logical_dvc_handle];

    
    CreateSwapchain(swapchain_handle, vkapp_info.swapchains[swapchain_handle], swc_wrap.surface_handle, swc_wrap.logical_dvc_handle);
    //VkSwapchainWrap& new_swc_wrap = vkapp.swapchains[new_swc];
    // Now destroy and recreate framebuffers

    for(int i = 0; i < fb_infos.size(); ++i) {
        AddSWCFramebuffer(swc_wrap.logical_dvc_handle, fb_infos[i], swapchain_handle);
    }
    // TODO: Recreate Renderpasses -> Image Change

    swc_wrap.need_resize = false;

}

void mfly::vk::DestroySwapchain(VkSwapchainWrap& swc_wrap) {
    VkDevice dvc = vkapp.logical_dvcs[swc_wrap.logical_dvc_handle];
    vkDeviceWaitIdle(dvc);
    
    vkDestroySwapchainKHR(dvc, swc_wrap.swapchain, nullptr);
    // Destroying a swapchain already destroys the swapchain images...
    // Thus they should be popped out of the pool of images and not deleted appart...
    for (auto img : swc_wrap.images) {
        std::optional<VkImageMemWrap> i = vkapp.images.pop(img);
        //if(i.has_value()) vkDestroyImage(dvc, i->img, nullptr);
    }
    swc_wrap.images.clear();

    for (auto imgview : swc_wrap.img_views) {
        std::optional<VkImageView> v = vkapp.img_views.pop(imgview);
        if(v.has_value()) vkDestroyImageView(dvc, *v, nullptr);
    }
    swc_wrap.img_views.clear();

    for (auto fb_k : swc_wrap.framebuffers) {
        std::optional<VkFramebufferWrap> fb = vkapp.framebuffers.pop(fb_k);
        if(fb.has_value()) vkDestroyFramebuffer(dvc, fb->framebuffer, nullptr);
    }
    swc_wrap.framebuffers.clear();

    for(auto fb_info_h : swc_wrap.fb_infos) {
        vkapp_info.framebuffers.pop(fb_info_h);
    }
    swc_wrap.fb_infos.clear();

    //vkDestroySemaphore(dvc, swc_wrap.img_semaphore, nullptr);
    //vkapp.semaphores.pop(swc_wrap.img_semaphore_h);

    //vkDestroyFence(dvc, swc_wrap.img_fence, nullptr);
    //vkapp.fences.pop(swc_wrap.img_fence_h);
    
}