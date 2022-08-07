#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

#include <cstdio>
#include <iostream>

#include <mfly_window/mfly_window.hpp>
#include <mfly_gpu/mfly_gpu.hpp>

int main(int argc, const char** argv)
{
    enki_TS.Initialize();
    mfly::win::Init();

    mfly::gpu::ProvideSurfaceFun(mfly::win::getGAPISurface);
    mfly::gpu::DefaultInit();
    
    printf("Press [Enter] to close...\n");
    while(!mfly::win::PreUpdate())
    {
        
    }
    
    return 0;
}

// CrossWindow Delegate

