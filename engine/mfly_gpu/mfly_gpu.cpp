#include "mfly_gpu.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_funcs.hpp>

VkApplicationInfo application_info = {};
VkInstanceCreateInfo create_info = {};
VkInstance instance;
const char* instance_extensions[2] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
const char* enabled_layers[1] = {"VK_LAYER_KHRONONS_validation"};
// Extensions are super important
// Such as vkCreateDebugUtilsMessesngerEXT with extension name VK_EXT_debug_utils
// Extensions usually use VK_EXT while builtin use VK_KHR
// Some extensions have dependencies!
// They might also need configuration structs!
// Have to add create_info.pNext = extension
// Then chain them from feature create_info to create info
// They also have to be in order of description in the enabled_extensions[] array

uint32_t vkPDeviceCount;
VkPhysicalDevice* physical_devices = nullptr;

float queue_prio = 1.f;
VkDeviceQueueCreateInfo queue_create_info = {};
// Queues are bound to a family perpetually, with specific amount
// Receive and describe commands to be processed to the device
// Commands in a single queue can be processed parallelly
// Multiples Queues help parallellism -> good when they are not dependant
// You can create multiples queues when creating a single logical device
// Good to specialize the queues -> Transfer, async,...
// GPUInfo and tools by Sascha Willems https://github.com/SaschaWillems/Vulkan

VkDeviceCreateInfo device_create_info = {};
VkDevice device;
const char* device_extensions[1] = {"VK_KHR_swapchain"};
// GPUs have driver and isntalled extensions -> Varies from Device to instance
// Much more per GPU than per instance *Ray Tracing for example)
// 

// Module Basics
uint16_t mfly::gpu::Init()
{
    uint16_t ret = 0;

    // Declare Vulkan application
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = VK_API_VERSION_1_3;

    
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = sizeof(instance_extensions)/sizeof(char*);
    create_info.ppEnabledExtensionNames = instance_extensions;
    create_info.enabledLayerCount = 0;//sizeof(enabled_layers)/sizeof(char*);
    create_info.ppEnabledLayerNames = nullptr; //enabled_layers;

    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    printf("VkResult is: %d\n", result); 

    vkEnumeratePhysicalDevices(instance, &vkPDeviceCount, nullptr);
    physical_devices = new VkPhysicalDevice[vkPDeviceCount];
    vkEnumeratePhysicalDevices(instance, &vkPDeviceCount, physical_devices);
    //assert(vkPDeviceCount > 0);
    VkPhysicalDevice dev  = physical_devices[0];

    // vkEnumerateDeviceExtensionProperties(dev, ...)
    // vlGetPhysicalDeviceProperties(dev, ...)
    // They are a good instance on how the library mostly returns info
    // YOu give iethe r set value in __count and retrive info a struct set to that size
    // or you pass a null in all poitners and then then pointer will be used to tell how many there are
    // then do a 2nd call and retrive the intended amount of info
    // https://stackoverflow.com/questions/37662614/calling-vkenumeratedeviceextensionproperties-twice-is-it-required

    
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = 0;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_prio;

    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = device_extensions;

    result = vkCreateDevice(dev, &device_create_info, nullptr, &device);
    printf("VkResult is: %d\n", result); 

    return ret;
}

uint16_t mfly::gpu::Close()
{
    uint16_t ret = 0;
    delete[] physical_devices; // memleak...
    return ret;
}

uint16_t mfly::gpu::PreUpdate()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::gpu::DoUpdate()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::gpu::PostUpdate()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::gpu::AsyncDispatch()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::gpu::AsyncGather()
{
    uint16_t ret = 0;

    return ret;
}

// Module Specificss