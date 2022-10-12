#ifndef MFLY_GFX_PIPE
#define MFLY_GFX_PIPE

#include <vulkan/vulkan.hpp>
#include <mfly_slotmap.h>

namespace mfly::vk {
    struct VkDynamicPipeStateInfoWrap {
            std::vector<VkDynamicState> states;
            // more?
            // Not strictly needed
        };

        struct VkVtxPipeStateInfoWrap {
            sm_key vtx_attribute_descriptor_handle;
            sm_key vtx_binding_descriptor_handle;
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
            uint32_t count = 0;
            std::vector<sm_key> descriptor_sets_handles; // Assume precreated
            std::vector<sm_key> push_constants_handles; // Assume precreated
        };
        typedef VkLayoutPipeStageInfoWrap PipeLayoutInfo;

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
            std::vector<sm_key> shaders;
            sm_key render_pass_handle;
        };

        sm_key CreateGraphicsPipeline(sm_key& dvc_handle, VkGraphicsPipeStateInfoWrap pipe_info);

        struct VkGraphicsPipelineWrap {
            VkPipelineLayout layout;
            VkPipeline pipe;
            // All the other things
        };

    //===================================================
    void SetDynState(sm_key& swapchain_handle, VkCommandBuffer cmd_buf); // Dyn state depends on each pipeline defined... how to pass such variable struct?
    /// What feels ideal for this setup is something like a CommadnBufferWrap
    /// The struct holds directly a CommandBuffer { vec<DrawPipelineWrap> {pipeline, dynstate, vec<draw_commands>}}
    /// *To be amplified with descriptors, descriptor sets, binding and whatever,
    /// Thus you should only need to call to draw a commandbuffer, which previously you've set its info 

}

#endif