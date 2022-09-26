#include "mfly_gpu.hpp"

#include <vulkan/vulkan_funcs.hpp>

template<class T>
inline uint32_t PushNonInvalid(std::vector<T>& vec, int64_t check_val) {
    int64_t v = vec.size()-1;
    if(check_val > v){
        vec.push_back(T());
        check_val = vec.size()-1;
    }

    return check_val;
}

template<class T>
inline void VecFromHandles(const std::vector<uint32_t> handles, const std::vector<T>& data, std::vector<T>& out) {
    out.clear();
    for(auto h : handles) {
        out.push_back(data[h]);
    }
}

namespace mfly
{
    namespace gpu
    {
        // Understand if the CreateInfo structures will be useful at runtime or only on creation
        // If not -> worth it to keep the memory for debuggin on the wrapper?

        static const float queue_prio = 1.;
        struct VkQueueWrap
        {
            float queue_prio = 1.f;
            VkDeviceQueueCreateInfo queue_create_info = {};
        };

        struct VkBufferMemWrap
        {
            VkBuffer buffer;
            std::vector<VkBufferView> views;
            uint32_t mem_handle = -1;
        };

        struct VkImageMemWrap
        {
            VkImage img;
            std::vector<uint32_t> view_handles;
            uint32_t mem_handle = -1;
        };

        struct VkAllocMemWrap
        {
            VkDeviceMemory mem;
            uint64_t next_offset;
            uint64_t size;
        };

        struct VkShaderModuleWrap {
            VkShaderModule shader;
            VkPipelineShaderStageCreateInfo stage_info = {};
        };

        struct VkGraphicsPipelineWrap {
            VkPipelineLayout layout;
            VkPipeline pipe;
            // All the other things
        };
        
        struct VkSubPassWrap {
            VkSubpassDescription subpass;
            VkSubPassInfoWrap info;
        };

        struct VkRenderPassWrap {
            VkRenderPass pass;
            VkRenderPassInfoWrap info; // For regen
        };

        struct VkRenderPassBeginInfoWrap {
            std::vector<VkClearValue> colors;
            VkRenderPassBeginInfo create_info;
        };

        struct VkSubpassDescWrap {
            VkSubpassDescription desc;
            VkSubPassInfoWrap info;
        };

        // None of this is thread safe right now
        static struct VkApp
        {
            VkInstance instance;

            std::vector<VkPhysicalDevice> phys_dvcs;
            std::vector<VkDevice> logical_dvcs;

            std::vector<VkBufferMemWrap> buffers;
            std::vector<VkAllocMemWrap> mem;
            std::vector<VkImageMemWrap> images;
            std::vector<VkImageView> img_views; // Separate as used for Images, Framebuffers, Attachment, Swapchain Images,...

            VkSurfaceKHR main_surface;
            std::vector<VkSurfaceKHR> surfaces;
            GetSurfaceFun get_surface = nullptr;

            std::vector<VkSwapchainWrap> swapchains;

            std::vector<VkShaderModuleWrap> shaders;

            std::vector<VkFramebufferWrap> framebuffers;
            std::vector<VkAttachmentDescription> attachment_descs;

            std::vector<VkGraphicsPipelineWrap> graphic_pipes;

            std::vector<VkSubpassDescWrap> subpasses;

            std::vector<VkRenderPass> render_passes;
            std::vector<VkRenderPassInfoWrap> render_pass_infos;

            std::vector<VkCommandPool> cmd_pools;
            std::vector<VkCommandBuffer> cmd_buffers;
            VkCommandBuffer* curr_cmd_buffer = nullptr;
            std::vector<VkCommandBufferBeginInfo> begin_cmd_infos;
            std::vector<VkRenderPassBeginInfoWrap> begin_renderpass_infos;

            std::vector<VkSemaphore> semaphores;
            std::vector<VkFence> fences;

        } vkapp;

        static std::vector<VkDeviceQueueCreateInfo> declared_queues;
        static std::vector<std::vector<float>> declared_priorities;

        static struct VkAppInfo
        {
            VkInstanceInfoWrap info;
            std::vector<VkPhysDeviceInfoWrap> p_dvcs;
            std::vector<VkLDeviceInfoWrap> l_dvcs;
            std::vector<VkSwapchainCreateInfoKHR> swapchains;
            std::vector<VkBufferInfoWrap> buffers;
            std::vector<VkMemInfoWrap> mem_allocs;
            std::vector<VkImageInfoWrap> imgs;
            std::vector<VkFramebufferInfoWrap> framebuffers;

        } vkapp_info;

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

// GPUs have driver and isntalled extensions -> Varies from Device to instance
// Much more per GPU than per instance *Ray Tracing for example)
//

uint32_t mfly::gpu::InitVkInstance(VkInstanceInfoWrap& info)
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
    vkEnumeratePhysicalDevices(vkapp.instance, &phys_dvc_count, nullptr);
    vkapp.phys_dvcs.resize(phys_dvc_count);
    vkEnumeratePhysicalDevices(vkapp.instance, &phys_dvc_count, vkapp.phys_dvcs.data());

    // DEBUG INFO
    vkapp_info.info = info;

