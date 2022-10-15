#ifndef MFLY_GPU
#define MFLY_GPU

#include <stdint.h>
#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>
#include <vector>
#include "types/vk_init_types.h"
#include "types/vk_shaders.h"
#include "types/vk_images.h"
#include "types/vk_buffers.h"
#include "types/vk_gfx_pipe.h"
#include "types/vk_cmds.h"
#include "types/vk_renderpass.h"

namespace mfly::vk
{

        // Module Functions
        uint32_t DefaultInit();
        
        
        // When adding commands seems like everyone requires the same commandbuffer
        // Would be sensible to pass the actual data instead of a handle to avoid fetches to foreign memory?
        uint32_t Close();
        uint32_t PreUpdate();
        uint32_t DoUpdate();
        uint32_t PostUpdate();
        uint32_t AsyncDispatch();
        uint32_t AsyncGather();

        //uint32_t event_code_start = 0;
        uint32_t SwapchainNextImage(sm_key& swapchain_handle);
        enum ERRORCODE
        {
            GOOD = 0,
            BAD = 1,
        };
#if defined(DEBUG_STRINGS)
        const char*[] debug_strings
        {
            "Good, move on.",
            "Something went wrong, message not set",
        }
        const char* DebugMessage(uint32_t errorcode) {
            if(errorcode > (sizeof(debug_string)/sizeof(const char*)))
                return debug_string[1];
            else
                return debug_string[errorcode];
        }
#endif

        // Module Specifics
        /// void*: VkInstance the library provides and the window api most likely requires
        /// uint32_t: Window number that we want a surface from (usually the main window - 0)
        /// expected result: VkSurfaceKHR
        typedef void*(*GetSurfaceFun)(void*, uint32_t);
        void ProvideSurfaceFun(GetSurfaceFun fun);

        VkSemaphore& AddSemaphore(sm_key& dvc_handle, sm_key& semaphore_handle);
        VkFence& AddFence(sm_key& dvc_handle, sm_key& fence_handle);


    //===================================
    struct VkApp {
        VkInstance instance;

        mfly::slotmap<VkPhysicalDevice> phys_dvcs;
        mfly::slotmap<VkDevice> logical_dvcs;

        mfly::slotmap<VkBufferMemWrap> buffers;
        mfly::slotmap<VkAllocMemWrap> mem;
        mfly::slotmap<VkImageMemWrap> images;
        mfly::slotmap<VkImageView> img_views; // Separate as used for Images, Framebuffers, Attachment, Swapchain Images,...

        VkSurfaceKHR main_surface;
        mfly::slotmap<VkSurfaceKHR> surfaces;
        GetSurfaceFun get_surface = nullptr;

        mfly::slotmap<VkSwapchainWrap> swapchains;

        mfly::slotmap<VkShaderModuleWrap> shaders;

        mfly::slotmap<VkFramebufferWrap> framebuffers;
        mfly::slotmap<VkAttachmentDescription> attachment_descs;

        mfly::slotmap<VkGraphicsPipelineWrap> graphic_pipes;

        mfly::slotmap<VkSubpassDescWrap> subpasses;

        mfly::slotmap<VkRenderPass> render_passes;
        mfly::slotmap<VkRenderPassInfoWrap> render_pass_infos;

        mfly::slotmap<VkCommandPool> cmd_pools;
        mfly::slotmap<VkCommandBuffer> cmd_buffers;
        VkCommandBuffer* curr_cmd_buffer = nullptr;
        mfly::slotmap<VkCommandBufferBeginInfo> begin_cmd_infos;
        mfly::slotmap<VkRenderPassBeginInfoWrap> begin_renderpass_infos;

        mfly::slotmap<VkSemaphore> semaphores;
        mfly::slotmap<VkFence> fences;
    };

    struct VkAppInfo {
        VkInstance_InitInfo info;
        mfly::slotmap<VkPDVC_InitInfo> p_dvcs;
        mfly::slotmap<VkLDVC_InitInfo> l_dvcs;
        mfly::slotmap<VkSwapchainCreateInfoKHR> swapchains;
        mfly::slotmap<VkBuffer_InitInfo> buffers;
        mfly::slotmap<VkMem_InitInfo> mem_allocs;
        mfly::slotmap<VkImage_InitInfo> imgs;
        mfly::slotmap<VkFramebufferInfoWrap> framebuffers;

    };

    //==============================================================
    void ExampleBuffersImages();
};

extern mfly::vk::VkApp vkapp;
extern mfly::vk::VkAppInfo vkapp_info;

#endif // MFLY_GPU