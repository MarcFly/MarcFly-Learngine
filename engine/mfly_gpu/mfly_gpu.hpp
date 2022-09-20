#ifndef MFLY_GPU
#define MFLY_GPU

#include <stdint.h>
#include <vulkan/vulkan.hpp>

// UNDERSTAND MACRO TO DISABLE OPTIMIZATIONS IN VMKGL
#ifdef MEM_OPTIMIZE
#undef MEMOPT;
#define MEMOPT(a) ;
#define MEMOPT_ALT(a) a;
#else
#define MEMOPT(a) a;
#undef MEMOPT_ALT;
#define MEMOPT_ALT(a) ; 
#endif

namespace mfly
{
    namespace gpu
    {
        // Module Functions
        uint16_t DefaultInit();

        // Function to gather Vulkan Instance possible Extensions
        struct VkPhysDeviceInfoWrap {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
        };

        struct VkInstanceInfoWrap{
            VkApplicationInfo app_info = {};
            VkInstanceCreateInfo create_info = {};
            std::vector<const char*> exts;
            std::vector<const char*> layers;
            
            std::vector<VkExtensionProperties> available_exts;
        };
        uint16_t InitVkInstance(VkInstanceInfoWrap& instance_info); // Startup Vulkan
        
        struct VkLDeviceInfoWrap {
            std::vector<VkDeviceQueueCreateInfo> queues_info;
            std::vector<std::vector<float>> prios;
            VkDeviceCreateInfo create_info = {};
            std::vector<const char*> exts;
            std::vector<void*> exts_info;
        };
        uint16_t InitQueues(VkLDeviceInfoWrap& info, uint16_t phys_dvc_handle = 0);
        VkQueue RetrieveQueue(uint32_t device_handle, uint32_t family, uint32_t index);
        VkDevice GetLogicalDevice(uint32_t logical_dvc_handle);
        uint16_t CreateSwapchain(VkSwapchainCreateInfoKHR info, uint16_t surface_handle, uint16_t logical_dvc_handle);
        
        struct VkFramebufferWrap {
            VkFramebuffer framebuffer;
            uint32_t img_view_handle;
        };

        struct VkSwapchainWrap
        {
            VkSwapchainKHR swapchain;
            uint16_t logical_dvc_handle;
            std::vector<VkImage> images;
            std::vector<uint32_t> img_view_handles;
            VkExtent2D area;
            uint32_t curr_image;

            VkSemaphore img_semaphore;
            VkFence img_fence;

            std::vector<VkFramebufferWrap> framebuffers; // Should they be associated?
        };
        const VkSwapchainWrap RetrieveSwapchain(uint32_t swapchain_handle);

        struct VkShaderInfoWrap{
            uint32_t* bytecode;
            uint64_t code_size;
            uint16_t existing_shader = UINT16_MAX;
        };
        
        struct VkShaderStageInfoWrap {
            uint32_t start, end; // Uses 2 values for start and end when used in a bulk
            // Recreating a pipeline should be asking the previously created pipeline again...
            
            uint16_t logical_dvc;
            uint32_t stage;
            const char* name;
        };
        
        struct VkDynamicPipeStateInfoWrap {
            std::vector<VkDynamicState> states;
            // more?
            // Not strictly needed
        };

        struct VkVtxPipeStateInfoWrap {
            uint32_t vtx_attribute_descriptor_handle;
            uint32_t vtx_binding_descriptor_handle;
            // These must be uploaded previously and known
            // They will get back the intended data pointer and size
        };
        struct VkInputPipeStateInfoWrap {
            uint32_t topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            uint32_t restart_on_indexed = false;
            // When Drawing with indices, a vertex with value 0xFF / 0xFFFFFFFF / 0xFFF, it loops the indices i think, research more
        };

        struct VkRasterPipeStateInfoWrap {
            bool disable_all = false; // TRUE for not drawing anything
            bool clamp_depth = false; // TRUE for letting discarded geometry to be draw (no depth discard...)
            uint32_t poly_mode = VK_POLYGON_MODE_FILL; // LINE = Draw edges / POINT = Draw vertices / FILL = Draw faces
            uint32_t cull_mode = VK_CULL_MODE_BACK_BIT;
            uint32_t front_face = VK_FRONT_FACE_CLOCKWISE; // Given triangle a,b,c -> draw vertices in a->c or a<-c orders...
            
            bool bias = false;
            float bias_constant = 0; 
            float bias_clamp = 0;
            float bias_slope = 0;
        };
        typedef VkRasterPipeStateInfoWrap RasterInfo;

        struct VkMultiSamplePipeStageInfoWrap {
            uint32_t num_samples = VK_SAMPLE_COUNT_1_BIT; // samples per pixel, multiples of 2
            
