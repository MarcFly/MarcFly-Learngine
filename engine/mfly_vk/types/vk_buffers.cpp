#include "vk_buffers.h"
#include "../mfly_vk.hpp"

using namespace mfly;

uint32_t mfly::vk::AllocMem(VkDMEMHandles& handles, VkMem_InitInfo& mem_info) {
    uint64_t use_offset = 0;
    VkDevice& dvc = vkapp.logical_dvcs[handles.logical_dvc_handle];

    if (!vkapp.mem.has_key(handles.mem_handle))
    {
        handles.mem_handle = vkapp.mem.push_back(VkAllocMemWrap());
        VkAllocMemWrap &mem = vkapp.mem[handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???

        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &mem.mem);
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?
        handles.mem_handle = vkapp_info.mem_allocs.push_back(mem_info);
    }
    else
    { // Using a known allocation
        VkAllocMemWrap &mem = vkapp.mem[handles.mem_handle];
        use_offset = mem.next_offset;
        mem.next_offset += mem_info.mem.size;
        //buf.mem_handle = info.handles.mem_handle;
    }

    return use_offset;
}

sm_key mfly::vk::CreateBuffer(VkBuffer_InitInfo info)
{
    info.buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    sm_key buf_key = vkapp.buffers.push_back(VkBufferMemWrap());
    VkBufferMemWrap &buf = vkapp.buffers[buf_key];
    VkDevice &dvc = vkapp.logical_dvcs[info.handles.logical_dvc_handle]; // TODO: Select logical dvc properly
    vkCreateBuffer(dvc, &info.buffer, nullptr, &buf.buffer);

    
    VkMem_InitInfo mem_info = {};
    vkGetBufferMemoryRequirements(dvc, buf.buffer, &mem_info.mem);

    uint64_t use_offset = AllocMem(info.handles, mem_info); 
    buf.mem_handle = info.handles.mem_handle;

    vkBindBufferMemory(dvc, buf.buffer, vkapp.mem[info.handles.mem_handle].mem, use_offset); // Offset? What?

    // Create View for Texel Buffers
    if (info.buffer.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | info.buffer.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        for (VkBufferViewCreateInfo& view_info : info.views)
        {
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = buf.buffer;

            buf.views.push_back(VkBufferView());
            vkCreateBufferView(dvc, &view_info, nullptr, &buf.views.back());
        }
    }

    // DEBUG INFO
    vkapp_info.buffers.push_back(info);

    return buf_key;
}