    for (int i = 0; i < vkapp.phys_dvcs.size(); ++i) {
        vkapp_info.p_dvcs.push_back(VkPhysDeviceInfoWrap());
        VkPhysDeviceInfoWrap &dvc_info = vkapp_info.p_dvcs.back();
        vkGetPhysicalDeviceProperties(vkapp.phys_dvcs[i], &dvc_info.properties);
        vkGetPhysicalDeviceFeatures(vkapp.phys_dvcs[i], &dvc_info.features);
    }

    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
    vkapp_info.info.available_exts.resize(ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, vkapp_info.info.available_exts.data());

    return 0;
}

uint32_t mfly::gpu::InitQueues(VkLDeviceInfoWrap &info, uint32_t phys_dvc_handle)
{
    uint32_t size = info.queues_info.size();
    info.queues_info.back().pQueuePriorities = info.prios.back().data();
    for (int i = size - 2; i > 0; --i)
    {
        info.queues_info[i].pNext = &info.queues_info[i + 1];
        info.queues_info[i].pQueuePriorities = info.prios[i].data();
    }

    vkapp.logical_dvcs.push_back(VkDevice());
    VkDevice* device = &vkapp.logical_dvcs.back();

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

    VkResult result = vkCreateDevice(vkapp.phys_dvcs[phys_dvc_handle], &info.create_info, nullptr, device);
    assert(result == 0);
    printf("VkResult is: %d\n", result);

    // DEBUG INFO
    vkapp_info.l_dvcs.push_back(info);

    return 0;
}

VkQueue mfly::gpu::RetrieveQueue(uint32_t device_handle, uint32_t family, uint32_t index) {
    VkQueue ret;
    vkGetDeviceQueue(vkapp.logical_dvcs[device_handle], family, index, &ret);
    return ret;
}

uint32_t mfly::gpu::CreateSwapchain(VkSwapchainCreateInfoKHR info, uint32_t surface_handle, uint32_t logical_dvc_handle, uint32_t existing)
{
    uint32_t prev_val = existing;
    existing = PushNonInvalid(vkapp.swapchains, existing);
    bool recreate = existing != prev_val;

    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = vkapp.surfaces[surface_handle];
    // There are many more settings, research

    info.minImageCount = 1;
    
    VkSwapchainWrap &scw = vkapp.swapchains[existing];
    scw.logical_dvc_handle = logical_dvc_handle;
    VkDevice &dvc = vkapp.logical_dvcs[logical_dvc_handle];
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkResult res = vkCreateSwapchainKHR(dvc, &info, nullptr, &scw.swapchain);

    scw.area = info.imageExtent;
    scw.surface_handle = surface_handle;

    uint32_t img_count;
    //vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, nullptr);
    img_count = 1;
    scw.images.resize(img_count);
    vkGetSwapchainImagesKHR(dvc, scw.swapchain, &img_count, scw.images.data());

    // TODO: Proper info for the image views
    VkImageViewCreateInfo view_info = {};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT if it is different format than image is specified
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    // This is when knowing if a texture is not in RGBA order... old textures or other formats
    // Most of the cases this is still irrelevant as for internal formats you will most probably implement your own buffer...

    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    // Image info data...

    for(int i = 0; i < scw.images.size(); ++i) {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = scw.images[i];

        scw.img_views.push_back(VkImageView());
        
        vkCreateImageView(dvc, &view_info, nullptr, &scw.img_views.back());
    }

    std::pair<uint32_t, VkSemaphore> sem = AddSemaphore(scw.img_fence_h);
    std::pair<uint32_t, VkFence>  fence = AddFence(scw.img_semaphore_h);
    scw.img_semaphore_h = sem.first;
    scw.img_semaphore = sem.second;
    scw.img_fence_h = fence.first;
    scw.img_fence = fence.second;

    // DEBUG INFO
    uint32_t info_id = PushNonInvalid(vkapp_info.swapchains, existing);
    vkapp_info.swapchains[info_id] = info;

    return existing;
}

void mfly::gpu::TriggerResizeSwapchain(uint32_t swapchain_handle, VkExtent2D area) {
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    swc_wrap.need_resize = true;
    swc_wrap.area = area;
    vkapp_info.swapchains[swapchain_handle].imageExtent = area;
    for(int i = 0; i < swc_wrap.fb_infos.size(); ++i) {
        vkapp_info.framebuffers[i].extent = area;
    }
}

void mfly::gpu::RecreateSwapchain(uint32_t swapchain_handle) {
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkDeviceWaitIdle(dvc);

    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    swc_wrap.need_resize = false;

    vkDestroySwapchainKHR(dvc, swc_wrap.swapchain, nullptr);

    for (auto imgview : swc_wrap.img_views) {
        vkDestroyImageView(dvc, imgview, nullptr);
    }
    swc_wrap.img_views.clear();

    for (auto img : swc_wrap.images) {
        vkDestroyImage(dvc, img, nullptr);
    }
    swc_wrap.images.clear();
    swc_wrap.framebuffers.clear();
    
    CreateSwapchain(vkapp_info.swapchains[swapchain_handle], swc_wrap.surface_handle, swc_wrap.logical_dvc_handle, swapchain_handle);
    
    // Now destroy and recreate framebuffers

    for(int i = 0; i < swc_wrap.fb_infos.size(); ++i) {
        AddSWCFramebuffer(vkapp_info.framebuffers[swc_wrap.fb_infos[i]], swapchain_handle, i);
    }
    // TODO: Recreate Renderpasses -> Image Change

}

void mfly::gpu::DestroySwapchain(VkSwapchainWrap& swc_wrap) {
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkDestroySwapchainKHR(dvc, swc_wrap.swapchain, nullptr);

    for (auto imgview : swc_wrap.img_views) {
        vkDestroyImageView(dvc, imgview, nullptr);
    }
    swc_wrap.img_views.clear();

    for (auto img : swc_wrap.images) {
        vkDestroyImage(dvc, img, nullptr);
    }
    swc_wrap.images.clear();
}

const mfly::gpu::VkSwapchainWrap mfly::gpu::RetrieveSwapchain(uint32_t swapchain_handle) { return vkapp.swapchains[swapchain_handle]; }

uint32_t mfly::gpu::CreateBuffer(VkBufferInfoWrap info)
{
    info.buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    vkapp.buffers.push_back(VkBufferMemWrap());
    VkBufferMemWrap &buf = vkapp.buffers.back();
    VkDevice &dvc = vkapp.logical_dvcs[0]; // TODO: Select logical dvc properly
    vkCreateBuffer(dvc, &info.buffer, nullptr, &buf.buffer);

    uint64_t use_offset = 0;
    VkMemInfoWrap mem_info = {};
    vkGetBufferMemoryRequirements(dvc, buf.buffer, &mem_info.mem);

    if (info.handles.mem_handle == UINT32_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size() - 1;
        VkAllocMemWrap &mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());

        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size() - 1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?
        vkapp_info.mem_allocs.push_back(mem_info);
        info.handles.mem_info_handle = vkapp_info.mem_allocs.size() - 1;
    }
    else
    { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
        buf.mem_handle = info.handles.mem_handle;
    }
    vkBindBufferMemory(dvc, buf.buffer, vkapp.mem[info.handles.mem_handle].mem, use_offset); // Offset? What?

