#ifndef MFLY_WINDOW
#define MFLY_WINDOW

#include <stdint.h>
#include <CrossWindow/CrossWindow.h>
#include <vector>

namespace mfly
{
    namespace win
    {
        // Module Functions
        uint16_t Init();
        uint16_t Close();
        uint16_t PreUpdate();
        uint16_t DoUpdate();
        uint16_t PostUpdate();
        uint16_t AsyncDispatch();
        uint16_t AsyncGather();

        static uint16_t event_code_start;

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

        struct window
        {
            xwin::EventQueue xqueue;
            xwin::WindowDesc xdesc;
            xwin::Window xwindow;
            uint8_t id = 0;
        };

        //static std::vector<mfly::win::window> windows;
    };
};

void TestWindws();

#endif