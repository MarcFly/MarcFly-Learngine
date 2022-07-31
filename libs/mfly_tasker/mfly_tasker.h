#ifndef _MFLY_TASKER_
#define _MFLY_TASKER_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include "pcg/pcg_basic.h"

namespace mfly {

    struct TaskID
    {
        uint32_t id = 0;
        int16_t priority = 0;
        uint32_t current_thread = -1;
        bool operator <(const TaskID& check) const { return this->id < check.id; }
        bool operator >(const TaskID& check) const { return this->id > check.id; }
        bool operator ==(const TaskID& check) const { return this->id == check.id; }
        size_t operator ()(const TaskID& check) const { return check.id; }
    };
};

template<>
class std::hash<mfly::TaskID>
{
public:
    size_t operator()(const mfly::TaskID& task) const { return task.id; }
};

namespace mfly {
    

    //bool OrderTaskID(const TaskID& t1, const TaskID& t2);

    class Task
    {
    public:
        virtual void run() = 0;
        //bool operator <(const Task& check) { return this->id.id < check.id.id; }
    public:
        int16_t priority = 0;
        std::mutex access_lock;
        std::vector<TaskID> dependencies;
    };

    class Thread
    {
    public:
        void ThreadLoop();

    public:
        std::mutex _schedule_mtx;
        std::condition_variable _schedule_event;
        bool is_running_task = false;
        TaskID running_task;
        std::thread thread;
        std::unordered_multimap<TaskID, Task*> tasks;
        std::priority_queue<TaskID> scheduled_tasks;

        uint32_t total_tasks = 0;
    };

    namespace Tasker
    {
        bool Init();
        void Update();
        bool Close();

        TaskID ScheduleTask(Task* task, uint16_t priority = 0, TaskID* dependencies = nullptr, uint32_t num_deps = 0);
        Task* RetrieveTask(const TaskID& task_key);
        
        //uint32_t NumThreads();
        uint32_t NumTasks();
        double GetDepCheckTime();
        void ResetDepCheckTime();
        void PrintTotalTasks();
        
    };



};



#endif