    // Create View for Texel Buffers
    if (info.buffer.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | info.buffer.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        for (auto view_info : info.views)
        {
            view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            view_info.buffer = buf.buffer;

            buf.views.push_back(VkBufferView());
            vkCreateBufferView(dvc, &view_info, nullptr, &buf.views.back());
        }
    }

    // DEBUG INFO
    vkapp_info.buffers.push_back(info);

    return vkapp.buffers.size() - 1;
}

uint32_t mfly::gpu::CreateImage(VkImageInfoWrap info)
{
    // Example Image
    info.img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    vkapp.images.push_back(VkImageMemWrap());
    VkImageMemWrap &img = vkapp.images.back();
    // TODO: Select logical device properly
    VkDevice dvc = vkapp.logical_dvcs[0];
    vkCreateImage(dvc, &info.img, nullptr, &img.img);

    uint64_t use_offset = 0;
    VkMemInfoWrap mem_info = {};
    vkGetImageMemoryRequirements(dvc, img.img, &mem_info.mem);

    if (info.handles.mem_handle == UINT32_MAX)
    {
        vkapp.mem.push_back(VkAllocMemWrap());
        info.handles.mem_handle = vkapp.mem.size() - 1;
        VkAllocMemWrap &mem = vkapp.mem[info.handles.mem_handle];
        use_offset = mem.next_offset;

        mem_info.mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem.size = mem_info.mem_info.allocationSize = mem_info.mem.size;
        mem.next_offset = mem.next_offset + mem.size;
        mem_info.mem_info.memoryTypeIndex = 0; // TODO: Find Using Req???
        vkapp.mem.push_back(VkAllocMemWrap());

        vkAllocateMemory(dvc, &mem_info.mem_info, nullptr, &vkapp.mem.back().mem);
        info.handles.mem_handle = vkapp.mem.size() - 1;
        // How do I deal with the memory allocation?
        // How should it be done, is the AMD VKMemoryAllocator good or better?
        // Is this something that is tied to th ebuffer and should be known always?
        
        vkapp_info.mem_allocs.push_back(mem_info);
        info.handles.mem_info_handle = vkapp_info.mem_allocs.size() - 1;
    }
    else
    { // Using a known allocation
        use_offset = vkapp.mem[info.handles.mem_handle].next_offset;
        vkapp.mem[info.handles.mem_handle].next_offset += mem_info.mem.size;
        img.mem_handle = info.handles.mem_handle;
    }

    vkBindImageMemory(dvc, img.img, vkapp.mem[info.handles.mem_handle].mem, use_offset);

    for (auto view_info : info.views)
    {
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = img.img;

        img.view_handles.push_back(vkapp.img_views.size());
        vkapp.img_views.push_back(VkImageView());
        
        vkCreateImageView(dvc, &view_info, nullptr, &vkapp.img_views.back());
    }

    // DEBUG INFO
    vkapp_info.imgs.push_back(info);

    return vkapp.images.size() - 1;
}

uint32_t mfly::gpu:: AddShader(VkShaderInfoWrap shader_info, uint32_t logical_dvc, uint32_t stage) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_info.code_size;
    create_info.pCode = shader_info.bytecode;
    
    if(shader_info.existing_shader == UINT32_MAX) {
        vkapp.shaders.push_back(VkShaderModuleWrap());
        shader_info.existing_shader = vkapp.shaders.size()-1;
    }
    VkShaderModule& shader = vkapp.shaders[shader_info.existing_shader].shader;
    VkShaderModule tmp;
    VkResult res = vkCreateShaderModule(vkapp.logical_dvcs[logical_dvc], &create_info, nullptr, &tmp);
    if(res != VK_SUCCESS){
        return UINT32_MAX;
    }
    else {
        shader = tmp;
        return shader_info.existing_shader;
    }
}

