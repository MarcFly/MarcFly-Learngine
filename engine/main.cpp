#include <stdio.h>
#include <enkiTS/src/TaskScheduler.h>
enki::TaskScheduler enki_TS;

#include <mfly_window/mfly_window.hpp>

#include <cstdio>
#include <iostream>

int main(int argc, const char** argv)
{
    enki_TS.Initialize();

    printf("Press [Enter] to close...");

    system("pause");
    return 0;
}

// CrossWindow Delegate

