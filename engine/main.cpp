#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

#include <cstdio>
#include <iostream>

#include <mfly_window/mfly_window.hpp>
#include <mfly_gpu/mfly_gpu.hpp>
#include <mfly_shaders/mfly_shaders.hpp>
#include <sque_timer.h>
int main(int argc, const char** argv)
{
    InitTimer();
    CalibrateTimer();

    enki_TS.Initialize();
    mfly::win::Init();

    mfly::gpu::ProvideSurfaceFun(mfly::win::getGAPISurface);
    mfly::gpu::DefaultInit();
    
    mfly::shaders::AddDefines("MY_DEFINE", "1");

    const char* shaders[] = {
        "#version 310 es\nvoid main() {int x = MY_DEFINE;}",
        "#version 310 es\nint main() {int x = MY_DEFINE;}"
    };
    mfly::shaders::Shader raw_s[2];
    raw_s[0].code = "#version 310 es\nvoid main() {int x = MY_DEFINE;}";
    raw_s[0].type = mfly::shaders::ShaderKind::VERTEX;
    raw_s[1].code = "#version 310 es\nint main() {int x = MY_DEFINE;}";
    raw_s[1].type = mfly::shaders::ShaderKind::VERTEX;
    mfly::shaders::AddShaders(raw_s, 2);
    uint16_t s_h[] = {0, 1};
    uint16_t d_h = 0;



    mfly::shaders::CreateGroup(s_h, 2, &d_h, 1);
    mfly::shaders::CompileGroups();
    std::vector<mfly::shaders::ShaderByteCode> bytecodes(2);
    mfly::shaders::ReturnBytecode(bytecodes);

    mfly::gpu::VkShaderBulk bulk;
    mfly::gpu::VkShaderInfoWrap shader1;
    shader1.bytecode = bytecodes[0].bytecode.data();
    shader1.code_size = bytecodes[0].bytecode.size() * sizeof(uint32_t);
    mfly::gpu::VkShaderStageInfoWrap stage1;
    stage1.start = -1 + (stage1.end = 1);
    stage1.logical_dvc = 0;
    stage1.stage = VK_SHADER_STAGE_VERTEX_BIT;
    bulk.shader_infos.push_back(shader1);
    bulk.declares.push_back(stage1);

    mfly::gpu::AddShaders(bulk);

    printf("Press [Enter] to close...\n");
    while(!mfly::win::PreUpdate())
    {   

    }
    
    return 0;
}

// CrossWindow Delegate

