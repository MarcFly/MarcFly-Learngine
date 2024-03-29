#include "mfly_vk.hpp"

#include <vulkan/vulkan.hpp>
#include <assert.h>

struct mfly::vk::VkApp vkapp;
struct mfly::vk::VkAppInfo vkapp_info;
namespace mfly
{
    namespace vk
    {
        // Understand if the CreateInfo structures will be useful at runtime or only on creation
        // If not -> worth it to keep the memory for debuggin on the wrapper?

        static const float queue_prio = 1.;
        

        

        

        static std::vector<VkDeviceQueueCreateInfo> declared_queues;
        static std::vector<std::vector<float>> declared_priorities;

    };
};

using namespace mfly;

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

// GPUs have driver and isntalled extensions -> Varies from Device to instance
// Much more per GPU than per instance *Ray Tracing for example)


VkSemaphore& mfly::vk::AddSemaphore(sm_key& dvc_handle, sm_key& semaphore_handle) {
    VkDevice& dvc = vkapp.logical_dvcs[dvc_handle];
    if(vkapp.semaphores.has_key(semaphore_handle))
        vkDestroySemaphore(dvc, vkapp.semaphores[semaphore_handle], nullptr);

    VkSemaphore& semaphore = vkapp.semaphores.insert(semaphore_handle, VkSemaphore());

    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // TODO: Properly select logical device
    VkResult res = vkCreateSemaphore(dvc, &info, nullptr, &semaphore);

    return semaphore;
}

VkFence& mfly::vk::AddFence(sm_key& dvc_handle, sm_key& fence_handle) {
    VkDevice& dvc = vkapp.logical_dvcs[dvc_handle];
    if(vkapp.fences.has_key(fence_handle))
        vkDestroyFence(dvc, vkapp.fences[fence_handle], nullptr);
    
    VkFence& fence = vkapp.fences.insert(fence_handle, VkFence());

    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    // TODO: Properly select logical device
    VkResult res = vkCreateFence(dvc, &info, nullptr, &fence);

    return fence;
}

// Module Basics
uint32_t mfly::vk::DefaultInit()
{
    uint32_t ret = 0;

    VkInstance_InitInfo app_info;
    app_info.exts.push_back("VK_KHR_surface");
    app_info.exts.push_back("VK_KHR_win32_surface");

    app_info.layers.push_back("VK_LAYER_KHRONOS_validation");
    InitVkInstance(app_info);

    // assert(vkPDeviceCount > 0);
    sm_key phys_dvc_key = vkapp.phys_dvcs.GetKeyAtIDX(0);
    VkPhysicalDevice dev = vkapp.phys_dvcs[0]; // Grab Pointer to device in use

    // vkEnumerateDeviceExtensionProperties(dev, ...)
    // vkGetPhysicalDeviceProperties(dev, ...)
    // They are a good example on how the library mostly returns info
    // YOu give iethe r set value in __count and retrive info a struct set to that size
    // or you pass a null in all poitners and then then pointer will be used to tell how many there are
    // then do a 2nd call and retrive the intended amount of info
    // https://stackoverflow.com/questions/37662614/calling-vkenumeratedeviceextensionproperties-twice-is-it-required

    // Example default queue
    VkLDVC_InitInfo l_dvc_info;

    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = 0;
    qci.queueCount = 1;

    l_dvc_info.queues_info.push_back(qci);
    l_dvc_info.prios.push_back(std::vector<float>());
    l_dvc_info.prios.back().push_back(1);
    l_dvc_info.queues_info.back().pQueuePriorities = l_dvc_info.prios.back().data();
    // l_dvc_info.exts.push_back("VK_KHR_acceleration_structure");
    l_dvc_info.exts.push_back("VK_KHR_swapchain");

    // Example: Device Extension Features/Create Info
    // Define it as usual or create memory in the `exts_info` void* vector
    // Then modify it with the data required and you are set
    // It is not pretty but works
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR rt_struct_info = {};
    // rt_struct_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    // rt_struct_info.accelerationStructure = VK_TRUE;
    // l_dvc_info.exts_info.push_back(new VkPhysicalDeviceAccelerationStructureFeaturesKHR(rt_struct_info));
    // auto check_f = (VkPhysicalDeviceAccelerationStructureFeaturesKHR*)l_dvc_info.exts_info.back();
    // Not enabled because it is not supported on 5700G

    sm_key logical_dvc_handle = InitQueues(l_dvc_info, phys_dvc_key);

    // Ask window manager to get a surface
    vkapp.main_surface = (VkSurfaceKHR)vkapp.get_surface((void *)vkapp.instance, 0); // use main window for main surface
    sm_key surface_handle = vkapp.surfaces.push_back(vkapp.main_surface);

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.minImageCount = 4;

    // Should use vkGetPhysicalDeviceSurfaceFormatsKHR to find supported
    swapchain_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_info.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.imageExtent = VkExtent2D{2560, 1440};
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // VK_PRESENT_MODE_MAILBOX_KHR; // The good one

    sm_key sch_handle;
    CreateSwapchain(sch_handle, swapchain_info, surface_handle, logical_dvc_handle);

    // Swapchain requires a set of framebuffers and attachments to draw too
    // Requires a vec of Framebuffers and their respecitve imageviews

    // DEBUG INFO
    VkAppInfo &t_info = vkapp_info;

    return ret;
}

