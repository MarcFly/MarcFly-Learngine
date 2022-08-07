#include "mfly_gpu.hpp"

#include <vulkan/vulkan_funcs.hpp>



namespace mfly {
    namespace gpu {
        // Understand if the CreateInfo structures will be useful at runtime or only on creation
        // If not -> worth it to keep the memory for debuggin on the wrapper?

        static const float queue_prio = 1.;
        struct VkQueueWrap {
            float queue_prio = 1.f;
            VkDeviceQueueCreateInfo queue_create_info = {};
        };

        

        struct VkSwapchainWrap {
            VkSwapchainCreateInfoKHR create_info = {};
            VkSwapchainKHR swapchain;
            VkImage* image_handles;
            uint32_t image_count;

            uint32_t curr_image;

            VkSemaphore img_semaphore;
            VkFence img_fence;
        };

        static struct VkApp {
            VkInstance instance;

            VkPhysicalDevice* phys_dvcs = nullptr;
            uint32_t phys_dvc_count;
            std::vector<VkDevice> logical_dvcs;

            VkSurfaceKHR main_surface;
            GetSurfaceFun get_surface = nullptr;

            std::vector<VkSwapchainWrap> swapchains;

        } vkapp;
        
        static std::vector<VkDeviceQueueCreateInfo> declared_queues;
        static std::vector<std::vector<float>> declared_priorities;

        #ifdef DEBUG_VK
        static struct VkAppInfo {
            VkInstanceWrap info;
            std::vector<VkLDeviceWrap> devices;
            
        } app_info;
        #endif
        
        
    };
};

// Extensions are super important
// Such as vkCreateDebugUtilsMessesngerEXT with extension name VK_EXT_debug_utils
// Extensions usually use VK_EXT while builtin use VK_KHR
// Some extensions have dependencies!
// They might also need configuration structs!
// Have to add create_info.pNext = extension
// Then chain them from feature create_info to create info
// They also have to be in order of description in the enabled_extensions[] array

// Queues are bound to a family perpetually, with specific amount
// Receive and describe commands to be processed to the device
// Commands in a single queue can be processed parallelly
// Multiples Queues help parallellism -> good when they are not dependant
// You can create multiples queues when creating a single logical device
// Good to specialize the queues -> Transfer, async,...
// GPUInfo and tools by Sascha Willems https://github.com/SaschaWillems/Vulkan

const char* device_extensions[] = {"VK_KHR_swapchain"};
// GPUs have driver and isntalled extensions -> Varies from Device to instance
// Much more per GPU than per instance *Ray Tracing for example)
// 

uint16_t mfly::gpu::InitVkInstance(VkInstanceWrap info) {
    
    // Declare Vulkan application
    info.app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.app_info.apiVersion = VK_API_VERSION_1_3;

    info.create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    info.create_info.enabledExtensionCount = info.exts.size();
    info.create_info.ppEnabledExtensionNames = info.exts.data();

    info.create_info.enabledLayerCount = info.layers.size();
    info.create_info.ppEnabledLayerNames = info.layers.data(); //enabled_layers;

    // Missing Extension create info linking
    // Learn about Allocation Callbacks
    VkResult result = vkCreateInstance(&info.create_info, nullptr, &vkapp.instance);
    assert(result == 0);
    printf("VkResult is: %d\n", result);

    vkEnumeratePhysicalDevices(vkapp.instance, &vkapp.phys_dvc_count, nullptr);
    vkapp.phys_dvcs = new VkPhysicalDevice[vkapp.phys_dvc_count];
    vkEnumeratePhysicalDevices(vkapp.instance, &vkapp.phys_dvc_count, vkapp.phys_dvcs);

    // Add Instance info to Debug info

    return 0;
}

// Maybe decalre should pass pointer that we invalidate instead of data duplication
// Still, not memory critical theoretically
uint16_t mfly::gpu::DeclareQueue(VkDeviceQueueCreateInfo info, float* prios) {
    
    
    declared_priorities.push_back(std::vector<float>());
    uint32_t size = declared_priorities.size();
    std::vector<float>& prios_vec = declared_priorities[size-1];
    prios_vec.reserve(info.queueCount);
    for(int i = 0; i < info.queueCount; ++i)
        prios_vec.push_back(prios[i]);

    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    //info.pQueuePriorities = prios_vec.data();
    declared_queues.push_back(info);

    return declared_queues.size()-1;
}

