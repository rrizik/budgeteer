#ifndef CLOCK_H
#define CLOCK_H

typedef u64 GetTicks(void);
typedef f64 GetSecondsElapsed(u64 start, u64 end);
typedef f64 GetMsElapsed(u64 start, u64 end);
typedef u64 GetCycles(void);

struct Clock{
    f64 dt;
    u64 frequency; // NOTE: How many ticks there are per second
    GetTicks* get_os_timer;
    GetSecondsElapsed* get_seconds_elapsed;
    GetMsElapsed* get_ms_elapsed;
    GetCycles* get_cpu_timer;
};
static Clock clock = {0};

static u64 get_os_timer_frequency(void);
static u64 get_os_timer();
static u64 get_cpu_timer();
static f64 get_seconds_elapsed(u64 end, u64 start);
static f64 get_ms_elapsed(u64 end, u64 start);
static void init_clock(Clock* clock);

#endif
