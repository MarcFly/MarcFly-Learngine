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
            VkSwapchainKHR swapchain;
            VkImage* image_handles;
            uint32_t image_count;

            uint32_t curr_image;

            VkSemaphore img_semaphore;
            VkFence img_fence;
        };

        struct VkBufferMemWrap {
            VkBuffer buffer;
            std::vector<VkBufferView> views;
        };

        struct VkImageMemWrap {
            VkImage img;
            std::vector<VkImageView> views;
        };
        
        struct VkAllocMemWrap {
            VkDeviceMemory mem;
            uint64_t next_offset;
            uint64_t size;
        };

        static struct VkApp {
            VkInstance instance;

            VkPhysicalDevice* phys_dvcs = nullptr;
            uint32_t phys_dvc_count;
            std::vector<VkDevice> logical_dvcs;

            VkSurfaceKHR main_surface;
            std::vector<VkSurfaceKHR> surfaces;
            GetSurfaceFun get_surface = nullptr;

            std::vector<VkSwapchainWrap> swapchains;

            std::vector<VkBufferMemWrap> buffers;
            std::vector<VkAllocMemWrap> mem;

            std::vector<VkImageMemWrap> images;

        } vkapp;
        
        static std::vector<VkDeviceQueueCreateInfo> declared_queues;
        static std::vector<std::vector<float>> declared_priorities;

        static struct VkAppInfo {
            VkInstanceInfoWrap info;
            std::vector<VkPhysDeviceInfoWrap> p_dvcs;
            std::vector<VkLDeviceInfoWrap> l_dvcs;
            std::vector<VkSwapchainCreateInfoKHR> swapchains;
            std::vector<VkBufferInfoWrap> buffers;
            std::vector<VkMemInfoWrap> mem_allocs;
            std::vector<VkImageInfoWrap> imgs;

        } MEMOPT(vkapp_info);
        
        
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

uint16_t mfly::gpu::InitVkInstance(VkInstanceInfoWrap info) {
    
    // Declare Vulkan application
    info.app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.app_info.apiVersion = VK_API_VERSION_1_3;

    info.create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // On MAC using MoltenVK or similar porting, flag and extension:
    //info.create_info.flags |=  VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    //info.exts.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // Tidbit -> emplace_back == push_back but better -> No inferred copy on constructors or data not in scope stack!
    // example: string s; vector.emplace_back(s) -> s is copied because is in scope stack
    // example2: vector.emplace_back(std::string("NewString"); -> Moved to vector scope so no copy!
    
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

    for(int i = 0; i < vkapp.phys_dvc_count; ++i) {
        vkapp_info.p_dvcs.push_back(VkPhysDeviceInfoWrap());
        VkPhysDeviceInfoWrap& dvc_info = vkapp_info.p_dvcs.back();
        vkGetPhysicalDeviceProperties(vkapp.phys_dvcs[i], &dvc_info.properties);
        vkGetPhysicalDeviceFeatures(vkapp.phys_dvcs[i], &dvc_info.features);
    }

    // DEBUG INFO
    MEMOPT(
        vkapp_info.info = info;
        uint32_t ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
        vkapp_info.info.available_exts.resize(ext_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, vkapp_info.info.available_exts.data());
    );
    
    return 0;
}

uint16_t mfly::gpu::InitQueues(VkLDeviceInfoWrap& info, uint16_t phys_dvc_handle) {    
    uint32_t size = info.queues_info.size();
    info.queues_info.back().pQueuePriorities = info.prios.back().data();
    for(int i = size - 2; i > 0; --i) {
        info.queues_info[i].pNext = &info.queues_info[i+1];
        info.queues_info[i].pQueuePriorities = info.prios[i].data();
    }
    
    vkapp.logical_dvcs.push_back(VkDevice());
    VkDevice& device = vkapp.logical_dvcs.back();
    
    info.create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.create_info.queueCreateInfoCount = info.queues_info.size();
    info.create_info.pQueueCreateInfos = info.queues_info.data();
    info.create_info.enabledExtensionCount = info.exts.size();
    info.create_info.ppEnabledExtensionNames = info.exts.data();

    VkResult result = vkCreateDevice(vkapp.phys_dvcs[phys_dvc_handle], &info.create_info, nullptr, &device);
    assert(result == 0);
    printf("VkResult is: %d\n", result); 

    // DEBUG INFO
    MEMOPT(
        vkapp_info.l_dvcs.push_back(info);
    )
    
    
    return 0;
}

