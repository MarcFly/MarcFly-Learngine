#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

int main(int argc, const char** argv)
{
    enki_TS.Initialize();

    printf("Press [Enter] to close...");
    
    return std::getchar();
}

// CrossWindow Delegate