            bool per_sample = false; // Multisample every sample not only edges
            uint32_t min_per_samples = 1.0f; // shade rate per fragment
            uint32_t sample_mask_handle; // Handle to the sample VkSampleMask to use, needs to be previously uploaded and known
            bool alpha_coverage = false; // Temp alpha values controls coverage????
            bool set_alpha_to_one = false; // ????
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#fragops-covg
        };
        typedef VkMultiSamplePipeStageInfoWrap MultiSampleInfo;

        struct VkColorBlendAttachmentInfoWrap {
            uint32_t channel_mask = 1 | 2 | 4 | 8; // R G B A / Which channels are writable
            bool blend = false;

            // Default to Alpha Blending when true
            uint32_t src_color = VK_BLEND_FACTOR_SRC_ALPHA;
            uint32_t dst_color = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            uint32_t color_op = VK_BLEND_OP_ADD;

            uint32_t src_alpha = VK_BLEND_FACTOR_ONE;
            uint32_t dst_alpha = VK_BLEND_FACTOR_ZERO;
            uint32_t alpha_op = VK_BLEND_OP_ADD; 
        };
        typedef VkColorBlendAttachmentInfoWrap ColorBlendInfo;

        struct VkBlendPipeStateInfoWrap {
            bool logic_blend = false;
            uint32_t logic_op = VK_LOGIC_OP_COPY;
            std::vector<ColorBlendInfo> attachments;
            std::vector<float> constants; // ?
        };

        struct VkLayoutPipeStageInfoWrap {
            uint16_t count = 0;
            std::vector<uint32_t> descriptor_sets_handles; // Assume precreated
            std::vector<uint32_t> push_constants_handles; // Assume precreated
        };
        typedef VkLayoutPipeStageInfoWrap PipeLayoutInfo;
        
        struct VkGraphicsPipelineInfoWrap {

        };

        struct VkGraphicsPipeStateInfoWrap {
            std::vector<VkDynamicState> dyn_states;
            uint32_t num_stages = 2; // At least Vertex and Raster?
            VkVtxPipeStateInfoWrap vtx_info;
            VkInputPipeStateInfoWrap input_info;
            // Multiple viewports requires VK_KHR_MULTIVIEW
            std::vector<VkViewport> viewports;
            std::vector<VkRect2D> scissors;

            VkRasterPipeStateInfoWrap raster_info;
            VkMultiSamplePipeStageInfoWrap multisample_info;
            // Depth Testing... https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
            VkBlendPipeStateInfoWrap blend_info;
            PipeLayoutInfo layout_info;

            const char* name;
            std::vector<uint32_t> shaders;
            uint32_t render_pass_handle;
        };

        uint16_t CreateGraphicsPipeline(VkGraphicsPipeStateInfoWrap pipe_info);

        struct VkFramebufferInfoWrap {
            uint32_t render_pass_handle;
            std::vector<uint32_t> img_view_handles; // Framebuffers are linked to previously created IMAGES (agora sim entendo)
            VkExtent2D extent;
            uint32_t num_layers;
        };
        uint32_t AddFramebuffer(VkFramebufferInfoWrap fb_info, uint32_t existing = UINT32_MAX);
        
        // Not usable?
        struct VkAttachmentInfoWrap {
            uint32_t format = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat...
            uint32_t samples = VK_SAMPLE_COUNT_1_BIT; // 1 Bit per sample VkSampleCountFlagBits
            struct {
                uint32_t load = VK_ATTACHMENT_LOAD_OP_CLEAR; // What to do before rendering
                uint32_t store = VK_ATTACHMENT_STORE_OP_STORE; // What to do after rendering
                uint32_t stencil_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                uint32_t stencil_store = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            } ops;
            VkImageLayout input_layout = VK_IMAGE_LAYOUT_UNDEFINED; // Which type of image comes, not needed but can be optimized if known
            VkImageLayout output_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image for a swapchain
            // CHANGE for color attachment or transfer/copy operations!
        };     
        uint32_t AddAttachmentDesc(VkAttachmentInfoWrap info, uint32_t existing = UINT32_MAX);

        struct VkSubPassInfoWrap {
            std::vector<VkAttachmentReference> framebuffers;
            // Bind them depending on the pipeline they reference
            std::vector<VkAttachmentReference> inputs;
            VkAttachmentReference depth_stencil[2];
    
            // Not used for now at all
            std::vector<VkAttachmentReference> sampling_resolves;
            std::vector<uint32_t> preserve; // TODO: Reasearch preserve attachments from VkSubpassDescription
        };
        uint32_t AddSubPass(VkSubPassInfoWrap& info, uint32_t existing = UINT32_MAX); // Invalidates vectors!
        
        struct VkRenderPassInfoWrap {
            std::vector<uint32_t> attachment_handles;
            std::vector<uint32_t> subpass_handles;
            uint32_t pipeline_handle;
        };

        uint32_t CreateRenderPass(VkRenderPassInfoWrap info, uint32_t existing = UINT32_MAX);
        uint32_t RegenRenderPass(uint32_t handle); // For when known subpasses are changed
        // Ideally, a RegenAllRenderPasses which takes into account versioning of their subpasses?

