#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

#include <cstdio>
#include <iostream>

#include <mfly_window/mfly_window.hpp>
#include <mfly_vk/mfly_vk.hpp>
#include <mfly_shaders/mfly_shaders.hpp>
#include <sque_timer.h>

const char* shaders[] = {
        "#version 450\nlayout(location = 0) out vec3 fragColor;\nvec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5));\n \
vec3 colors[3] = vec3[]( \
    vec3(1.0, 0.0, 0.0), \
    vec3(0.0, 1.0, 0.0), \
    vec3(0.0, 0.0, 1.0) \
); \
void main() { \
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); \
    fragColor = colors[gl_VertexIndex];}", 
    "#version 450\nlayout(location = 0) in vec3 fragColor; \
layout(location = 0) out vec4 outColor; \
void main() { \
    outColor = vec4(fragColor, 1.0);}"
    };

std::vector<uint32_t> shader_handles;

void TestShaderLib() {
    mfly::shaders::AddDefines("MY_DEFINE", "1");
    
    
    mfly::shaders::Shader raw_s[2];
    raw_s[0].code = shaders[0];
    raw_s[0].type = mfly::shaders::ShaderKind::VERTEX;
    raw_s[1].code = shaders[1];
    raw_s[1].type = mfly::shaders::ShaderKind::FRAGMENT;
    mfly::shaders::AddShaders(raw_s, 2);
    uint16_t s_h[] = {0, 1};
    uint16_t d_h = 0;
    
    mfly::shaders::CreateGroup(s_h, 2, &d_h, 1);
    std::vector<mfly::shaders::ShaderByteCode> bytecodes;
    mfly::shaders::CompileGroups(bytecodes);
    
    mfly::vk::VkShaderBulk bulk;

    mfly::vk::VkShader_InitInfo shader1;
    shader1.bytecode = bytecodes[0].bytecode.data();
    shader1.code_size = bytecodes[0].bytecode.size() * sizeof(uint32_t);
    
    mfly::vk::VkShader_InitInfo shader2;
    shader2.bytecode = bytecodes[1].bytecode.data();
    shader2.code_size = bytecodes[1].bytecode.size() * sizeof(uint32_t);

    // A stages is for declaring as said, stages, VERTEX/FRAGMENT,...
    // So have to at which number of the vector they start and when to end in the vector
    mfly::vk::VkShaderStage_InitInfo stage1;
    stage1.start = -1 + (stage1.end = 1);
    stage1.logical_dvc = 0;
    stage1.stage = VK_SHADER_STAGE_VERTEX_BIT;
    bulk.shader_infos.push_back(shader1);
    bulk.declares.push_back(stage1);

    mfly::vk::VkShaderStage_InitInfo stage2;
    stage2.start = -1 + (stage2.end = 2);
    stage2.logical_dvc = 0;
    stage2.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    bulk.shader_infos.push_back(shader2);
    bulk.declares.push_back(stage2);
    
    shader_handles.resize(bulk.shader_infos.size());
    uint32_t* handles_p = mfly::vk::AddShaders(bulk);
    memcpy(shader_handles.data(), handles_p, sizeof(uint32_t)*bulk.shader_infos.size());
    delete handles_p;
    bool test_empty_str = false;
}

#include<vulkan/vulkan.hpp>
VkSemaphore img_available;
std::pair<uint32_t, VkSemaphore> render_finished;
VkFence frame_in_flight;

