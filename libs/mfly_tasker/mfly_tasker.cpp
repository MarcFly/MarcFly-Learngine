#include "mfly_tasker.h"
using namespace mfly;

static std::vector<std::thread*> threads;
static std::queue<Task*> scheduled_tasks;
static std::mutex _schedule_mtx;
static std::condition_variable _schedule_event;

static bool exit_flag;

uint32_t Tasker::NumThreads()
{
    return threads.size();
}

uint32_t Tasker::NumTasks() { 
    return scheduled_tasks.size(); 
}

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
                    t = scheduled_tasks.front();
                    scheduled_tasks.pop();
                }
            }
        }

        if(exit_flag) break;

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

void Tasker::ScheduleTask(Task* task)
{
    std::unique_lock<std::mutex> qLock(_schedule_mtx);
    scheduled_tasks.push(task);
    _schedule_event.notify_one();
}