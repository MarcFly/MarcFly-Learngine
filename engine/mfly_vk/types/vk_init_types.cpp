#include "vk_init_types.h"
#include "../mfly_vk.hpp"

using namespace mfly;

sm_key mfly::vk::InitVkInstance(VkInstance_InitInfo& info)
{
    // Declare Vulkan application
    info.app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.app_info.apiVersion = VK_API_VERSION_1_3;

    info.create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // On MAC using MoltenVK or similar porting, flag and extension:
    // info.create_info.flags |=  VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    // info.exts.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // Tidbit -> emplace_back == push_back but better -> No inferred copy on constructors or data not in scope stack!
    // example: string s; vector.emplace_back(s) -> s is copied because is in scope stack
    // example2: vector.emplace_back(std::string("NewString"); -> Moved to vector scope so no copy!

    info.create_info.enabledExtensionCount = info.exts.size();
    info.create_info.ppEnabledExtensionNames = info.exts.data();

    info.create_info.enabledLayerCount = info.layers.size();
    info.create_info.ppEnabledLayerNames = info.layers.data(); // enabled_layers;

    // Missing Extension create info linking
    // Learn about Allocation Callbacks
    VkResult result = vkCreateInstance(&info.create_info, nullptr, &vkapp.instance);
    assert(result == 0);

    printf("VkResult is: %d\n", result);

    uint32_t phys_dvc_count;
    std::vector<VkPhysicalDevice> phys_dvcs;
    vkEnumeratePhysicalDevices(vkapp.instance, &phys_dvc_count, nullptr);
    phys_dvcs.resize(phys_dvc_count);
    vkEnumeratePhysicalDevices(vkapp.instance, &phys_dvc_count, phys_dvcs.data());
    for(VkPhysicalDevice& pdvc : phys_dvcs)
        vkapp.phys_dvcs.push(pdvc);

    // DEBUG INFO
    vkapp_info.info = info;

    for (int i = 0; i < vkapp.phys_dvcs.size(); ++i) {
        sm_key pdvc_k = vkapp_info.p_dvcs.push_back(VkPDVC_InitInfo());
        VkPDVC_InitInfo &dvc_info = vkapp_info.p_dvcs[pdvc_k];
        vkGetPhysicalDeviceProperties(vkapp.phys_dvcs[i], &dvc_info.properties);
        vkGetPhysicalDeviceFeatures(vkapp.phys_dvcs[i], &dvc_info.features);
    }

    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
    vkapp_info.info.available_exts.resize(ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, vkapp_info.info.available_exts.data());

    return vkapp.phys_dvcs.GetKeyAtIDX(0);
}

void mfly::vk::InitQueues(VkLDVC_InitInfo &info, sm_key phys_device_handle)
{
    uint32_t size = info.queues_info.size();
    info.queues_info.back().pQueuePriorities = info.prios.back().data();
    for (int i = size - 2; i > 0; --i)
    {
        info.queues_info[i].pNext = &info.queues_info[i + 1];
        info.queues_info[i].pQueuePriorities = info.prios[i].data();
    }

    sm_key ldvc_key = vkapp.logical_dvcs.push_back(VkDevice());
    VkDevice* device = &vkapp.logical_dvcs[ldvc_key];

    info.create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.create_info.queueCreateInfoCount = info.queues_info.size();
    info.create_info.pQueueCreateInfos = info.queues_info.data();
    info.create_info.enabledExtensionCount = info.exts.size();
    info.create_info.ppEnabledExtensionNames = info.exts.data();
    if (info.exts_info.size() > 0)
    {
        info.create_info.pNext = &info.exts_info[0];
        for (int i = 1; i<info.exts_info.size()> 1; ++i)
        {
            // Know that 2nd field of every extension info is pNext a Void*
            // then memcpy into that the pointer to the next one
            char **curr_info = (char **)&info.exts_info[i - 1];     // Retrieve last info
            char **pNext = &(*curr_info) + sizeof(VkStructureType); // Get to Byte where pNext starts
            memcpy(&pNext, &info.exts_info[i], sizeof(void *));
        }
    }

    VkResult result = vkCreateDevice(vkapp.phys_dvcs[phys_device_handle], &info.create_info, nullptr, device);
    assert(result == 0);
    printf("VkResult is: %d\n", result);

    // DEBUG INFO
    vkapp_info.l_dvcs.push_back(info);
}