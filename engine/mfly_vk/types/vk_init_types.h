#ifndef MFLY_VK_INIT_TYPES
#define MFLY_VK_INIT_TYPES

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>

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
    // TODO: Function to find best candidate based on Extensions, Layers
    
    // Returns the first physical device found, should use function to find best candidate instead
    sm_key InitVkInstance(VkInstance_InitInfo& instance_info); // Startup Vulkan


    //========================================================

    struct VkLDVC_InitInfo {
        std::vector<VkDeviceQueueCreateInfo> queues_info;
        std::vector<std::vector<float>> prios;
        VkDeviceCreateInfo create_info = {};
        std::vector<const char*> exts;
        std::vector<void*> exts_info;
    };

    sm_key InitQueues(VkLDVC_InitInfo& info, sm_key phys_device_handle);

    struct VkQueueWrap
    {
        float queue_prio = 1.f;
        VkDeviceQueueCreateInfo queue_create_info = {};
    };


};

#endif