        struct VkShaderBulk {
            std::vector<VkShaderInfoWrap> shader_infos;
            std::vector<VkShaderStageInfoWrap> declares;
        };

        uint16_t AddShader(VkShaderInfoWrap shader_info, uint16_t logical_dvc, uint32_t stage);
        uint16_t* AddShaders(const VkShaderBulk& shader_bulk);

        struct VkCmdPoolInfoWrap {
            uint32_t flags = 0; // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = commands are recorded frequently // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = commands recorded in entire group
            uint32_t queue_family = 0; // 0 is graphics family
        };
        uint32_t AddCmdPool(VkCmdPoolInfoWrap info, uint32_t existing = UINT32_MAX);

        struct VkCmdBufInfoWrap {
            uint32_t pool_handle;
            uint32_t level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VK_COMMAND_BUFFER_LEVEL_PRIMARY = Submittable - not callable // VK_COMMAND_BUFFER_LEVEL_SECONDARY = Not Submittable - callable
            // Huh?
            uint32_t count = 1;
        };
        uint32_t AddCmdBuffers(VkCmdBufInfoWrap info);
        
        struct VkBeginInfoWrap {
            uint32_t flags = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = Rerecorder after being submitted 
            // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = Secondary and will only be used within a single render passs
            // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = will be resubmitted multiple times during execution
            uint32_t parent_handle = UINT32_MAX;
        };
        uint32_t AddRecordBegin(VkBeginInfoWrap info, uint32_t existing = UINT32_MAX);
        VkCommandBuffer BeginRecord(uint32_t begin_handle, uint32_t cmd_buf_handle);

        struct VkBeginRenderPassInfoWrap {
            uint32_t render_pass_handle;
            uint32_t framebuffer_handle;
            VkOffset2D offset;
            VkExtent2D extent;
            std::vector<VkClearValue> clear_colors;
        };
        uint32_t AddRenderPassBegin(VkBeginRenderPassInfoWrap info, uint32_t existing = UINT32_MAX);
        uint32_t BeginRenderPass(uint32_t begin_handle, VkCommandBuffer cmd_buf);
        // When adding commands seems like everyone requires the same commandbuffer
        // Would be sensible to pass the actual data instead of a handle to avoid fetches to foreign memory?
        uint32_t BindPipeline(uint32_t pipeline_handle, VkCommandBuffer cmd_buf); // Right now only graphic pipelines...
        uint32_t SetDynState(VkCommandBuffer cmd_buf); // Dyn state depends on each pipeline defined... how to pass such variable struct?
        /// What feels ideal for this setup is something like a CommadnBufferWrap
        /// The struct holds directly a CommandBuffer { vec<DrawPipelineWrap> {pipeline, dynstate, vec<draw_commands>}}
        /// *To be amplified with descriptors, descriptor sets, binding and whatever,
        /// Thus you should only need to call to draw a commandbuffer, which previously you've set its info 

        uint16_t Close();
        uint16_t PreUpdate();
        uint16_t DoUpdate();
        uint16_t PostUpdate();
        uint16_t AsyncDispatch();
        uint16_t AsyncGather();

        //uint16_t event_code_start = 0;

        enum ERRORCODE
        {
            GOOD = 0,
            BAD = 1,
        };
#if defined(DEBUG_STRINGS)
        const char*[] debug_strings
        {
            "Good, move on.",
            "Something went wrong, message not set",
        }
        const char* DebugMessage(uint16_t errorcode) {
            if(errorcode > (sizeof(debug_string)/sizeof(const char*)))
                return debug_string[1];
            else
                return debug_string[errorcode];
        }
#endif

        // Module Specifics
        /// void*: VkInstance the library provides and the window api most likely requires
        /// uint16_t: Window number that we want a surface from (usually the main window - 0)
        /// expected result: VkSurfaceKHR
        typedef void*(*GetSurfaceFun)(void*, uint16_t);
        void ProvideSurfaceFun(GetSurfaceFun fun);

        struct VkMemInfoWrap {
            VkMemoryRequirements mem = {};
            VkMemoryAllocateInfo mem_info = {};
        };
        // Memory Allocation?
        struct VkDMEMHandles {
            uint16_t logical_dvc_handle = 0;
            uint16_t mem_handle = -1;
            uint32_t mem_info_handle = -1;
        };
        struct VkBufferInfoWrap {
            VkBufferCreateInfo buffer = {};
            VkDMEMHandles handles;
            std::vector<VkBufferViewCreateInfo> views;
        };
        uint32_t CreateBuffer(VkBufferInfoWrap info);
        
        struct VkImageInfoWrap {
            VkImageCreateInfo img = {};
            VkDMEMHandles handles;
            std::vector<VkImageViewCreateInfo> views = {};
        };
        uint32_t CreateImage(VkImageInfoWrap info);

        VkSemaphore AddSemaphore();
        VkFence AddFence();

    };
};

#endif // MFLY_GPU