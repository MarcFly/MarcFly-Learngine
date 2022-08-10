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

#include "tscns.h"
static TSCNS tscns;
inline void InitTimer() {
    tscns.init();
}
inline void CalibrateTimer() {
    tscns.calibrate();
}

class TSCNS_Timer {
public:																																
	TSCNS_Timer() : is_stopped(false), is_active(true)
    {
        double now = tscns.rdns();
        stop_at_ms = start_at_ms = now/100000.;
        stop_at_us = start_at_us = now/1000.;
        stop_at_ns = start_at_ns = now;
    }																													
																																
	void Start()
    {
        is_active = true;
        is_stopped = false;
        double now = tscns.rdns();
        stop_at_ms = start_at_ms = now/1000000.;
        stop_at_us = start_at_us = now/1000.;
        stop_at_ns = start_at_ns = now;
    }	

	void Stop()
    {
        double now = tscns.rdns();
        stop_at_ms = now/1000000.;
        stop_at_us = now/1000.;
        stop_at_ns = now;
        is_stopped = true;
    }																												
	void Kill()
    {
        double now = tscns.rdns();
        stop_at_ms = now/1000000.;
        stop_at_us = now/1000.;
        stop_at_ns = now;
        is_stopped = true;
    }

	bool IsStopped() const { return is_stopped; }

	bool IsActive() const { return is_active; }
    
    double ReadMilliSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_ms - start_at_ms;
        double now = tscns.rdns()/1000000.;
        return now - start_at_ms;
        // 1660033411191.5017
        // 1660033382558.7542
    }

	double ReadMicroSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_us - start_at_us;
        double now = tscns.rdns()/1000.;
        return now - start_at_us;
    }

	double ReadNanoSec() const
    {
        if (is_stopped || !is_active)
            return stop_at_ns - start_at_ns;
        double now = tscns.rdns();
        return (now - start_at_us);
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