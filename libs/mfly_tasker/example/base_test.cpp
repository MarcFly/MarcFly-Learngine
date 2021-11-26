#include <mfly_tasker.h>
#include <../sque_timer/sque_timer.h>
#include <Windows.h>

/*
    Principles for a task to be worth multithreading
    - Task Scheduling costs around 15-30 microseconds
    - Task costs a magnitude more including setup cost
    - Task is easily independent
    - Task must be done a lot of times
        - Or can be done once every some time and result is not critical
*/

#define LOOPS 10000

#define ITERS 100000

int waste_time()
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < ITERS; ++i)
        sum += 1;

    return sum;
}

class long_task : public mfly::Task
{
    void run() override {
        waste_time();
    }
};


void Test0_AvgWasteTimeCost()
{
    SQUE_Timer t1;
    double total_time = 0;
    for (uint32_t i = 0; i < LOOPS; ++i)
    {
        t1.Start();
        waste_time();
        t1.Stop();
        total_time += t1.ReadMicroSec();
    }
    printf("Base waste time cost: %f us\n", total_time / (double)LOOPS);
}

void Test1_ScheduleWasteTime()
{
    SQUE_Timer t1;

    t1.Start();
    waste_time();
    t1.Stop();
    double time = t1.ReadMicroSec();

    // Single Thread 10.000 100microsecond waits
    // t1.Start();
    // for (uint32_t i = 0; i < LOOPS; ++i)
    // {
    //     waste_time();
    // }
    // t1.Stop();
    // time = t1.ReadMicroSec();
    // printf("Single Thread: %f us\n", time);

    SQUE_Timer t2;
    t1.Start();
    mfly::TaskID deps_test;
    deps_test.current_thread = 23;
    mfly::TaskID* p_dep = &deps_test;
    for (uint32_t i = 0; i < LOOPS; ++i)
    {
        mfly::Tasker::ScheduleTask(new long_task(), 0, p_dep, 1);
    }
    while (mfly::Tasker::NumTasks() != 0) {}
    t1.Stop();
    time = t1.ReadMicroSec();
    printf("Bad Multi Thread: %f us\n", time);
}


void Test2_ScheduleCost()
{
    mfly::Tasker::ResetDepCheckTime();

    SQUE_Timer t1;
    double total_time = 0;
    uint32_t i = 0;
    for (i = 0; i < LOOPS; ++i)
    {
        mfly::Task* push = new long_task();
        t1.Start();
        mfly::Tasker::ScheduleTask(push);
        t1.Stop();
        total_time += t1.ReadMicroSec();
    }
    
    printf("Schedule Cost with Dependency and multiple queues: %f us\n", total_time / (double)LOOPS);

    while (mfly::Tasker::NumTasks() != 0) {};

    printf("Average cost from take to run task (dpendency check): %f us\n", mfly::Tasker::GetDepCheckTime() / double(LOOPS));
}

int main(int argc, char** argv)
{
    mfly::Tasker::Init();

    Test0_AvgWasteTimeCost();

    Test1_ScheduleWasteTime();
    mfly::Tasker::PrintTotalTasks();
    Test2_ScheduleCost();

    mfly::Tasker::PrintTotalTasks();
    

    mfly::Tasker::Close();

    return 0;
}