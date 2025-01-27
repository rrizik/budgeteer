#ifndef CLOCK_C
#define CLOCK_C

#ifndef CLOCK_TIMER
#define CLOCK_TIMER __rdtsc()
#endif

static u64
get_os_timer_frequency(void){
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return((u64)frequency.QuadPart);
}

static u64
get_os_timer(){
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return((u64)result.QuadPart);
}

static u64
get_cpu_timer(){
    return CLOCK_TIMER;
}

static f64
get_seconds_elapsed(u64 end, u64 start){
    f64 result;
    result = ((f64)(end - start) / ((f64)t_clock.frequency));
    return(result);
}

static f64
get_ms_elapsed(u64 end, u64 start){
    f64 result;
    result = (1000 * ((f64)(end - start) / ((f64)t_clock.frequency)));
    return(result);
}

static void init_clock(Clock* t_clock){
    t_clock->frequency = get_os_timer_frequency();

    t_clock->get_os_timer = get_os_timer;
    t_clock->get_seconds_elapsed = get_seconds_elapsed;
    t_clock->get_ms_elapsed = get_ms_elapsed;
    t_clock->get_cpu_timer = get_cpu_timer;
}

#endif
