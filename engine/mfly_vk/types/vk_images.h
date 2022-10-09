#ifndef MFLY_VK_IMAGES
#define MFLY_VK_IMAGES

#include <vulkan/vulkan.hpp>
#include "vk_buffers.h"

namespace mfly::vk {
    struct VkImage_InitInfo {
        VkImageCreateInfo img = {};
        VkDMEMHandles handles;
        std::vector<VkImageViewCreateInfo> views = {};
    };
    uint32_t CreateImage(VkImage_InitInfo info);
    struct VkImageMemWrap
    {
        VkImage img;
        std::vector<uint32_t> view_handles;
        uint32_t mem_handle = -1;
    };

    //===================================================
    struct VkFramebufferWrap {
        VkFramebuffer framebuffer;
        uint32_t img_view_handle;
    };

    struct VkFramebufferInfoWrap {
        uint32_t render_pass_handle;
        std::vector<uint32_t> img_view_handles; // Framebuffers are linked to previously created IMAGES (agora sim entendo)
        VkExtent2D extent;
        uint32_t num_layers;
    };

    uint32_t AddFramebuffer(VkFramebufferInfoWrap fb_info, uint32_t existing = UINT32_MAX);
    uint32_t AddSWCFramebuffer(VkFramebufferInfoWrap info, uint32_t swapchain_handle, uint32_t existing = UINT32_MAX);
        
    //==============================================

    struct VkSwapchainWrap
    {
        VkSwapchainKHR swapchain;
        uint32_t logical_dvc_handle;
        uint32_t surface_handle;
        bool need_resize = false;

        std::vector<VkImage> images;
        std::vector<VkImageView> img_views;
        VkExtent2D area;
        uint32_t curr_image;

        uint32_t img_semaphore_h = UINT32_MAX;
        VkSemaphore img_semaphore;
        uint32_t img_fence_h = UINT32_MAX;
        VkFence img_fence;

        std::vector<uint32_t> framebuffers; // Should be associated so that they are recreated with the swapchain
        std::vector<uint32_t> fb_infos;
    };

    void TriggerResizeSwapchain(uint32_t swapchain_handle, VkExtent2D area);
    uint32_t CreateSwapchain(VkSwapchainCreateInfoKHR info, uint32_t surface_handle, uint32_t logical_dvc_handle, uint32_t existing = UINT32_MAX);
    void RecreateSwapchain(uint32_t swapchain_handle);
    void DestroySwapchain(VkSwapchainWrap& swc_wrap);

};

#endif