#include "vk_cmds.h"
#include "../mfly_vk.hpp"

using namespace mfly;

sm_key mfly::vk::AddCmdPool(sm_key& dvc_handle, VkCmdPoolInfoWrap info) {
    sm_key k = vkapp.cmd_pools.push(VkCommandPool());
    VkCommandPool& pool = vkapp.cmd_pools[k];

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = info.flags;
    create_info.queueFamilyIndex = info.queue_family;

    // TODO: Proper device selection
    VkResult res = vkCreateCommandPool(vkapp.logical_dvcs[dvc_handle], &create_info, nullptr, &pool);
    if(res != VK_SUCCESS) printf("Failed to create queue");

    return k;
}

std::vector<sm_key> mfly::vk::AddCmdBuffers(sm_key& dvc_handle, VkCmdBufInfoWrap info) {
    
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vkapp.cmd_pools[info.pool_handle];
    alloc_info.level = (VkCommandBufferLevel)info.level;
    alloc_info.commandBufferCount = info.count;    

    std::vector<VkCommandBuffer> cmd_buffers;
    cmd_buffers.resize(info.count);
    VkResult res = vkAllocateCommandBuffers(vkapp.logical_dvcs[dvc_handle], &alloc_info, cmd_buffers.data());
    if(res != VK_SUCCESS) printf("Faield to create command buffers");

    std::vector<sm_key> buf_handles;    
    for(VkCommandBuffer& cmd_buf : cmd_buffers)
        buf_handles.push_back(vkapp.cmd_buffers.push(cmd_buf));

    return buf_handles;
}

sm_key mfly::vk::AddRecordBegin(VkBeginInfoWrap info) {
    sm_key k =  vkapp.begin_cmd_infos.push(VkCommandBufferBeginInfo());
    VkCommandBufferBeginInfo& create_info = vkapp.begin_cmd_infos[k];
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    create_info.flags = info.flags;
    create_info.pInheritanceInfo = nullptr; // TODO: This should extract info from a CommandBufferWrap of a primary command buffer... 
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferInheritanceInfo.html

    return k;
}

VkCommandBuffer mfly::vk::BeginRecord(sm_key& begin_handle, sm_key& cmd_buf_handle) {
    VkCommandBufferBeginInfo info = vkapp.begin_cmd_infos[begin_handle];
    VkCommandBuffer cmd_buf = vkapp.cmd_buffers[cmd_buf_handle];

    vkResetCommandBuffer(cmd_buf, 0);
    VkResult res = vkBeginCommandBuffer(cmd_buf, &info);
    if(res != VK_SUCCESS) printf("Failed to begin command buffer");
    return cmd_buf;
}