uint32_t mfly::vk::Close()
{
    uint32_t ret = 0;

    for(int i = 0; i < vkapp.logical_dvcs.size(); ++i) {
        vkDeviceWaitIdle(vkapp.logical_dvcs[i]);
    }

    // Most nullptr here are baseon pAllocatorCallback which I do not know what is it
    // Something for custom allocators like AMD vulkan mem allocation?
    vkDestroyDescriptorPool;
    vkDestroyDescriptorSetLayout;
    vkDestroySampler;
    
    for(int i = 0; i < vkapp.cmd_pools.size(); ++i) {
        vkDestroyCommandPool(vkapp.logical_dvcs[0], vkapp.cmd_pools[i], nullptr);
    }

    for(int i = 0; i < vkapp.framebuffers.size(); ++i) {
        vkDestroyFramebuffer(vkapp.logical_dvcs[0], vkapp.framebuffers[i].framebuffer, nullptr);
    }

    for(int i = 0; i < vkapp.semaphores.size(); ++i) {
        vkDestroySemaphore(vkapp.logical_dvcs[0], vkapp.semaphores[i], nullptr);
    }
    for(int i = 0; i < vkapp.fences.size(); ++i) {
        vkDestroyFence(vkapp.logical_dvcs[0], vkapp.fences[i], nullptr);
    }

    for(int i = 0; i < vkapp.graphic_pipes.size(); ++i) {
        vkDestroyPipeline(vkapp.logical_dvcs[0], vkapp.graphic_pipes[i].pipe, nullptr);
        vkDestroyPipelineLayout(vkapp.logical_dvcs[0], vkapp.graphic_pipes[i].layout, nullptr);
    }
    for(int i = 0; i < vkapp.render_passes.size(); ++i) {
        vkDestroyRenderPass(vkapp.logical_dvcs[0], vkapp.render_passes[i], nullptr);
    }

    for (int i = 0; i < vkapp.swapchains.size(); ++i) {
        DestroySwapchain(vkapp.swapchains[i]);
    }
    
    {
        VkDevice dvc = vkapp.logical_dvcs[0];
        for (int i = 0; i < vkapp.buffers.size(); ++i)
        {
            for(int i = 0; i < vkapp.buffers[i].views.size(); ++i){
                vkDestroyBufferView(dvc, vkapp.buffers[i].views[i], nullptr);
            }
            vkDestroyBuffer(dvc, vkapp.buffers[i].buffer, nullptr);
        }

        for (int i = 0; i < vkapp.img_views.size(); ++i) {
            vkDestroyImageView(dvc, vkapp.img_views[i], nullptr);
        }

        for (int i = 0; i < vkapp.images.size(); ++i) {
            vkDestroyImage(dvc, vkapp.images[i].img, nullptr);
        }
    }

    for (int i = 0; i < vkapp.logical_dvcs.size(); ++i) {
        vkDestroyDevice(vkapp.logical_dvcs[i], nullptr);
    }

    vkapp.phys_dvcs.clear(); // memleak...
    
    vkDestroyInstance(vkapp.instance, nullptr);
    return ret;
}