uint32_t* mfly::gpu::AddShaders(const VkShaderBulk& shader_bulk) {
    uint32_t* handles = new uint32_t[shader_bulk.shader_infos.size()];
    uint32_t shader_num = vkapp.shaders.size();
    vkapp.shaders.reserve(vkapp.shaders.size() + shader_bulk.shader_infos.size());
    for(auto group : shader_bulk.declares) {
        for(int i = group.start; i < group.end; ++i){
            uint32_t ret = AddShader(shader_bulk.shader_infos[i], group.logical_dvc, group.stage);
            if(ret != UINT32_MAX) {
                VkShaderModuleWrap& shader = vkapp.shaders[ret];
                shader.stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader.stage_info.pName = group.name;
                shader.stage_info.stage = (VkShaderStageFlagBits)group.stage;
                shader.stage_info.module = shader.shader;
            }
            handles[i] = ret;
        }
    }
    return handles;
}

uint32_t mfly::gpu::AddFramebuffer(VkFramebufferInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.framebuffers, existing);
    VkFramebufferWrap& fb_wrap = vkapp.framebuffers[existing];

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    std::vector<VkImageView> img_views;
    VecFromHandles(info.img_view_handles, vkapp.img_views, img_views);
    create_info.attachmentCount = img_views.size();
    create_info.pAttachments = img_views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[0], &create_info, nullptr, &fb_wrap.framebuffer);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    return existing;
}

uint32_t mfly::gpu::AddSWCFramebuffer(VkFramebufferInfoWrap info, uint32_t swapchain_handle, uint32_t existing) {
    uint32_t prev_val = existing;
    VkSwapchainWrap& swc_wrap = vkapp.swapchains[swapchain_handle];
    
    existing = PushNonInvalid(vkapp.framebuffers, existing);
    swc_wrap.framebuffers[PushNonInvalid(swc_wrap.framebuffers, existing)] = existing;
    VkFramebuffer& fb = vkapp.framebuffers[existing].framebuffer;
    if(prev_val == existing) vkDestroyFramebuffer(vkapp.logical_dvcs[0], fb,nullptr);

    VkFramebufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    create_info.attachmentCount = swc_wrap.img_views.size();
    create_info.pAttachments = swc_wrap.img_views.data();
    create_info.width = info.extent.width;
    create_info.height = info.extent.height;
    create_info.layers = info.num_layers;
    create_info.renderPass = vkapp.render_passes[info.render_pass_handle];

    // TODO: select logical device
    VkResult res = vkCreateFramebuffer(vkapp.logical_dvcs[0], &create_info, nullptr, &fb);
    if(res != VK_SUCCESS) printf("Failed to create framebuffer");

    uint32_t info_existing = PushNonInvalid(vkapp_info.framebuffers, existing);
    uint32_t fb_info_h = PushNonInvalid(swc_wrap.fb_infos, info_existing);
    swc_wrap.fb_infos[fb_info_h] = info_existing;
    VkFramebufferInfoWrap& fb_info = vkapp_info.framebuffers[info_existing];
    fb_info.extent = info.extent;
    for(int i = 0; i < swc_wrap.img_views.size(); ++i)
        info.img_view_handles.push_back(i);
    fb_info.img_view_handles.swap(info.img_view_handles);
    fb_info.num_layers = info.num_layers;
    fb_info.render_pass_handle;

    return existing;
}

uint32_t mfly::gpu::AddAttachmentDesc(VkAttachmentInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.attachment_descs, existing);
    VkAttachmentDescription& desc = vkapp.attachment_descs[existing];
    
    desc.loadOp = (VkAttachmentLoadOp)info.ops.load;
    desc.storeOp = (VkAttachmentStoreOp)info.ops.store;
    desc.stencilLoadOp = (VkAttachmentLoadOp)info.ops.stencil_load;
    desc.stencilStoreOp = (VkAttachmentStoreOp)info.ops.stencil_store;

    desc.samples = (VkSampleCountFlagBits)info.samples;
    desc.format = (VkFormat)info.format;

    // TODO: Research VkAttachment Description Flags
    
    desc.initialLayout = info.input_layout;
    desc.finalLayout = info.output_layout;

    return existing;
}

uint32_t mfly::gpu::AddSubPass(VkSubPassInfoWrap& info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.subpasses, existing);
    
    VkSubpassDescWrap& sp_wrap = vkapp.subpasses[existing];
    VkSubpassDescription& subpass = sp_wrap.desc;
    VkSubPassInfoWrap& subpass_info = sp_wrap.info;
    subpass_info.framebuffers.swap(info.framebuffers);
    subpass_info.depth_stencil->layout = info.depth_stencil->layout;
    subpass_info.depth_stencil->attachment = info.depth_stencil->attachment;
    subpass_info.inputs.swap(info.inputs);
    subpass_info.preserve.swap(info.preserve);
    subpass_info.sampling_resolves.swap(info.sampling_resolves);

    subpass = {};
    // TODO: Research https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescriptionFlagBits.html
    //subpass.flags
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // TODO: bindpoint to info
    subpass.colorAttachmentCount = subpass_info.framebuffers.size();
    subpass.pColorAttachments = (subpass.colorAttachmentCount == 0) ? nullptr : subpass_info.framebuffers.data();
    subpass.inputAttachmentCount = subpass_info.inputs.size();
    subpass.pInputAttachments = (subpass.inputAttachmentCount == 0) ? nullptr : subpass_info.inputs.data();
    //subpass.pDepthStencilAttachment = subpass_info.depth_stencil; // Not used for now
    subpass.preserveAttachmentCount = subpass_info.preserve.size();
    subpass.pPreserveAttachments = (subpass.preserveAttachmentCount == 0) ? nullptr : subpass_info.preserve.data();
    subpass.pResolveAttachments = (subpass_info.sampling_resolves.size() == 0) ? nullptr : subpass_info.sampling_resolves.data();
    
    return existing;
}


