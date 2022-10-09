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

        std::pair<uint32_t, VkSemaphore> AddSemaphore(uint32_t existing);
        std::pair<uint32_t, VkFence> AddFence(uint32_t existing);


    //===================================
    struct VkApp {
        VkInstance instance;

        std::vector<VkPhysicalDevice> phys_dvcs;
        std::vector<VkDevice> logical_dvcs;

        std::vector<VkBufferMemWrap> buffers;
        std::vector<VkAllocMemWrap> mem;
        std::vector<VkImageMemWrap> images;
        std::vector<VkImageView> img_views; // Separate as used for Images, Framebuffers, Attachment, Swapchain Images,...

        VkSurfaceKHR main_surface;
        std::vector<VkSurfaceKHR> surfaces;
        GetSurfaceFun get_surface = nullptr;

        std::vector<VkSwapchainWrap> swapchains;

        std::vector<VkShaderModuleWrap> shaders;

        std::vector<VkFramebufferWrap> framebuffers;
        std::vector<VkAttachmentDescription> attachment_descs;

        std::vector<VkGraphicsPipelineWrap> graphic_pipes;

        std::vector<VkSubpassDescWrap> subpasses;

        std::vector<VkRenderPass> render_passes;
        std::vector<VkRenderPassInfoWrap> render_pass_infos;

        std::vector<VkCommandPool> cmd_pools;
        std::vector<VkCommandBuffer> cmd_buffers;
        VkCommandBuffer* curr_cmd_buffer = nullptr;
        std::vector<VkCommandBufferBeginInfo> begin_cmd_infos;
        std::vector<VkRenderPassBeginInfoWrap> begin_renderpass_infos;

        std::vector<VkSemaphore> semaphores;
        std::vector<VkFence> fences;
    };

    struct VkAppInfo {
        VkInstance_InitInfo info;
        std::vector<VkPDVC_InitInfo> p_dvcs;
        std::vector<VkLDVC_InitInfo> l_dvcs;
        std::vector<VkSwapchainCreateInfoKHR> swapchains;
        std::vector<VkBuffer_InitInfo> buffers;
        std::vector<VkMem_InitInfo> mem_allocs;
        std::vector<VkImage_InitInfo> imgs;
        std::vector<VkFramebufferInfoWrap> framebuffers;

    };
};

template<class T>
inline uint32_t PushNonInvalid(std::vector<T>& vec, int64_t check_val) {
    int64_t v = vec.size()-1;
    if(check_val > v){
        vec.push_back(T());
        check_val = vec.size()-1;
    }

    return check_val;
}

template<class T>
inline void VecFromHandles(const std::vector<uint32_t> handles, const std::vector<T>& data, std::vector<T>& out) {
    out.clear();
    for(auto h : handles) {
        out.push_back(data[h]);
    }
}

extern mfly::vk::VkApp vkapp;
extern mfly::vk::VkAppInfo vkapp_info;

#endif // MFLY_GPU