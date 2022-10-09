#ifndef MFLY_VK_BUFFER_TYPES
#define MFLY_VK_BUFFER_TYPES

#include <vulkan/vulkan.hpp>

namespace mfly::vk {
    struct VkMem_InitInfo {
            VkMemoryRequirements mem = {};
            VkMemoryAllocateInfo mem_info = {};
        };
        // Memory Allocation?
        struct VkDMEMHandles {
            uint32_t logical_dvc_handle = 0;
            uint32_t mem_handle = -1;
            uint32_t mem_info_handle = -1;
        };
        struct VkBuffer_InitInfo {
            VkBufferCreateInfo buffer = {};
            VkDMEMHandles handles;
            std::vector<VkBufferViewCreateInfo> views;
        };
        
        uint32_t CreateBuffer(VkBuffer_InitInfo info);
        

    struct VkBufferMemWrap
    {
        VkBuffer buffer;
        std::vector<VkBufferView> views;
        uint32_t mem_handle = -1;
    };

    struct VkAllocMemWrap
    {
        VkDeviceMemory mem;
        uint64_t next_offset;
        uint64_t size;
    };


};

#endif