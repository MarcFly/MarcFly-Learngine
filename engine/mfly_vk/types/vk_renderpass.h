#ifndef MFLY_VK_RENDERPASS
#define MFLY_VK_RENDERPASS

#include <vulkan/vulkan.hpp>

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
    uint32_t AddAttachmentDesc(VkAttachmentInfoWrap info, uint32_t existing = UINT32_MAX);

    struct VkSubPassInfoWrap {
        std::vector<VkAttachmentReference> framebuffers;
        // Bind them depending on the pipeline they reference
        std::vector<VkAttachmentReference> inputs;
        VkAttachmentReference depth_stencil[2];

        // Not used for now at all
        std::vector<VkAttachmentReference> sampling_resolves;
        std::vector<uint32_t> preserve; // TODO: Reasearch preserve attachments from VkSubpassDescription
    };
    uint32_t AddSubPass(VkSubPassInfoWrap& info, uint32_t existing = UINT32_MAX); // Invalidates vectors!
    
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
        std::vector<uint32_t> attachment_handles;
        std::vector<uint32_t> subpass_handles;
        uint32_t pipeline_handle;
    };


    uint32_t CreateRenderPass(VkRenderPassInfoWrap info, uint32_t existing = UINT32_MAX);
    uint32_t RegenRenderPass(uint32_t handle); // For when known subpasses are changed
    // Ideally, a RegenAllRenderPasses which takes into account versioning of their subpasses?
    
    struct VkRenderPassWrap {
        VkRenderPass pass;
        VkRenderPassInfoWrap info; // For regen
    };
    
    //=====================================================

    struct VkBeginInfoWrap {
        uint32_t flags = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = Rerecorder after being submitted 
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = Secondary and will only be used within a single render passs
        // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = will be resubmitted multiple times during execution
        uint32_t parent_handle = UINT32_MAX;
    };
    uint32_t AddRecordBegin(VkBeginInfoWrap info, uint32_t existing = UINT32_MAX);
    VkCommandBuffer BeginRecord(uint32_t begin_handle, uint32_t cmd_buf_handle);

    struct VkBeginRenderPassInfoWrap {
        uint32_t render_pass_handle;
        uint32_t framebuffer_handle;
        VkOffset2D offset;
        VkExtent2D extent;
        std::vector<VkClearValue> clear_colors;
    };
    uint32_t AddRenderPassBegin(VkBeginRenderPassInfoWrap info, uint32_t existing = UINT32_MAX);
    uint32_t BeginRenderPass(uint32_t begin_handle, VkCommandBuffer cmd_buf);
    

    struct VkRenderPassBeginInfoWrap {
        std::vector<VkClearValue> colors;
        VkRenderPassBeginInfo create_info;
    };

};


#endif