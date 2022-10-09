#ifndef MFLY_VK_INIT_TYPES
#define MFLY_VK_INIT_TYPES

#include <vulkan/vulkan.hpp>

namespace mfly::vk {
    struct VkPDVC_InitInfo {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
    };

    struct VkInstance_InitInfo{
        VkApplicationInfo app_info = {};
        VkInstanceCreateInfo create_info = {};
        std::vector<const char*> exts;
        std::vector<const char*> layers;
        
        std::vector<VkExtensionProperties> available_exts;
    };

    uint32_t InitVkInstance(VkInstance_InitInfo& instance_info); // Startup Vulkan


    //========================================================

    struct VkLDVC_InitInfo {
        std::vector<VkDeviceQueueCreateInfo> queues_info;
        std::vector<std::vector<float>> prios;
        VkDeviceCreateInfo create_info = {};
        std::vector<const char*> exts;
        std::vector<void*> exts_info;
    };

    uint32_t InitQueues(VkLDVC_InitInfo& info, uint32_t phys_dvc_handle = 0);

    struct VkQueueWrap
    {
        float queue_prio = 1.f;
        VkDeviceQueueCreateInfo queue_create_info = {};
    };


};

#endif