#ifndef _MFLY_TASKER_
#define _MFLY_TASKER_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <pcg/pcg_basic.h>

namespace mfly {

    struct TaskID
    {
        uint32_t id = 0;
        uint16_t priority = 0;
        bool operator <(const TaskID& check) const { return this->id < check.id; }
        bool operator >(const TaskID& check) const { return this->id > check.id; }
        bool operator ==(const TaskID& check) const { return this->id == check.id; }
    };
    
    class Task
    {
    public:
        virtual void run() = 0;
        //bool operator <(const Task& check) { return this->id.id < check.id.id; }
    public:
        std::vector<uint32_t> dependencies;
        std::mutex access_lock;
    };

    namespace Tasker
    {
        bool Init();
        void Update();
        bool Close();

        TaskID ScheduleTask(Task* task, uint16_t priority = 0);
        Task* RetrieveTask(const TaskID& task_key);
        
        uint32_t NumThreads();
        uint32_t NumTasks();
    };



};



#endif