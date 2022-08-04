#include "mfly_gpu.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_funcs.hpp>

VkApplicationInfo application_info = {};
VkInstanceCreateInfo create_info = {};
VkInstance instance;
const char* instance_extensions[] = {
    "VK_KHR_surface", 
    "VK_KHR_win32_surface"
};
const char* enabled_layers[] = {"VK_LAYER_KHRONOS_validation"};
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
const char* device_extensions[] = {"VK_KHR_swapchain"};
// GPUs have driver and isntalled extensions -> Varies from Device to instance
// Much more per GPU than per instance *Ray Tracing for example)
// 

VkSurfaceKHR surface;
VkSwapchainCreateInfoKHR swapchain_create_info = {};
VkSwapchainKHR swapchain;
VkImage* swapchainImageHandles;
uint32_t image_count;

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
    VkPhysicalDevice dev  = physical_devices[0]; // Grab Pointer to device in use

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

    // Ask window manager to get a surface
    surface = (VkSurfaceKHR)mfly::win::getGAPISurface(0, (vk::Instance)instance);

    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = 4;
    
    // Should use vkGetPhysicalDeviceSurfaceFormatsKHR to fins su&pported
    swapchain_create_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    
    swapchain_create_info.imageExtent = VkExtent2D{1920, 1080};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // The good one
    // There are many more settings, research
    vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);

    // Check how many images we actually got and retrieve them
    
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr); // Sending no array to fill to get count
    swapchainImageHandles = new VkImage[image_count];
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchainImageHandles);
    return ret;
}
#include "../mfly_window/mfly_window.hpp"
// The swapchain you acquire image to interact with
// App draws over acquired image
// Then we hand the image back to image and immediately ask for another to the swapchain to keep the loop going
// Like OpenGLSwapBuffers... or however it was said -> Swapchain nows presents image on screen (Vertically - vsync)
// After used, the swapchain holds the image in the image bucket for the app to use and does not have anything to present
// Like this we can set a buffer of images for the buffer to present instead of being starved for images

// In the case that the swapchain has no more images to provide while it is presenting one, it will drop image being present to be sent back
// then use the next in the buffer to continue the presentation -> Tearing -> Ideally, don't send if there is one in presentation (vsync - wait for a vertical draw to occurr on screen)

// This is solved with Presentation Modes

uint16_t mfly::gpu::Close()
{
    uint16_t ret = 0;
    delete[] physical_devices; // memleak...
    return ret;
}

//temporary variables
VkSemaphore imageAvailableSemaphore;
VkFence imageAvailableFence;
uint32_t currImageIndex;

uint16_t mfly::gpu::PreUpdate()
{
    uint16_t ret = 0;

    // Acquire image to draw on
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, // Max time to wait for image (usually as much as possible)
                            imageAvailableSemaphore, // Signal when acquired -> Wait on GPU
                            imageAvailableFence, // Signal when acquired -> Wait on CPU
                            &currImageIndex); // Which image to use from the handles of images in the swapchain

    return ret;
}

VkQueue queue;
VkSemaphore renderFinishedSemaphore;
VkFence syncCPUwithGPU_fence;

uint16_t mfly::gpu::DoUpdate()
{
    uint16_t ret = 0;

    VkSubmitInfo submit_info = {}; // Sutrct to give to queue that will send things to GPU
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 0; // As many calls you need to do
    submit_info.pCommandBuffers = nullptr; // Struct that holds the draw calls
    submit_info.waitSemaphoreCount = 1; // Wait for ???
    submit_info.signalSemaphoreCount = 1; // Number of Semaphores to alert the queue is done
    submit_info.pSignalSemaphores = &renderFinishedSemaphore;

    vkQueueSubmit(queue, 1, &submit_info, syncCPUwithGPU_fence); // Check fence for syncing
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