void TestCreateGraphicsPipeline() {
    //img_available = mfly::vk::AddSemaphore();
    render_finished = mfly::vk::AddSemaphore(UINT32_MAX);
    //frame_in_flight = mfly::vk::AddFence();

    // Add Subpass and attachment
    mfly::vk::VkAttachmentInfoWrap attachment_info;
    mfly::vk::AddAttachmentDesc(attachment_info);
    //mfly::vk::AddAttachmentDesc(attachment_info);
    //mfly::vk::AddAttachmentDesc(attachment_info);
    //mfly::vk::AddAttachmentDesc(attachment_info);
    

    mfly::vk::VkSubPassInfoWrap subpass_info;
    subpass_info.framebuffers.push_back(VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    //subpass_info.framebuffers.push_back(VkAttachmentReference{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    //subpass_info.framebuffers.push_back(VkAttachmentReference{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    //subpass_info.framebuffers.push_back(VkAttachmentReference{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    
    mfly::vk::AddSubPass(subpass_info);

    mfly::vk::VkRenderPassInfoWrap render_pass;
    render_pass.attachment_handles.push_back(0);
    //render_pass.attachment_handles.push_back(1);
    //render_pass.attachment_handles.push_back(2);
    //render_pass.attachment_handles.push_back(3);
    render_pass.subpass_handles.push_back(0);
    uint32_t rp_handle = mfly::vk::CreateRenderPass(render_pass);

    const mfly::vk::VkSwapchainWrap& swc_wrap = vkapp.swapchains[0];   

    mfly::vk::VkFramebufferInfoWrap fb_info;
    fb_info.extent.width = swc_wrap.area.width;
    fb_info.extent.height = swc_wrap.area.height;

    //fb_info.img_view_handles.push_back(0); // For now use the only image created
    fb_info.num_layers = 1;
    fb_info.render_pass_handle = rp_handle;
    
    mfly::vk::AddSWCFramebuffer(fb_info, 0 );
    mfly::vk::VkGraphicsPipeStateInfoWrap default_pipe;
    default_pipe.name = "main"; // Function name to execute, so fucking bad, make always main and that's it ffs
    default_pipe.render_pass_handle = 0;
    
    default_pipe.vtx_info.vtx_attribute_descriptor_handle = UINT32_MAX;
    default_pipe.vtx_info.vtx_binding_descriptor_handle = UINT32_MAX;

    VkViewport vp = {};
    vp.x = vp.y = vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vp.width = swc_wrap.area.width;
    vp.height = swc_wrap.area.height;
    // Will not render correctly but its to test
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = {swc_wrap.area.width,swc_wrap.area.height};

    default_pipe.scissors.push_back(scissor);
    default_pipe.viewports.push_back(vp);

    default_pipe.dyn_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    default_pipe.dyn_states.push_back(VK_DYNAMIC_STATE_SCISSOR);

    default_pipe.shaders.push_back(shader_handles[0]);
    default_pipe.shaders.push_back(shader_handles[1]);
    default_pipe.blend_info.attachments.push_back(mfly::vk::ColorBlendInfo());

    mfly::vk::CreateGraphicsPipeline(default_pipe);

    mfly::vk::VkBeginInfoWrap begin_info;
    mfly::vk::AddRecordBegin(begin_info);
    mfly::vk::VkBeginRenderPassInfoWrap begin_rp_info;
    begin_rp_info.clear_colors.push_back(VkClearValue{{{1.f,0.f,0.f,1.f}}});
    //begin_rp_info.clear_colors.push_back(VkClearValue{1.,0.,0.,1.});
    //begin_rp_info.clear_colors.push_back(VkClearValue{1.,0.,0.,1.});
    //begin_rp_info.clear_colors.push_back(VkClearValue{1.,0.,0.,1.});
    begin_rp_info.extent = {swc_wrap.area.width, swc_wrap.area.height};
    begin_rp_info.offset = {0,0};
    begin_rp_info.framebuffer_handle = 0;
    begin_rp_info.render_pass_handle = 0;
    mfly::vk::AddRenderPassBegin(begin_rp_info);

    mfly::vk::VkCmdPoolInfoWrap cmd_pool_info;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    mfly::vk::AddCmdPool(cmd_pool_info);

    mfly::vk::VkCmdBufInfoWrap cmd_buf_info;
    cmd_buf_info.pool_handle = 0;

    mfly::vk::AddCmdBuffers(cmd_buf_info);
}


#include <mfly_slotmap.h>

int main(int argc, const char** argv)
{
    mfly::slotmap<uint32_t> example;
    example;

    InitTimer();
    CalibrateTimer();

    enki_TS.Initialize();
    mfly::win::Init();

    mfly::vk::ProvideSurfaceFun(mfly::win::getGAPISurface);
    mfly::vk::DefaultInit();
    
    TestShaderLib();
    TestCreateGraphicsPipeline();
    mfly::win::RegisterResizeCallback([](uint32_t i, float w, float h){mfly::vk::TriggerResizeSwapchain(i, VkExtent2D{(uint32_t)w,(uint32_t)h});});

    printf("Press [Enter] to close...\n");
    while(!mfly::win::PreUpdate())
    {   

        // Actually draw
        VkDevice dvc = vkapp.logical_dvcs[0];
        mfly::vk::PreUpdate(); // Preupdate should handle frames in flight too
        
        // Between preupdate and update one should do the begin record end record
        
        // Update then does the submits

        VkCommandBuffer cmd_buf = mfly::vk::BeginRecord(0, 0);
        mfly::vk::BeginRenderPass(0, cmd_buf);
        mfly::vk::BindPipeline(0, cmd_buf);
        mfly::vk::SetDynState(cmd_buf);
        vkCmdDraw(cmd_buf, 3, 1, 0,0);
        vkCmdEndRenderPass(cmd_buf);
        if(vkEndCommandBuffer(cmd_buf) != VK_SUCCESS) printf("Failed to record cmdbuffer");
        const mfly::vk::VkSwapchainWrap& swc_wrap = vkapp.swapchains[0];

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore wait_semaphores[] = {swc_wrap.img_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buf;

        VkSemaphore signal_semaphores[] = {render_finished.second};
        submit_info.pSignalSemaphores = signal_semaphores;
        submit_info.signalSemaphoreCount = 1;

        VkQueue present_queue;
        vkGetDeviceQueue(dvc, 0,0, &present_queue);
        VkResult res = vkQueueSubmit( present_queue, 1, &submit_info, frame_in_flight);
        
        VkPresentInfoKHR presentation = {};
        presentation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentation.waitSemaphoreCount = 1;
        presentation.pWaitSemaphores = signal_semaphores;

        presentation.swapchainCount = 1;
        presentation.pSwapchains = &swc_wrap.swapchain;
        presentation.pImageIndices = &swc_wrap.curr_image;
        presentation.pResults = nullptr;

        vkQueuePresentKHR(present_queue, &presentation);
    }
    
    return 0;
}