// temporary variables

// TODO: Redo for each swapchain as in multithreading with multiple swapchains, the app decides to call them when needed

uint32_t mfly::vk::SwapchainNextImage(sm_key& swapchain_handle) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    VkDevice& dvc = vkapp.logical_dvcs[swc_wrap.logical_dvc_handle];
    vkWaitForFences(dvc, 1, &swc_wrap.img_fence, true, UINT64_MAX);
    if(swc_wrap.need_resize) 
        RecreateSwapchain(swapchain_handle);
    VkSwapchainWrap& swc_wrap_r1 = vkapp.swapchains[swapchain_handle];
    vkWaitForFences(dvc, 1, &swc_wrap_r1.img_fence, true, UINT64_MAX);
    VkResult res = vkAcquireNextImageKHR(dvc, swc_wrap_r1.swapchain,
                            UINT64_MAX,            // Max time to wait for image (usually as much as possible)
                            swc_wrap_r1.img_semaphore, // Signal when acquired -> Wait on GPU
                            VK_NULL_HANDLE,     // Signal when acquired -> Wait on CPU // Not needed for now
                            &swc_wrap_r1.curr_image);  // Which image to use from the handles of images in the swapchain
                            
    if(res == VK_ERROR_OUT_OF_DATE_KHR /*|| res == VK_SUBOPTIMAL_KHR*/)
        RecreateSwapchain(swapchain_handle);
    else 
        vkResetFences(dvc, 1, &swc_wrap_r1.img_fence);

    // Further Explanation on Semaphores and Fences
    // BINARY Semaphores are GPU Signals, it is sent to Vulkan for a task to signal when finish
    // Another task uses that same semaphore to wait until it is signaled to now start working
    // In that way, a semaphore can be passed to be signaled by only 1 task, but multiple can wait on it
    // However, a task can only wait on a single semaphore, we can branch out but not back in

    // For that we need Timeline Semaphores (TODO)

    // Fences are for CPU to wait on GPU signals

    return 0;
}

uint32_t mfly::vk::PreUpdate()
{
    uint32_t ret = 0;
    
    return ret;
}

uint32_t mfly::vk::DoUpdate()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::vk::PostUpdate()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::vk::AsyncDispatch()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::vk::AsyncGather()
{
    uint32_t ret = 0;

    return ret;
}

// Module Specificss
void mfly::vk::ProvideSurfaceFun(GetSurfaceFun fun)
{
    vkapp.get_surface = fun;
}

//=============================================

void mfly::vk::ExampleBuffersImages() {
    // Example Buffers
    VkBuffer_InitInfo buf_info;
    buf_info.buffer.size = 1024;
    buf_info.buffer.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CreateBuffer(buf_info);

    VkBuffer_InitInfo buf_info_txl;
    buf_info_txl.buffer.size = 1024;
    buf_info_txl.buffer.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buf_info_txl.buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    buf_info_txl.views.push_back(VkBufferViewCreateInfo());
    uint32_t buf_info_1_h = buf_info_txl.views.size()-1;
    VkBufferViewCreateInfo &view_info = buf_info_txl.views[buf_info_1_h];
    view_info = {};
    view_info.offset = 0;
    view_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    view_info.range = VK_WHOLE_SIZE;

    CreateBuffer(buf_info_txl);

    // Example Image
    VkImage_InitInfo img_info;
    img_info.img = {};
    img_info.img.imageType = VK_IMAGE_TYPE_2D;
    img_info.img.format = VK_FORMAT_R8G8B8A8_SRGB;
    img_info.img.samples = VK_SAMPLE_COUNT_1_BIT;             // For this case if it is not 1 it does not work, why?
    img_info.img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Type of image
    img_info.img.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    img_info.img.extent = VkExtent3D{1540, 845, 1};
    img_info.img.arrayLayers = 1;
    img_info.img.mipLevels = 1;
    // Image info data...

    // View
    img_info.views.push_back(VkImageViewCreateInfo());
    VkImageViewCreateInfo &img_view_info = img_info.views.back();
    img_view_info = {};
    img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    img_view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
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

    img_info.handles.logical_dvc_handle = vkapp.logical_dvcs.GetKeyAtIDX(0);
    CreateImage(img_info);
}