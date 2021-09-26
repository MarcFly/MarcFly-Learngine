#include "mfly_tasker.h"
using namespace mfly;

#include <unordered_map>

#include <functional>

template<>
class std::hash<TaskID>
{
    public:
    size_t operator()(const TaskID& task) const { return task.id; }
};

// Data

static std::vector<std::thread*> threads;
static std::unordered_map<TaskID, Task*> tasks;
static std::priority_queue<TaskID> scheduled_tasks;
static std::mutex _schedule_mtx;
static std::condition_variable _schedule_event;

static bool exit_flag;

// Data Getters

uint32_t Tasker::NumThreads()
{
    return threads.size();
}

uint32_t Tasker::NumTasks() { 
    return scheduled_tasks.size(); 
}

// Base Funs

void ThreadLoop()
{
    while(true)
    {
        Task* t = 0;
        {
            std::unique_lock<std::mutex> lock(_schedule_mtx);
            while(t == NULL && !exit_flag)
            {
                if(scheduled_tasks.empty())
                    _schedule_event.wait(lock);
                else
                {
                    auto it = tasks.find(scheduled_tasks.top());
                    t = it->second;
                    tasks.erase(it);
                    scheduled_tasks.pop();
                }
            }
        }

        if(exit_flag) break;

        // Lock access to task data
        std::unique_lock<std::mutex> task_lock(t->access_lock);
        t->run();
    }
}

bool Tasker::Init()
{
    for(uint16_t i = 0; i < std::thread::hardware_concurrency(); ++i)
        threads.push_back(new std::thread(ThreadLoop));

    return true;
}

void Tasker::Update()
{
    // No callback tasks within tasker
}

bool Tasker::Close()
{
    bool ret = true;
    
    exit_flag = true;
    _schedule_event.notify_all();
    for (uint16_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }

    threads.clear();

    return ret;
}

TaskID Tasker::ScheduleTask(Task* task, uint16_t priority)
{
    std::unique_lock<std::mutex> qLock(_schedule_mtx);
    TaskID key;
    key.id = pcg32_random();
    key.priority = priority;
    tasks.insert(std::pair<TaskID, Task*>(key, task)).second;
    scheduled_tasks.push(key);
    _schedule_event.notify_one();
    return key;
}

Task* Tasker::RetrieveTask(const TaskID& task_key)
{
    Task* ret = NULL;

    auto it = tasks.find(task_key);
    if(it == tasks.end()) return NULL;
    // Should it be unscheduled?
    // It should be at least locked...
    if(!it->second->access_lock.try_lock())
        return NULL;
        
    return it->second;
}

// Utilities