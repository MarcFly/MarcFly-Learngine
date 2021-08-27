#include "mfly_window.hpp"
#include <enkiTS/src/TaskScheduler.h>

extern enki::TaskScheduler enki_TS;
namespace mfly { namespace win { std::vector<mfly::win::window> windows; }}

// Module Basics
uint16_t mfly::win::Init()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::win::Close()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::win::PreUpdate()
{
    uint16_t ret = 0;
    
    for (uint8_t i = 0; i < windows.size(); ++i)
    {
        mfly::win::window& win = windows[i];
        win.xqueue.update();
        while (!win.xqueue.empty())
        {
            // Take care of events
            // Do them
            // Generate parallel tasks,...
            win.xqueue.pop();
        }
    }

    return ret;
}

uint16_t mfly::win::DoUpdate()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::win::PostUpdate()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::win::AsyncDispatch()
{
    uint16_t ret = 0;

    return ret;
}

uint16_t mfly::win::AsyncGather()
{
    uint16_t ret = 0;

    return ret;
}

// Module Specifics

extern int main(int argc, const char** argv);

void xmain(int argc, const char** argv)
{
    printf("Done this");
    main(argc, argv);
}