#ifndef MFLY_VK_RENDERPASS
#define MFLY_VK_RENDERPASS

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>

namespace mfly::vk {
    // Not usable?
    struct VkAttachmentInfoWrap {
        uint32_t format = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat...
        uint32_t samples = VK_SAMPLE_COUNT_1_BIT; // 1 Bit per sample VkSampleCountFlagBits
        struct {
            uint32_t load = VK_ATTACHMENT_LOAD_OP_CLEAR; // What to do before rendering
            uint32_t store = VK_ATTACHMENT_STORE_OP_STORE; // What to do after rendering
            uint32_t stencil_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            uint32_t stencil_store = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        } ops;
        VkImageLayout input_layout = VK_IMAGE_LAYOUT_UNDEFINED; // Which type of image comes, not needed but can be optimized if known
        VkImageLayout output_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image for a swapchain
        // CHANGE for color attachment or transfer/copy operations!
    };     
    void AddAttachmentDesc(sm_key& subpass_key, VkAttachmentInfoWrap info);

    struct VkSubPassInfoWrap {
        std::vector<VkAttachmentReference> framebuffers;
        // Bind them depending on the pipeline they reference
        std::vector<VkAttachmentReference> inputs;
        VkAttachmentReference depth_stencil[2];

        // Not used for now at all
        std::vector<VkAttachmentReference> sampling_resolves;
        std::vector<uint32_t> preserve; // TODO: Reasearch preserve attachments from VkSubpassDescription
    };
    void AddSubPass(sm_key& subpass_handle, VkSubPassInfoWrap& info); // Invalidates vectors!
    
    struct VkSubPassWrap {
        VkSubpassDescription subpass;
        VkSubPassInfoWrap info;
    };

    struct VkSubpassDescWrap {
        VkSubpassDescription desc;
        VkSubPassInfoWrap info;
    };

    //====================================================

    struct VkRenderPassInfoWrap {
        std::vector<sm_key> attachment_handles;
        std::vector<sm_key> subpass_handles;
        sm_key pipeline_handle;
    };


    void CreateRenderPass(sm_key& renderpasshandle, sm_key& renderpass_info_handle, VkRenderPassInfoWrap info);
    void RegenRenderPass(sm_key& handle); // For when known subpasses are changed
    // Ideally, a RegenAllRenderPasses which takes into account versioning of their subpasses?
    
    struct VkRenderPassWrap {
        VkRenderPass pass;
        VkRenderPassInfoWrap info; // For regen
    };
    
    //=====================================================

    struct VkBeginRenderPassInfoWrap {
        sm_key render_pass_handle;
        sm_key framebuffer_handle;
        VkOffset2D offset;
        VkExtent2D extent;
        std::vector<VkClearValue> clear_colors;
    };
    void AddRenderPassBegin(sm_key& begin_renderpass_handle, VkBeginRenderPassInfoWrap info);
    void BeginRenderPass(sm_key& begin_handle, VkCommandBuffer cmd_buf);
    

    struct VkRenderPassBeginInfoWrap {
        std::vector<VkClearValue> colors;
        VkRenderPassBeginInfo create_info;
    };

};


#endif