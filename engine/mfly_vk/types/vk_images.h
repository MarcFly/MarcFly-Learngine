#ifndef MFLY_VK_IMAGES
#define MFLY_VK_IMAGES

#include <vulkan/vulkan.hpp>
#include "vk_buffers.h"
#include <mfly_slotmap.h>

namespace mfly::vk {
    struct VkImage_InitInfo {
        VkImageCreateInfo img = {};
        VkDMEMHandles handles;
        std::vector<VkImageViewCreateInfo> views = {};
    };
    sm_key CreateImage(VkImage_InitInfo info);
    struct VkImageMemWrap
    {
        VkImage img;
        std::vector<sm_key> view_handles;
        sm_key mem_handle;
    };

    //==============================================

    struct VkSwapchainWrap
    {
        VkSwapchainKHR swapchain;
        sm_key logical_dvc_handle;
        sm_key surface_handle;
        bool need_resize = false;

        std::vector<sm_key> images;
        std::vector<sm_key> img_views;
        VkExtent2D area;
        uint32_t curr_image;

        sm_key img_semaphore_h;
        VkSemaphore img_semaphore;
        sm_key img_fence_h;
        VkFence img_fence;

        std::vector<sm_key> framebuffers; // Should be associated so that they are recreated with the swapchain
        std::vector<sm_key> fb_infos;
    };

    void TriggerResizeSwapchain(sm_key swapchain_handle, VkExtent2D area);
    void CreateSwapchain(sm_key& swapchain_handle, VkSwapchainCreateInfoKHR info, sm_key surface_handle, sm_key logical_dvc_handle);
    void RecreateSwapchain(sm_key& swapchain_handle);
    void DestroySwapchain(VkSwapchainWrap& swc_wrap);

    //===================================================

    struct VkFramebufferWrap {
        VkFramebuffer framebuffer;
        sm_key img_view_handle;
    };

    struct VkFramebufferInfoWrap {
        sm_key render_pass_handle;
        std::vector<sm_key> img_view_handles; // Framebuffers are linked to previously created IMAGES (agora sim entendo)
        VkExtent2D extent;
        uint32_t num_layers;
    };

    sm_key AddFramebuffer(sm_key& dvc_handle, VkFramebufferInfoWrap fb_info);
    sm_key AddSWCFramebuffer(sm_key& dvc_handle, VkFramebufferInfoWrap info, sm_key& swapchain_handle);
        

};

#endif