uint32_t mfly::gpu::CreateRenderPass(VkRenderPassInfoWrap info, uint32_t existing) {
    bool destroy_pass = existing != UINT32_MAX;
    PushNonInvalid(vkapp.render_passes, existing);
    existing = PushNonInvalid(vkapp.render_pass_infos, existing);
    VkRenderPass* pass = &vkapp.render_passes[existing];
    VkRenderPassInfoWrap& pass_info = vkapp.render_pass_infos[existing];

    if(destroy_pass) {
        // TODO: Select proper device
        vkDestroyRenderPass(vkapp.logical_dvcs[0], *pass, nullptr);
    }

    pass_info = info;
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = pass_info.attachment_handles.size();
    std::vector<VkAttachmentDescription> attachments;
    VecFromHandles(pass_info.attachment_handles, vkapp.attachment_descs, attachments);
    create_info.pAttachments = attachments.data();
    
    std::vector<VkSubpassDescWrap> subpasses;
    VecFromHandles(pass_info.subpass_handles, vkapp.subpasses, subpasses);
    std::vector<VkSubpassDescription> sp_descs;
    for(auto sp : subpasses) {sp_descs.push_back(sp.desc);}
    create_info.subpassCount = sp_descs.size();
    create_info.pSubpasses = sp_descs.data();
    // TODO: Research RenderPass Flags, Dependencies and pNext

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Wether they are internal subpasses or from other renderpasses
    dependency.dstSubpass = 0; // Index of the subpass?, per subpass dependency creation...
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What to wait on
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What waits on this
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Define the dependencies
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;


    VkResult res = vkCreateRenderPass(vkapp.logical_dvcs[0], &create_info, nullptr, pass);

    if(res != VK_SUCCESS)
        printf("Failed to create rende pass");

    return vkapp.render_passes.size()-1; // Always return, if it failed, you have to repair it or will crash somewhere else
}

