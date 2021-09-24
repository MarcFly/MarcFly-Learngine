#include <mfly_tasker.h>
#include <../sque_timer/sque_timer.h>
#include <Windows.h>

// Test to See when is a task worth multithreading
// Ryzen 9 5900x - 24 logical threads
// - 1000 iters of waste time ~ .473 microseconds
// It is worth multithreading when the total cost of repetition of a task
// is > 4 microseconds, as the task scheduling has its cost as the work setup
// This just takes into account repetitive step cost

void waste_time()
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < 1000; ++i)
        sum += 1;
}

class long_task : public mfly::Task
{
    void run() override {
        waste_time();
    }
};



int main(int argc, char** argv)
{
    mfly::Tasker::Init();


    SQUE_Timer t1;

    t1.Start();
    waste_time();
    t1.Stop();
    double time = t1.ReadMicroSec();

    // Single Thread 10.000 100microsecond waits
    t1.Start();
    for (uint32_t i = 0; i < 100; ++i)
    {
        waste_time();
    }
    t1.Stop();
    printf("Single Thread: %f\n", t1.ReadMicroSec());

    t1.Start();
    for (uint32_t i = 0; i < 100; ++i)
    {
        mfly::Tasker::ScheduleTask(new long_task());
    }
    while (mfly::Tasker::NumTasks() != 0) {}
    t1.Stop();
    printf("Bad Multi Thread: %f\n", t1.ReadMicroSec());

    mfly::Tasker::Close();

    return 0;
}