uint16_t mfly::gpu::CreateSwapchain(VkSwapchainCreateInfoKHR info, uint16_t surface_handle, uint16_t logical_dvc_handle) {

    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = vkapp.surfaces[surface_handle];
    // There are many more settings, research

    vkapp.swapchains.push_back(VkSwapchainWrap());
    uint16_t size = vkapp.swapchains.size();
    VkSwapchainWrap& scw = vkapp.swapchains.back();
    VkDevice& dvc = vkapp.logical_dvcs[logical_dvc_handle];
    vkCreateSwapchainKHR(dvc, &info, nullptr, &scw.swapchain);

    vkGetSwapchainImagesKHR(dvc, scw.swapchain, &scw.image_count, nullptr);
    scw.image_handles = new VkImage[scw.image_count];
    vkGetSwapchainImagesKHR(dvc, scw.swapchain, &scw.image_count, scw.image_handles);
    
    // DEBUG INFO
    MEMOPT(
        vkapp_info.swapchains.push_back(info);
    )

    return size;
}

uint32_t mfly::gpu::CreateBuffer(VkBufferInfoWrap info) {
    info.buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    vkapp.buffers.push_back(VkBufferMemWrap());
    VkBufferMemWrap& buf = vkapp.buffers.back();
    
    VkDevice& dvc = vkapp.logical_dvcs[info.handles.logical_dvc_handle];
    vkCreateBuffer(dvc, &info.buffer, nullptr, &buf.buffer);

    uint64_t use_offset = 0;
    VkMemInfoWrap mem_info = {};
    vkGetBufferMemoryRequirements(dvc, buf.buffer, &mem_info.mem);

    if(info.handles.mem_handle == UINT16_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size()-1;
        VkAllocMemWrap& mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());
        
        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size()-1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?    
        MEMOPT(
            vkapp_info.mem_allocs.push_back(mem_info);
            info.handles.mem_info_handle = vkapp_info.mem_allocs.size()-1;
        )
    }
    else { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
    }
    vkBindBufferMemory(dvc, buf.buffer, vkapp.mem[info.handles.mem_handle].mem, use_offset); // Offset? What?

    // Create View for Texel Buffers
    if(info.buffer.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | info.buffer.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {  
        for(auto view_info : info.views)
        {
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = buf.buffer;

            buf.views.push_back(VkBufferView());
            vkCreateBufferView(dvc, &view_info, nullptr, &buf.views.back());
        }
    }

    // DEBUG INFO
    MEMOPT(
        vkapp_info.buffers.push_back(info);
    )
    return vkapp.buffers.size()-1;
}

uint32_t mfly::gpu::CreateImage(VkImageInfoWrap info) {
    // Example Image
    info.img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    vkapp.images.push_back(VkImageMemWrap());
    VkImageMemWrap& img = vkapp.images.back();
    VkDevice& dvc = vkapp.logical_dvcs[info.handles.logical_dvc_handle];
    vkCreateImage(dvc, &info.img, nullptr, &img.img);

    uint64_t use_offset = 0;
    VkMemInfoWrap mem_info = {};
    vkGetImageMemoryRequirements(dvc, img.img, &mem_info.mem);

    if(info.handles.mem_handle == UINT16_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size()-1;
        VkAllocMemWrap& mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());
        
        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size()-1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?    
        MEMOPT(
            vkapp_info.mem_allocs.push_back(mem_info);
            info.handles.mem_info_handle = vkapp_info.mem_allocs.size()-1;
        )
    }
    else { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
    }

    vkBindImageMemory(dvc, img.img, vkapp.mem[info.handles.mem_handle].mem, use_offset);

    for(auto view_info : info.views) {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = img.img;

        img.views.push_back(VkImageView());
        vkCreateImageView(dvc, &view_info, nullptr, &img.views.back());
    }
    
    // DEBUG INFO
    MEMOPT(
        vkapp_info.imgs.push_back(info);
    )


    return vkapp.images.size()-1;
}

