#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

#include <cstdio>
#include <iostream>

#include <mfly_window/mfly_window.hpp>
#include <mfly_gpu/mfly_gpu.h>

int main(int argc, const char** argv)
{
    enki_TS.Initialize();
    mfly::win::Init();

    printf("Press [Enter] to close...");
    while(!mfly::win::PreUpdate())
    {
        
    }
    
    return 0;
}

// CrossWindow Delegate

