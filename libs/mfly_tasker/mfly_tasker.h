#ifndef _MFLY_TASKER_
#define _MFLY_TASKER_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <pcg/pcg_basic.h>

namespace mfly {

    class Task
    {
    public:
        virtual void run() = 0;

    public:
        uint32_t id;
        std::vector<uint32_t> dependencies;

    };

    namespace Tasker
    {
        bool Init();
        void Update();
        bool Close();

        void ScheduleTask(Task* task);

        uint32_t NumThreads();
        uint32_t NumTasks();
    };



};

#endif