#ifndef MFLY_VK_CMDS
#define MFLY_VK_CMDS

#include <vulkan/vulkan.hpp>

namespace mfly::vk {
     struct VkCmdPoolInfoWrap {
            uint32_t flags = 0; // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = commands are recorded frequently // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = commands recorded in entire group
            uint32_t queue_family = 0; // 0 is graphics family
        };
        uint32_t AddCmdPool(VkCmdPoolInfoWrap info, uint32_t existing = UINT32_MAX);

        struct VkCmdBufInfoWrap {
            uint32_t pool_handle;
            uint32_t level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VK_COMMAND_BUFFER_LEVEL_PRIMARY = Submittable - not callable // VK_COMMAND_BUFFER_LEVEL_SECONDARY = Not Submittable - callable
            // Huh?
            uint32_t count = 1;
        };
        uint32_t AddCmdBuffers(VkCmdBufInfoWrap info);
}

#endif