uint16_t mfly::gpu::InitQueues(VkLDeviceWrap info, uint32_t phys_dvc_handle) {

    // Chain the queues now to avoid the vector resizing and moving pointer while adding queues
    uint32_t size = declared_priorities.size();
    declared_queues[size-1].pQueuePriorities = declared_priorities[size-1].data();
    for(int i = declared_queues.size() - 2; i > 0; --i) {
        declared_queues[i].pNext = &declared_queues[i+1];
        declared_queues[i].pQueuePriorities = declared_priorities[i].data();
    }

    info.queues_info.swap(declared_queues);
    info.prios.swap(declared_priorities);
    
    vkapp.logical_dvcs.push_back(VkDevice());
    VkDevice& device = vkapp.logical_dvcs[vkapp.logical_dvcs.size()-1];
    
    info.create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.create_info.queueCreateInfoCount = info.queues_info.size();
    info.create_info.pQueueCreateInfos = info.queues_info.data();
    info.create_info.enabledExtensionCount = info.exts.size();
    info.create_info.ppEnabledExtensionNames = info.exts.data();

    VkResult result = vkCreateDevice(vkapp.phys_dvcs[phys_dvc_handle], &info.create_info, nullptr, &device);
    assert(result == 0);
    printf("VkResult is: %d\n", result); 

    //Add Device info to debug info

    return 0;
}

// Module Basics
uint16_t mfly::gpu::DefaultInit()
{   
    uint16_t ret = 0;

    VkInstanceWrap app_info;
    app_info.exts.push_back("VK_KHR_surface"); 
    app_info.exts.push_back("VK_KHR_win32_surface");
    
    app_info.layers.push_back("VK_LAYER_KHRONOS_validation");
    InitVkInstance(app_info);

    
    //assert(vkPDeviceCount > 0);
    VkPhysicalDevice dev  = vkapp.phys_dvcs[0]; // Grab Pointer to device in use

    // vkEnumerateDeviceExtensionProperties(dev, ...)
    // vlGetPhysicalDeviceProperties(dev, ...)
    // They are a good example on how the library mostly returns info
    // YOu give iethe r set value in __count and retrive info a struct set to that size
    // or you pass a null in all poitners and then then pointer will be used to tell how many there are
    // then do a 2nd call and retrive the intended amount of info
    // https://stackoverflow.com/questions/37662614/calling-vkenumeratedeviceextensionproperties-twice-is-it-required

    // Example default queue
    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = 0;
    qci.queueCount = 1;
    qci.pQueuePriorities = &queue_prio;
    float prio = 1;
    DeclareQueue(qci, &prio);

    VkLDeviceWrap logic_dvc_info;
    logic_dvc_info.exts.push_back("VK_KHR_swapchain");
    InitQueues(logic_dvc_info);

    // Ask window manager to get a surface
    vkapp.main_surface = (VkSurfaceKHR)vkapp.get_surface((void*)vkapp.instance, 0); // use main window for main surface
    //surface = (VkSurfaceKHR)mfly::win::getGAPISurface(0, (vk::Instance)vkapp.instance);

    vkapp.swapchains.push_back(VkSwapchainWrap());
    VkSwapchainWrap& curr_sc = vkapp.swapchains[vkapp.swapchains.size()-1];
    
    curr_sc.create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    curr_sc.create_info.surface = vkapp.main_surface;
    curr_sc.create_info.minImageCount = 4;
    
    // Should use vkGetPhysicalDeviceSurfaceFormatsKHR to fins su&pported
    curr_sc.create_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    curr_sc.create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    
    curr_sc.create_info.imageExtent = VkExtent2D{1920, 1080};
    curr_sc.create_info.imageArrayLayers = 1;
    curr_sc.create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    curr_sc.create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // The good one
    // There are many more settings, research
    vkCreateSwapchainKHR(vkapp.logical_dvcs[0], &curr_sc.create_info, nullptr, &curr_sc.swapchain);

    // Check how many images we actually got and retrieve them
    
    vkGetSwapchainImagesKHR(vkapp.logical_dvcs[0], curr_sc.swapchain, &curr_sc.image_count, nullptr); // Sending no array to fill to get count
    curr_sc.image_handles = new VkImage[curr_sc.image_count];
    vkGetSwapchainImagesKHR(vkapp.logical_dvcs[0], curr_sc.swapchain, &curr_sc.image_count, curr_sc.image_handles);
    return ret;
}

uint16_t mfly::gpu::Close()
{
    uint16_t ret = 0;
    delete[] vkapp.phys_dvcs; // memleak...
    return ret;
}

//temporary variables


uint16_t mfly::gpu::PreUpdate()
{
    uint16_t ret = 0;

    // Acquire image to draw on
    for(auto sc_wrap : vkapp.swapchains) {
        vkAcquireNextImageKHR(vkapp.logical_dvcs[0], sc_wrap.swapchain, 
                            UINT64_MAX,                 // Max time to wait for image (usually as much as possible)
                            sc_wrap.img_semaphore,      // Signal when acquired -> Wait on GPU
                            sc_wrap.img_fence,          // Signal when acquired -> Wait on CPU
                            &sc_wrap.curr_image);       // Which image to use from the handles of images in the swapchain
    }

    return ret;
}

VkQueue queue;
VkSemaphore renderFinishedSemaphore;
VkFence syncCPUwithGPU_fence;

uint16_t mfly::gpu::DoUpdate()
{
    uint16_t ret = 0;

    VkSubmitInfo submit_info = {}; // Struct to give to queue that will send things to GPU
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
void mfly::gpu::ProvideSurfaceFun(GetSurfaceFun fun) {
    vkapp.get_surface = fun;
}