#include "mfly_window.hpp"
#include <enkiTS/src/TaskScheduler.h>
#include <CrossWindow/Graphics.h>

extern enki::TaskScheduler enki_TS;
namespace mfly { 
    namespace win { 
        std::vector<mfly::win::window> windows; 
        std::vector<ResizeCallback> resize_callbacks;
    }
}

// Module Basics
uint16_t mfly::win::Init()
{
    uint16_t ret = 0;
    windows.push_back(window());
    window& main = windows[windows.size()-1];

    main.id = windows.size()-1;
    main.xdesc.title = "Window Title";
    main.xdesc.name = "Test Window";
    main.xdesc.closable = false;
    main.xdesc.visible = true;
    main.xdesc.maximizable = true;
    main.xdesc.movable = true;
    main.xdesc.resizable = true;
    main.xdesc.centered = true;

    
    main.xwindow.create(main.xdesc, main.xqueue);
    

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
            const xwin::Event& event = win.xqueue.front();
            // Take care of events
            // Do them
            // Generate parallel tasks,...
            switch(event.type) {
                case xwin::EventType::Close:
                    win.xwindow.close();
                    ret = 1;
                    break;
                case xwin::EventType::Resize:
                    xwin::UVec2 size_win = win.xwindow.getWindowSize();
                    
                    for(auto cb : resize_callbacks) {
                        cb(i, size_win.x, size_win.y);
                    }

            }

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

#include <direct.h>
#include <Windows.h>
#include <CrossWindow/CrossWindow.h>
#include <CrossWindow/Graphics.h>

static std::vector<vk::SurfaceKHR> surfaces(10);

void* mfly::win::getGAPISurface(void* gapi_instance, uint32_t window_handle)
{  
    xwin::Window* win = &mfly::win::windows[window_handle].xwindow;
    vk::Instance fin_inst = (vk::Instance)(VkInstance)gapi_instance;

    return (void*)(VkSurfaceKHR)xgfx::getSurface(win, fin_inst);
}

// TODO: Callback vectors + register funs?

void mfly::win::RegisterResizeCallback(ResizeCallback resize_fun) { resize_callbacks.push_back(resize_fun); }