uint32_t mfly::gpu::CreateGraphicsPipeline(VkGraphicsPipeStateInfoWrap pipe_info) {
    VkPipelineVertexInputStateCreateInfo vtx_info = {};
    vtx_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Function to set descriptors from binding descriptors and attribute descriptors
    vtx_info.vertexBindingDescriptionCount = vtx_info.vertexAttributeDescriptionCount = 0;
    vtx_info.pVertexBindingDescriptions = nullptr;
    vtx_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_info = {};
    input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_info.topology = (VkPrimitiveTopology)pipe_info.input_info.topology;
    input_info.primitiveRestartEnable = pipe_info.input_info.restart_on_indexed;

    VkPipelineDynamicStateCreateInfo dyn_state = {};
    dyn_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_state.dynamicStateCount = pipe_info.dyn_states.size();
    dyn_state.pDynamicStates = pipe_info.dyn_states.data();

    VkPipelineViewportStateCreateInfo vp_state = {};
    vp_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp_state.viewportCount = pipe_info.viewports.size();
    vp_state.pViewports = pipe_info.viewports.data();
    vp_state.scissorCount = pipe_info.scissors.size();
    vp_state.pScissors = pipe_info.scissors.data();

    VkPipelineRasterizationStateCreateInfo raster_state = {};
    raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_state.depthClampEnable = pipe_info.raster_info.clamp_depth;
    raster_state.rasterizerDiscardEnable = pipe_info.raster_info.disable_all;
    
    raster_state.polygonMode = (VkPolygonMode)pipe_info.raster_info.poly_mode;
    raster_state.lineWidth = 1.0f; // TODO: Add to raster info
    
    raster_state.cullMode = pipe_info.raster_info.cull_mode;
    raster_state.frontFace = (VkFrontFace)pipe_info.raster_info.front_face;
    
    raster_state.depthBiasEnable = pipe_info.raster_info.bias;
    raster_state.depthBiasConstantFactor = pipe_info.raster_info.bias_constant;
    raster_state.depthBiasClamp = pipe_info.raster_info.bias_clamp;
    raster_state.depthBiasSlopeFactor = pipe_info.raster_info.bias_slope;

    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.sampleShadingEnable = pipe_info.multisample_info.per_sample;
    multisample_state.rasterizationSamples = (VkSampleCountFlagBits)pipe_info.multisample_info.num_samples;
    multisample_state.minSampleShading = pipe_info.multisample_info.min_per_samples;
    multisample_state.pSampleMask = nullptr; // TODO: Add holder of masks
    multisample_state.alphaToCoverageEnable = pipe_info.multisample_info.alpha_coverage;
    multisample_state.alphaToOneEnable = pipe_info.multisample_info.set_alpha_to_one;

    // TODO: Depth and Stencil Testing

    VkPipelineColorBlendStateCreateInfo blend_state = {};
    blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_state.logicOpEnable = pipe_info.blend_info.logic_blend;
    blend_state.logicOp = (VkLogicOp)pipe_info.blend_info.logic_op;

    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    for(auto attachment : pipe_info.blend_info.attachments) {
        attachments.push_back(VkPipelineColorBlendAttachmentState());
        VkPipelineColorBlendAttachmentState& curr = attachments.back();
        curr.colorWriteMask = attachment.channel_mask;
        curr.blendEnable = attachment.blend;
        curr.srcColorBlendFactor = (VkBlendFactor)attachment.src_color;
        curr.dstColorBlendFactor = (VkBlendFactor)attachment.dst_color;
        curr.colorBlendOp = (VkBlendOp)attachment.color_op;
        curr.srcAlphaBlendFactor = (VkBlendFactor)attachment.src_alpha;
        curr.dstAlphaBlendFactor = (VkBlendFactor)attachment.dst_alpha;
        curr.alphaBlendOp = (VkBlendOp)attachment.alpha_op;
    }
    blend_state.attachmentCount = pipe_info.blend_info.attachments.size();
    blend_state.pAttachments = attachments.data();
    for(int i = 0; i < pipe_info.blend_info.constants.size(); ++i) {
        blend_state.blendConstants[i] = pipe_info.blend_info.constants[i];
    }

    VkPipelineLayoutCreateInfo layout_state = {};
    layout_state.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // TODO: Get layout sets from handles and way to store them
    layout_state.setLayoutCount = pipe_info.layout_info.count;
    layout_state.pSetLayouts = nullptr;
    // TODO: Get constants from handles and way to store them
    layout_state.pushConstantRangeCount = pipe_info.layout_info.push_constants_handles.size();
    layout_state.pPushConstantRanges = nullptr;

    vkapp.graphic_pipes.push_back(VkGraphicsPipelineWrap());
    VkGraphicsPipelineWrap& pipe_wrap = vkapp.graphic_pipes.back();
    VkPipelineLayout* pipe = &pipe_wrap.layout;
    // TODO: Way to select proper device
    VkResult res = vkCreatePipelineLayout(vkapp.logical_dvcs[0], &layout_state, nullptr, pipe);
    if(res != VK_SUCCESS)
        printf("Failed to create pipeline layout\n"); 

    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = pipe_info.num_stages;
    std::vector<VkShaderModuleWrap> shaders;
    VecFromHandles(pipe_info.shaders, vkapp.shaders, shaders);
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
    for(auto shader : shaders) {
        shader_stage_infos.push_back(shader.stage_info);
        VkPipelineShaderStageCreateInfo& sh_info = shader_stage_infos.back();
        sh_info.pName = pipe_info.name;
    }   
    create_info.pStages = shader_stage_infos.data();

    create_info.stageCount = shader_stage_infos.size();
    create_info.pStages = shader_stage_infos.data();

    create_info.pVertexInputState = &vtx_info;
    create_info.pInputAssemblyState = &input_info;
    create_info.pViewportState = &vp_state;
    create_info.pRasterizationState = &raster_state;
    create_info.pMultisampleState = &multisample_state;
    create_info.pDepthStencilState = nullptr; // TODO
    create_info.pColorBlendState = &blend_state;
    create_info.pDynamicState = &dyn_state;
    create_info.layout = *pipe;
    
    create_info.renderPass = vkapp.render_passes[pipe_info.render_pass_handle];
    create_info.subpass = 0; // TODO: Properly set the subpass
    
    create_info.basePipelineHandle = VK_NULL_HANDLE; // TODO: Deriving from previously created / Requires flag VK_PIPELINE_CRETE_DERIVATIVE_BIT
    create_info.basePipelineIndex = -1; // TODO: index from the one that are about to be created... prefer handle maybe

    res = vkCreateGraphicsPipelines(vkapp.logical_dvcs[0], VK_NULL_HANDLE, 1, &create_info, nullptr, &pipe_wrap.pipe);
    // TODO: Allow multiple pipelines to be created at the same time
    if(res != VK_SUCCESS)
        printf("Failed to create graphics pipeline");

    return 0;
};

uint32_t mfly::gpu::AddCmdPool(VkCmdPoolInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.cmd_pools, existing);
    VkCommandPool* pool = &vkapp.cmd_pools[existing];

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = info.flags;
    create_info.queueFamilyIndex = info.queue_family;

    // TODO: Proper device selection
    VkResult res = vkCreateCommandPool(vkapp.logical_dvcs[0], &create_info, nullptr, pool);
    if(res != VK_SUCCESS) printf("Failed to create queue");

    return existing;
}

uint32_t mfly::gpu::AddCmdBuffers(VkCmdBufInfoWrap info) {
    uint32_t first = vkapp.cmd_buffers.size();
    for(int i = 0; i < info.count; ++i)
        PushNonInvalid(vkapp.cmd_buffers, UINT32_MAX);
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vkapp.cmd_pools[info.pool_handle];
    alloc_info.level = (VkCommandBufferLevel)info.level;
    alloc_info.commandBufferCount = info.count;    

    // TODO: Properly select logical dvc
    VkResult res = vkAllocateCommandBuffers(vkapp.logical_dvcs[0], &alloc_info, &vkapp.cmd_buffers[first]);
    if(res != VK_SUCCESS) printf("Faield to create command buffers");

    return first;
}

