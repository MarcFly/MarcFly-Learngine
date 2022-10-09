#include "vk_images.h"
#include "../mfly_vk.hpp"

uint32_t mfly::vk::CreateImage(VkImage_InitInfo info)
{
    // Example Image
    info.img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    vkapp.images.push_back(VkImageMemWrap());
    VkImageMemWrap &img = vkapp.images.back();
    // TODO: Select logical device properly
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkCreateImage(dvc, &info.img, nullptr, &img.img);

    uint64_t use_offset = 0;
    VkMem_InitInfo mem_info = {};
    vkGetImageMemoryRequirements(dvc, img.img, &mem_info.mem);

    if (info.handles.mem_handle == UINT32_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size() - 1;
        VkAllocMemWrap &mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());

        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size() - 1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?
        
        vkapp_info.mem_allocs.push_back(mem_info);
        info.handles.mem_info_handle = vkapp_info.mem_allocs.size() - 1;
    }
    else
    { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
        img.mem_handle = info.handles.mem_handle;
    }

    vkBindImageMemory(dvc, img.img, vkapp.mem[info.handles.mem_handle].mem, use_offset);

    for (auto view_info : info.views)
    {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = img.img;

        img.view_handles.push_back(vkapp.img_views.size());
        vkapp.img_views.push_back(VkImageView());
        
        vkCreateImageView(dvc, &view_info, nullptr, &vkapp.img_views.back());
    }

    // DEBUG INFO
    vkapp_info.imgs.push_back(info);

    return vkapp.images.size() - 1;
}

//========================================================


uint32_t mfly::vk::AddFramebuffer(VkFramebufferInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.framebuffers, existing);
    VkFramebufferWrap& fb_wrap = vkapp.framebuffers[existing];

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    std::vector<VkImageView> img_views;
    VecFromHandles(info.img_view_handles, vkapp.img_views, img_views);
    create_info.attachmentCount = img_views.size();
    create_info.pAttachments = img_views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[0], &create_info, nullptr, &fb_wrap.framebuffer);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    return existing;
}

uint32_t mfly::vk::AddSWCFramebuffer(VkFramebufferInfoWrap info, uint32_t swapchain_handle, uint32_t existing) {
    uint32_t prev_val = existing;
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    
    existing = PushNonInvalid(vkapp.framebuffers, existing);
    swc_wrap.framebuffers[PushNonInvalid(swc_wrap.framebuffers, existing)] = existing;
    VkFramebuffer& fb = vkapp.framebuffers[existing].framebuffer;
    if(prev_val == existing) vkDestroyFramebuffer(vkapp.logical_dvcs[0], fb,nullptr);

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    create_info.attachmentCount = swc_wrap.img_views.size();
    create_info.pAttachments = swc_wrap.img_views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[0], &create_info, nullptr, &fb);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    uint32_t info_existing = PushNonInvalid(vkapp_info.framebuffers, existing);
    uint32_t fb_info_h = PushNonInvalid(swc_wrap.fb_infos, info_existing);
    swc_wrap.fb_infos[fb_info_h] = info_existing;
    VkFramebufferInfoWrap& fb_info = vkapp_info.framebuffers[info_existing];
    fb_info.extent = info.extent;
    for(int i = 0; i < swc_wrap.img_views.size(); ++i)
        info.img_view_handles.push_back(i);
    fb_info.img_view_handles.swap(info.img_view_handles);
    fb_info.num_layers = info.num_layers;
    fb_info.render_pass_handle;

    return existing;
}

//========================================================

uint32_t mfly::vk::CreateSwapchain(VkSwapchainCreateInfoKHR info, uint32_t surface_handle, uint32_t logical_dvc_handle, uint32_t existing)
{
    uint32_t prev_val = existing;
    existing = PushNonInvalid(vkapp.swapchains, existing);
    bool recreate = existing != prev_val;

    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = vkapp.surfaces[surface_handle];
    // There are many more settings, research

    info.minImageCount = 1;
    
    VkSwapchainWrap &scw = vkapp.swapchains[existing];
    scw.logical_dvc_handle = logical_dvc_handle;
    VkDevice &dvc = vkapp.logical_dvcs[logical_dvc_handle];
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkResult res = vkCreateSwapchainKHR(dvc, &info, nullptr, &scw.swapchain);

    scw.area = info.imageExtent;
    scw.surface_handle = surface_handle;

    uint32_t img_count;
    //vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, nullptr);
    img_count = 1;
    scw.images.resize(img_count);
    vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, scw.images.data());

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

    for(int i = 0; i < scw.images.size(); ++i) {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = scw.images[i];

        scw.img_views.push_back(VkImageView());
        
        vkCreateImageView(dvc, &view_info, nullptr, &scw.img_views.back());
    }

    std::pair<uint32_t, VkSemaphore> sem = AddSemaphore(scw.img_fence_h);
    std::pair<uint32_t, VkFence>  fence = AddFence(scw.img_semaphore_h);
    scw.img_semaphore_h = sem.first;
    scw.img_semaphore = sem.second;
    scw.img_fence_h = fence.first;
    scw.img_fence = fence.second;

    // DEBUG INFO
    uint32_t info_id = PushNonInvalid(vkapp_info.swapchains, existing);
    vkapp_info.swapchains[info_id] = info;

    return existing;
}

void mfly::vk::TriggerResizeSwapchain(uint32_t swapchain_handle, VkExtent2D area) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    swc_wrap.need_resize = true;
    swc_wrap.area = area;
    vkapp_info.swapchains[swapchain_handle].imageExtent = area;
    for(int i = 0; i < swc_wrap.fb_infos.size(); ++i) {
        vkapp_info.framebuffers[i].extent = area;
    }
}

void mfly::vk::RecreateSwapchain(uint32_t swapchain_handle) {
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkDeviceWaitIdle(dvc);

    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    swc_wrap.need_resize = false;

    vkDestroySwapchainKHR(dvc, swc_wrap.swapchain, nullptr);

    for (auto imgview : swc_wrap.img_views) {
        vkDestroyImageView(dvc, imgview, nullptr);
    }
    swc_wrap.img_views.clear();

    for (auto img : swc_wrap.images) {
        vkDestroyImage(dvc, img, nullptr);
    }
    swc_wrap.images.clear();
    swc_wrap.framebuffers.clear();
    
    CreateSwapchain(vkapp_info.swapchains[swapchain_handle], swc_wrap.surface_handle, swc_wrap.logical_dvc_handle, swapchain_handle);
    
    // Now destroy and recreate framebuffers

    for(int i = 0; i < swc_wrap.fb_infos.size(); ++i) {
        AddSWCFramebuffer(vkapp_info.framebuffers[swc_wrap.fb_infos[i]], swapchain_handle, i);
    }
    // TODO: Recreate Renderpasses -> Image Change

}

void mfly::vk::DestroySwapchain(VkSwapchainWrap& swc_wrap) {
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkDestroySwapchainKHR(dvc, swc_wrap.swapchain, nullptr);

    for (auto imgview : swc_wrap.img_views) {
        vkDestroyImageView(dvc, imgview, nullptr);
    }
    swc_wrap.img_views.clear();

    for (auto img : swc_wrap.images) {
        vkDestroyImage(dvc, img, nullptr);
    }
    swc_wrap.images.clear();
}