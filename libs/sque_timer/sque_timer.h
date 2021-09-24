#ifndef _SQUE_TIMER_
#define _SQUE_TIMER_

#include <chrono>

typedef std::chrono::high_resolution_clock::duration high_res_clock;
#define TIME_NOW std::chrono::high_resolution_clock::now().time_since_epoch()
#define CAST_MILLI(now) (double)std::chrono::duration_cast<std::chrono::milliseconds>(now).count()
#define CAST_MICRO(now) (double)std::chrono::duration_cast<std::chrono::microseconds>(now).count()
#define CAST_NANOS(now) (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

class SQUE_Timer																												
{		
public:																																
	SQUE_Timer() : is_stopped(false), is_active(true)
    {
        high_res_clock now = std::chrono::high_resolution_clock::now().time_since_epoch();
        stop_at_ms = start_at_ms = CAST_MILLI(now);
        stop_at_us = start_at_us = CAST_MICRO(now);
        stop_at_ns = start_at_ns = CAST_NANOS(now);
    }																													
																																
	void Start()
    {
        is_active = true;
        is_stopped = false;
        high_res_clock now = TIME_NOW;
        stop_at_ms = start_at_ms = CAST_MILLI(now);
        stop_at_us = start_at_us = CAST_MICRO(now);
        stop_at_ns = start_at_ns = CAST_NANOS(now);
    }	

	void Stop()
    {
        high_res_clock now = TIME_NOW;
        stop_at_ms = CAST_MILLI(now);
        stop_at_us = CAST_MICRO(now);
        stop_at_ns = CAST_NANOS(now);
        is_stopped = true;
    }																												
	void Kill()
    {
        high_res_clock now = TIME_NOW;
        stop_at_ms = CAST_MILLI(now);
        stop_at_us = CAST_MICRO(now);
        stop_at_ns = CAST_NANOS(now);
        is_stopped = true;
    }

	bool IsStopped() const { return is_stopped; }

	bool IsActive() const { return is_active; }
    
    double ReadMilliSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_ms - start_at_ms;
        double now = CAST_MILLI(TIME_NOW);
        return now - start_at_ms;
    }

	double ReadMicroSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_us - start_at_us;
        double now = CAST_MICRO(TIME_NOW);
        return now - start_at_us;
    }

	double ReadNanoSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_ns - start_at_ns;
        double now = CAST_NANOS(TIME_NOW);
        return now - start_at_us;
    }
																																		
private:
	double start_at_ms;																													
	double start_at_ns;																													
	double start_at_us;																													
																																		
	double stop_at_ms;																													
	double stop_at_ns;																													
	double stop_at_us;																													
																																		
	bool is_stopped;																										
	bool is_active;																										
};

#endif`