uint32_t mfly::gpu::AddRecordBegin(VkBeginInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.begin_cmd_infos, existing);
    VkCommandBufferBeginInfo& create_info = vkapp.begin_cmd_infos[existing];
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    create_info.flags = info.flags;
    create_info.pInheritanceInfo = nullptr; // TODO: This should extract info from a CommandBufferWrap of a primary command buffer... 
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferInheritanceInfo.html

    return existing;
}

VkCommandBuffer mfly::gpu::BeginRecord(uint32_t begin_handle, uint32_t cmd_buf_handle) {
    VkCommandBufferBeginInfo info = vkapp.begin_cmd_infos[begin_handle];
    VkCommandBuffer cmd_buf = vkapp.cmd_buffers[cmd_buf_handle];

    vkResetCommandBuffer(cmd_buf, 0);
    VkResult res = vkBeginCommandBuffer(cmd_buf, &info);
    if(res != VK_SUCCESS) printf("Failed to begin command buffer");
    return cmd_buf;
}

uint32_t mfly::gpu::AddRenderPassBegin(VkBeginRenderPassInfoWrap info, uint32_t existing) {
    existing = PushNonInvalid(vkapp.begin_renderpass_infos, existing);
    VkRenderPassBeginInfoWrap& bri_wrap  = vkapp.begin_renderpass_infos[existing];
    info.clear_colors.swap(bri_wrap.colors);
    bri_wrap.create_info = {};
    bri_wrap.create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    bri_wrap.create_info.renderPass = vkapp.render_passes[info.render_pass_handle];
    bri_wrap.create_info.framebuffer = vkapp.framebuffers[info.framebuffer_handle].framebuffer;
    bri_wrap.create_info.renderArea.extent = info.extent;
    bri_wrap.create_info.renderArea.offset = info.offset;
    bri_wrap.create_info.pClearValues = bri_wrap.colors.data();
    bri_wrap.create_info.clearValueCount = bri_wrap.colors.size();

    return existing;
}

uint32_t mfly::gpu::BeginRenderPass(uint32_t begin_renderpass_handle, VkCommandBuffer cmd_buf) {
    VkRenderPassBeginInfoWrap& info = vkapp.begin_renderpass_infos[begin_renderpass_handle];

    // TODO: Change the last parameter based on required subpasses or not (currently only primary passes)
    info.create_info.framebuffer = vkapp.framebuffers[0].framebuffer;
    info.create_info.renderArea.extent = vkapp.swapchains[0].area;
    vkCmdBeginRenderPass(cmd_buf, &info.create_info, VK_SUBPASS_CONTENTS_INLINE);
    return 0;
}

uint32_t mfly::gpu::BindPipeline(uint32_t pipeline_handle, VkCommandBuffer cmd_buf) {
    VkGraphicsPipelineWrap pipe = vkapp.graphic_pipes[pipeline_handle];

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipe);

      

    return 0;
}

uint32_t mfly::gpu::SetDynState(VkCommandBuffer cmd_buf) {
    // Temp
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vkapp.swapchains[0].area.width);
    viewport.height = static_cast<float>(vkapp.swapchains[0].area.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vkapp.swapchains[0].area;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
    // *Temp  

    return 0;
}

VkDevice mfly::gpu::GetLogicalDevice(uint32_t logical_dvc_handle) { return vkapp.logical_dvcs[logical_dvc_handle]; }

std::pair<uint32_t, VkSemaphore> mfly::gpu::AddSemaphore(uint32_t existing) {
    uint32_t prev_val = existing;
    existing = PushNonInvalid(vkapp.semaphores, existing);
    VkSemaphore* semaphore = &vkapp.semaphores[existing];
    if(existing == prev_val) vkDestroySemaphore(vkapp.logical_dvcs[0], *semaphore, nullptr);

    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // TODO: Properly select logical device
    vkCreateSemaphore(vkapp.logical_dvcs[0], &info, nullptr, semaphore);

    return std::pair<uint32_t, VkSemaphore>(existing, *semaphore);
}

std::pair<uint32_t, VkFence> mfly::gpu::AddFence(uint32_t existing) {
    uint32_t prev_val = existing;
    existing = PushNonInvalid(vkapp.fences, existing);
    VkFence* fence = &vkapp.fences[existing];
    if(existing == prev_val) vkDestroyFence(vkapp.logical_dvcs[0], *fence, nullptr);

    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    // TODO: Properly select logical device
    vkCreateFence(vkapp.logical_dvcs[0], &info, nullptr, fence);

    return std::pair<uint32_t, VkFence>(existing, *fence);
}

// Module Basics
uint32_t mfly::gpu::DefaultInit()
{
    uint32_t ret = 0;

    VkInstanceInfoWrap app_info;
    app_info.exts.push_back("VK_KHR_surface");
    app_info.exts.push_back("VK_KHR_win32_surface");

    app_info.layers.push_back("VK_LAYER_KHRONOS_validation");
    InitVkInstance(app_info);

    // assert(vkPDeviceCount > 0);
    VkPhysicalDevice dev = vkapp.phys_dvcs[0]; // Grab Pointer to device in use

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

    InitQueues(l_dvc_info, 0);

    // Ask window manager to get a surface
    vkapp.main_surface = (VkSurfaceKHR)vkapp.get_surface((void *)vkapp.instance, 0); // use main window for main surface
    vkapp.surfaces.push_back(vkapp.main_surface);

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

    uint32_t sch_handle = CreateSwapchain(swapchain_info, 0, 0);

    // Swapchain requires a set of framebuffers and attachments to draw too
    // Requires a vec of Framebuffers and their respecitve imageviews


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
    VkBufferViewCreateInfo &view_info = buf_info_txl.views.back();
    view_info = {};
    view_info.offset = 0;
    view_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    view_info.range = VK_WHOLE_SIZE;

    CreateBuffer(buf_info_txl);

    // Example Image
    VkImageInfoWrap img_info;
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

    img_info.handles.logical_dvc_handle = 0;
    CreateImage(img_info);

    // DEBUG INFO
    VkAppInfo &t_info = vkapp_info;

    return ret;
}

