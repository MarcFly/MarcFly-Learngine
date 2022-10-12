#ifndef MFLY_VK_BUFFER_TYPES
#define MFLY_VK_BUFFER_TYPES

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>

namespace mfly::vk {
    struct VkMem_InitInfo {
        VkMemoryRequirements mem = {};
        VkMemoryAllocateInfo mem_info = {};
    };
    // Memory Allocation?
    struct VkDMEMHandles {
        sm_key logical_dvc_handle;
        sm_key mem_handle;
        sm_key mem_info_handle;
    };
    struct VkBuffer_InitInfo {
        VkBufferCreateInfo buffer = {};
        VkDMEMHandles handles;
        mfly::slotmap<VkBufferViewCreateInfo> views;
    };
    
    sm_key CreateBuffer(VkBuffer_InitInfo info);
        

    struct VkBufferMemWrap
    {
        VkBuffer buffer;
        mfly::slotmap<VkBufferView> views;
        sm_key mem_handle;
    };

    struct VkAllocMemWrap
    {
        VkDeviceMemory mem;
        uint64_t next_offset;
        uint64_t size;
    };

    // Returns the point of memory from which to do the allocation
    // Aka use_offset
    uint32_t AllocMem(VkDMEMHandles& handles, VkMem_InitInfo& mem_info);

};

#endif