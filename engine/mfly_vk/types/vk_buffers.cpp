#include "vk_buffers.h"
#include "../mfly_vk.hpp"

uint32_t mfly::vk::CreateBuffer(VkBuffer_InitInfo info)
{
    info.buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    vkapp.buffers.push_back(VkBufferMemWrap());
    VkBufferMemWrap &buf = vkapp.buffers.back();
    VkDevice &dvc = vkapp.logical_dvcs[0]; // TODO: Select logical dvc properly
    vkCreateBuffer(dvc, &info.buffer, nullptr, &buf.buffer);

    uint64_t use_offset = 0;
    VkMem_InitInfo mem_info = {};
    vkGetBufferMemoryRequirements(dvc, buf.buffer, &mem_info.mem);

    if (info.handles.mem_handle == UINT32_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size() - 1;
        VkAllocMemWrap &mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());

        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size() - 1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?
        vkapp_info.mem_allocs.push_back(mem_info);
        info.handles.mem_info_handle = vkapp_info.mem_allocs.size() - 1;
    }
    else
    { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
        buf.mem_handle = info.handles.mem_handle;
    }
    vkBindBufferMemory(dvc, buf.buffer, vkapp.mem[info.handles.mem_handle].mem, use_offset); // Offset? What?

    // Create View for Texel Buffers
    if (info.buffer.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | info.buffer.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        for (auto view_info : info.views)
        {
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = buf.buffer;

            buf.views.push_back(VkBufferView());
            vkCreateBufferView(dvc, &view_info, nullptr, &buf.views.back());
        }
    }

    // DEBUG INFO
    vkapp_info.buffers.push_back(info);

    return vkapp.buffers.size() - 1;
}