uint32_t mfly::gpu::Close()
{
    uint32_t ret = 0;

    for(auto dvc : vkapp.logical_dvcs){
        vkDeviceWaitIdle(dvc);
    }

    // Most nullptr here are baseon pAllocatorCallback which I do not know what is it
    // Something for custom allocators like AMD vulkan mem allocation?
    vkDestroyDescriptorPool;
    vkDestroyDescriptorSetLayout;
    vkDestroySampler;
    
    for(auto pool : vkapp.cmd_pools) {
        vkDestroyCommandPool(vkapp.logical_dvcs[0], pool, nullptr);
    }

    for(auto fb_wrap : vkapp.framebuffers) {
        vkDestroyFramebuffer(vkapp.logical_dvcs[0], fb_wrap.framebuffer, nullptr);
    }

    for(auto semaphore : vkapp.semaphores) {
        vkDestroySemaphore(vkapp.logical_dvcs[0], semaphore, nullptr);
    }
    for(auto fence : vkapp.fences) {
        vkDestroyFence(vkapp.logical_dvcs[0], fence, nullptr);
    }

    for(auto pipe : vkapp.graphic_pipes) {
        vkDestroyPipeline(vkapp.logical_dvcs[0], pipe.pipe, nullptr);
        vkDestroyPipelineLayout(vkapp.logical_dvcs[0], pipe.layout, nullptr);
    }
    for(auto pass : vkapp.render_passes) {
        vkDestroyRenderPass(vkapp.logical_dvcs[0], pass, nullptr);
    }

    for (auto scw : vkapp.swapchains) {
        DestroySwapchain(scw);
    }
    
    {
        VkDevice dvc = vkapp.logical_dvcs[0];
        for (auto buf : vkapp.buffers)
        {
            for(auto bufview : buf.views){
                vkDestroyBufferView(dvc, bufview, nullptr);
            }
            vkDestroyBuffer(dvc, buf.buffer, nullptr);
        }

        for (auto imgview : vkapp.img_views) {
            vkDestroyImageView(dvc, imgview, nullptr);
        }

        for (auto img : vkapp.images) {
            vkDestroyImage(dvc, img.img, nullptr);
        }
    }

    for (auto dvc : vkapp.logical_dvcs) {
        vkDestroyDevice(dvc, nullptr);
    }

    vkapp.phys_dvcs.clear(); // memleak...
    
    vkDestroyInstance(vkapp.instance, nullptr);
    return ret;
}

// temporary variables

uint32_t mfly::gpu::PreUpdate()
{
    uint32_t ret = 0;

    // Acquire image to draw on
    for(int i = 0; i < vkapp.swapchains.size(); ++i)
    {  
        VkSwapchainWrap& swc_wrap = vkapp.swapchains[i];
        vkWaitForFences(vkapp.logical_dvcs[0], 1, &swc_wrap.img_fence, true, UINT64_MAX);
        if(swc_wrap.need_resize) 
            RecreateSwapchain(i);
        VkResult res = vkAcquireNextImageKHR(vkapp.logical_dvcs[0], swc_wrap.swapchain,
                              UINT64_MAX,            // Max time to wait for image (usually as much as possible)
                              swc_wrap.img_semaphore, // Signal when acquired -> Wait on GPU
                              VK_NULL_HANDLE,     // Signal when acquired -> Wait on CPU // Not needed for now
                              &swc_wrap.curr_image);  // Which image to use from the handles of images in the swapchain
                              
        if(res == VK_ERROR_OUT_OF_DATE_KHR /*|| res == VK_SUBOPTIMAL_KHR*/)
            RecreateSwapchain(i);
        else 
            vkResetFences(vkapp.logical_dvcs[0], 1, &swc_wrap.img_fence);
        
    }
    // Further Explanation on Semaphores and Fences
    // BINARY Semaphores are GPU Signals, it is sent to Vulkan for a task to signal when finish
    // Another task uses that same semaphore to wait until it is signaled to now start working
    // In that way, a semaphore can be passed to be signaled by only 1 task, but multiple can wait on it
    // However, a task can only wait on a single semaphore, we can branch out but not back in

    // For that we need Timeline Semaphores (TODO)

    // Fences are for CPU to wait on GPU signals
    return ret;
}

uint32_t mfly::gpu::DoUpdate()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::gpu::PostUpdate()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::gpu::AsyncDispatch()
{
    uint32_t ret = 0;

    return ret;
}

uint32_t mfly::gpu::AsyncGather()
{
    uint32_t ret = 0;

    return ret;
}

// Module Specificss
void mfly::gpu::ProvideSurfaceFun(GetSurfaceFun fun)
{
    vkapp.get_surface = fun;
}