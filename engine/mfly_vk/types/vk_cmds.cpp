#include "vk_cmds.h"
#include "../mfly_vk.hpp"


uint32_t mfly::vk::AddCmdPool(VkCmdPoolInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.cmd_pools, existing);
    VkCommandPool* pool = &vkapp.cmd_pools[existing];

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = info.flags;
    create_info.queueFamilyIndex = info.queue_family;

    // TODO: Proper device selection
    VkResult res = vkCreateCommandPool(vkapp.logical_dvcs[0], &create_info, nullptr, pool);
    if(res != VK_SUCCESS) printf("Failed to create queue");

    return existing;
}

uint32_t mfly::vk::AddCmdBuffers(VkCmdBufInfoWrap info) {
    uint32_t first = vkapp.cmd_buffers.size();
    for(int i = 0; i < info.count; ++i)
        PushNonInvalid(vkapp.cmd_buffers, UINT32_MAX);
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vkapp.cmd_pools[info.pool_handle];
    alloc_info.level = (VkCommandBufferLevel)info.level;
    alloc_info.commandBufferCount = info.count;    

    // TODO: Properly select logical dvc
    VkResult res = vkAllocateCommandBuffers(vkapp.logical_dvcs[0], &alloc_info, &vkapp.cmd_buffers[first]);
    if(res != VK_SUCCESS) printf("Faield to create command buffers");

    return first;
}

uint32_t mfly::vk::AddRecordBegin(VkBeginInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.begin_cmd_infos, existing);
    VkCommandBufferBeginInfo& create_info = vkapp.begin_cmd_infos[existing];
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    create_info.flags = info.flags;
    create_info.pInheritanceInfo = nullptr; // TODO: This should extract info from a CommandBufferWrap of a primary command buffer... 
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferInheritanceInfo.html

    return existing;
}

VkCommandBuffer mfly::vk::BeginRecord(uint32_t begin_handle, uint32_t cmd_buf_handle) {
    VkCommandBufferBeginInfo info = vkapp.begin_cmd_infos[begin_handle];
    VkCommandBuffer cmd_buf = vkapp.cmd_buffers[cmd_buf_handle];

    vkResetCommandBuffer(cmd_buf, 0);
    VkResult res = vkBeginCommandBuffer(cmd_buf, &info);
    if(res != VK_SUCCESS) printf("Failed to begin command buffer");
    return cmd_buf;
}
