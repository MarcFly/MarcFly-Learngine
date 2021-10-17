#include "mfly_tasker.h"
using namespace mfly;

//#include <unordered_map>

//#include <functional>

#include <../sque_timer/sque_timer.h>
std::atomic<double> dep_check_time;

double mfly::Tasker::GetDepCheckTime()
{
    return dep_check_time;
}

void mfly::Tasker::ResetDepCheckTime()
{
    dep_check_time = 0;
}

// Data

static std::vector<mfly::Thread*> threads_c;
static std::vector<uint32_t> ordered_threads;
#include <map>
#include <set>
//static std::multimap<uint32_t, mfly::Thread*> ordered_threads;
//static std::set<
static bool exit_flag = false;

void mfly::Thread::ThreadLoop()
{
    while (true)
    {
        Task* t = NULL;
        {
            std::unique_lock<std::mutex> lock(_schedule_mtx);
            while (t == NULL && !exit_flag)
            {
                if (scheduled_tasks.empty())
                    _schedule_event.wait(lock);
                else
                {
                    SQUE_Timer t1;
                    t1.Start();
                    // Take most prioritary task assigned to the thread
                    std::unordered_map<TaskID, Task*>::iterator it = tasks.find(scheduled_tasks.top());
                    t = it->second;

                    // Ask other threads if they have its dependencies or are running them
                    static bool dep_found = false;
                    for (uint32_t i = 0; i < threads_c.size(); ++i)
                    {
                        for (uint32_t j = 0; j < t->dependencies.size(); ++j)
                            if (threads_c[i]->tasks.find(t->dependencies[j]) != threads_c[i]->tasks.end())
                            {
                                dep_found = true;
                                break;
                            }
                        if (dep_found) break;
                    }

                    scheduled_tasks.pop();

                    // If a dependency is waiting
                        // Lower priority by 1
                        // Reschedule it
                        // Set t to NULL
                    if (dep_found)
                    {
                        --t->priority;
                        TaskID tid;
                        tid.id = it->first.id;
                        tid.priority = it->first.priority-1;
                        scheduled_tasks.push(tid);
                        t = NULL;
                    }
                    // ElseIf no dependencies are waiting
                        // Set task as being ran
                        // Remove from the map and queue
                    else
                    {
                        is_running_task = true;
                        running_task = it->first;
                        tasks.erase(it);
                    }
                    t1.Stop();
                    dep_check_time = dep_check_time + t1.ReadMicroSec();
                }
            }
        }
        if (exit_flag) break;
        std::unique_lock<std::mutex> task_lock(t->access_lock);
        t->run();
        is_running_task = false;
    }
}

bool mfly::Tasker::Init()
{
    for (uint16_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        Thread* t = new Thread();
        t->thread = std::thread(&Thread::ThreadLoop, t);
        threads_c.push_back(t);
        ordered_threads.push_back(i);
    }

    return true;
}

void mfly::Tasker::Update()
{

}

bool mfly::Tasker::Close()
{
    bool ret = true;

    exit_flag = true;

    for (uint16_t i = 0; i < threads_c.size(); ++i)
    {
        threads_c[i]->_schedule_event.notify_all();
        threads_c[i]->thread.join();
        delete threads_c[i];
    }
    threads_c.clear();

    return ret;
}


TaskID mfly::Tasker::ScheduleTask(Task* task, uint16_t priority, TaskID* dependencies, uint32_t num_deps)
{
    TaskID key;
    bool task_queued = false;

    for (uint32_t j = 0; j < num_deps; ++j)
        task->dependencies.push_back(dependencies[j]);
    
    while (!task_queued)
    {
        for (uint16_t i = 0; i < threads_c.size(); ++i)
        {
            //ordered_threads.begin()->first = 1;
            mfly::Thread* t = threads_c[ordered_threads[i]];
            if (!t->_schedule_mtx.try_lock()) continue;

            key.id = pcg32_random();
            key.priority = priority;
            t->tasks.insert(std::pair<TaskID, Task*>(key, task));
            t->scheduled_tasks.push(key);
            t->_schedule_event.notify_one();
            task_queued = true;
            ++t->total_tasks;
            t->_schedule_mtx.unlock();

            uint32_t t_ind = ordered_threads[i];
            for (uint16_t j = i; j < ordered_threads.size() - 1; ++j)
                ordered_threads[j] = ordered_threads[j + 1];
            ordered_threads[ordered_threads.size() - 1] = t_ind;

            break;
        }
    }
    
    return key;
}

Task* mfly::Tasker::RetrieveTask(const TaskID& task_key)
{
    return NULL;
}

uint32_t mfly::Tasker::NumTasks()
{
    uint32_t ret = 0;
    for (uint16_t i = 0; i < threads_c.size(); ++i)
        ret += threads_c[i]->tasks.size();

    return ret;
}

void mfly::Tasker::PrintTotalTasks()
{
    for (uint16_t i = 0; i < threads_c.size(); ++i)
        printf("Thread %i tasks scheduled: %i\n", i, threads_c[i]->total_tasks);
}

//-------------------------------------------------------------

/*
static std::vector<std::thread*> threads;
static std::unordered_map<TaskID, Task*> tasks;
static std::mutex _running_mtx;
static std::unordered_map<TaskID, Task*> running_tasks;
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
        TaskID tid;
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
                    
                    // Active wait for each dependency
                    for(uint16_t i = 0; i < t->dependencies.size(); ++i)
                        // Bad
                        while(running_tasks.find(t->dependencies[i]) != running_tasks.end()){}

                    {
                        std::unique_lock<std::mutex> r_lock(_running_mtx);
                        tid = running_tasks.insert(std::pair<TaskID, Task*>(it->first, t)).first->first;
                    }

                    tasks.erase(it);
                    scheduled_tasks.pop();
                }
            }
        }

        if(exit_flag) break;

        // Lock access to task data
        std::unique_lock<std::mutex> task_lock(t->access_lock);
        
        t->run();
        {
            std::unique_lock<std::mutex> r_lock(_running_mtx);
            running_tasks.erase(running_tasks.find(tid));
        }
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

TaskID Tasker::ScheduleTask(Task* task, uint16_t priority, TaskID* dependencies, uint32_t num_dependencies)
{
    std::unique_lock<std::mutex> qLock(_schedule_mtx);
    TaskID key;
    key.id = pcg32_random();
    key.priority = priority;
    Task* t = tasks.insert(std::pair<TaskID, Task*>(key, task)).first->second;
    for(uint32_t i = 0; i < num_dependencies; ++i)
        t->dependencies.push_back(dependencies[i]);
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
*/
// Utilities