#ifndef MFLY_VK_CMDS
#define MFLY_VK_CMDS

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>

namespace mfly::vk {
     struct VkCmdPoolInfoWrap {
        uint32_t flags = 0; // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = commands are recorded frequently // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = commands recorded in entire group
        uint32_t queue_family = 0; // 0 is graphics family
    };
    sm_key AddCmdPool(sm_key& dvc_handle, VkCmdPoolInfoWrap info);

    struct VkCmdBufInfoWrap {
        sm_key pool_handle;
        uint32_t level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VK_COMMAND_BUFFER_LEVEL_PRIMARY = Submittable - not callable // VK_COMMAND_BUFFER_LEVEL_SECONDARY = Not Submittable - callable
        // Huh?
        uint32_t count = 1;
    };
    std::vector<sm_key> AddCmdBuffers(sm_key& dvc_handle, VkCmdBufInfoWrap info);

    struct VkBeginInfoWrap {
        uint32_t flags = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = Rerecorder after being submitted 
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = Secondary and will only be used within a single render passs
        // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = will be resubmitted multiple times during execution
        sm_key parent_handle;
    };

    sm_key AddRecordBegin(VkBeginInfoWrap info);
    VkCommandBuffer BeginRecord(sm_key& begin_handle, sm_key& cmd_buf_handle);
}

#endif