// Module Basics
uint16_t mfly::gpu::DefaultInit()
{   
    uint16_t ret = 0;

    VkInstanceInfoWrap app_info;
    app_info.exts.push_back("VK_KHR_surface"); 
    app_info.exts.push_back("VK_KHR_win32_surface");
    
    app_info.layers.push_back("VK_LAYER_KHRONOS_validation");
    InitVkInstance(app_info);

    
    //assert(vkPDeviceCount > 0);
    VkPhysicalDevice dev  = vkapp.phys_dvcs[0]; // Grab Pointer to device in use

    // vkEnumerateDeviceExtensionProperties(dev, ...)
    // vkGetPhysicalDeviceProperties(dev, ...)
    // They are a good example on how the library mostly returns info
    // YOu give iethe r set value in __count and retrive info a struct set to that size
    // or you pass a null in all poitners and then then pointer will be used to tell how many there are
    // then do a 2nd call and retrive the intended amount of info
    // https://stackoverflow.com/questions/37662614/calling-vkenumeratedeviceextensionproperties-twice-is-it-required

    // Example default queue
    VkLDeviceInfoWrap l_dvc_info;

    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = 0;
    qci.queueCount = 1;

    l_dvc_info.queues_info.push_back(qci);
    l_dvc_info.prios.push_back(std::vector<float>());
    l_dvc_info.prios.back().push_back(1);
    l_dvc_info.queues_info.back().pQueuePriorities = l_dvc_info.prios.back().data();

    l_dvc_info.exts.push_back("VK_KHR_swapchain");
    InitQueues(l_dvc_info, 0);

    // Ask window manager to get a surface
    vkapp.main_surface = (VkSurfaceKHR)vkapp.get_surface((void*)vkapp.instance, 0); // use main window for main surface
    vkapp.surfaces.push_back(vkapp.main_surface);

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.minImageCount = 4;
    
    // Should use vkGetPhysicalDeviceSurfaceFormatsKHR to find supported
    swapchain_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    
    swapchain_info.imageExtent = VkExtent2D{1920, 1080};
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // The good one
    
    CreateSwapchain(swapchain_info, 0, 0);


    // Example Buffers
    VkBufferInfoWrap buf_info;
    buf_info.buffer.size = 1024;
    buf_info.buffer.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    CreateBuffer(buf_info);

    VkBufferInfoWrap buf_info_txl;
    buf_info_txl.buffer.size = 1024;
    buf_info_txl.buffer.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buf_info_txl.buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    buf_info_txl.views.push_back(VkBufferViewCreateInfo());
    VkBufferViewCreateInfo& view_info = buf_info_txl.views.back();
    view_info = {};
    view_info.offset = 0;
    view_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    view_info.range = VK_WHOLE_SIZE;
    
    CreateBuffer(buf_info_txl);
    
    // Example Image
    VkImageInfoWrap img_info;
    img_info.img = {};
    img_info.img.imageType = VK_IMAGE_TYPE_2D;
    img_info.img.format = VK_FORMAT_R8G8B8A8_SNORM;
    img_info.img.samples = VK_SAMPLE_COUNT_1_BIT; // For this case if it is not 1 it does not work, why?
    img_info.img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Type of image
    img_info.img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    img_info.img.extent = VkExtent3D{1920, 1080, 1};
    img_info.img.arrayLayers = 1;
    img_info.img.mipLevels = 1;
    // Image info data...
    
    // View
    img_info.views.push_back(VkImageViewCreateInfo());
    VkImageViewCreateInfo& img_view_info = img_info.views.back();
    img_view_info = {};
    img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    img_view_info.format = VK_FORMAT_R8G8B8A8_SNORM;
    // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT if it is different format than image is specified
    img_view_info.components.r = VK_COMPONENT_SWIZZLE_B;
    img_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    img_view_info.components.b = VK_COMPONENT_SWIZZLE_R;
    img_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    // This is when knowing if a texture is not in RGBA order... old textures or other formats
    // Most of the cases this is still irrelevant as for internal formats you will most probably implement your own buffer...

    img_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_view_info.subresourceRange.baseMipLevel = 0;
    img_view_info.subresourceRange.levelCount = 1;
    img_view_info.subresourceRange.baseArrayLayer = 0;
    img_view_info.subresourceRange.layerCount = 1;
    
    img_info.handles.logical_dvc_handle = 0;
    CreateImage(img_info);

    // DEBUG INFO
    MEMOPT(
        VkAppInfo& t_info = vkapp_info;
    )

    return ret;
}

uint16_t mfly::gpu::Close()
{
    uint16_t ret = 0;
    delete[] vkapp.phys_dvcs; // memleak...

    vkDestroyInstance(vkapp.instance, nullptr);
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