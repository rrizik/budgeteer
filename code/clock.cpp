#ifndef CLOCK_C
#define CLOCK_C

static u64
clock_get_os_timer_frequency(void){
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return((u64)frequency.QuadPart);
}

static u64
clock_get_os_timer(){
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return((u64)result.QuadPart);
}

static u64
clock_get_cpu_timer(){
    return CLOCK_TIMER;
}

static f64
clock_get_seconds_elapsed(u64 end, u64 start){
    f64 result;
    result = ((f64)(end - start) / ((f64)t_clock.frequency));
    return(result);
}

static f64
clock_get_ms_elapsed(u64 end, u64 start){
    f64 result;
    result = (1000 * ((f64)(end - start) / ((f64)t_clock.frequency)));
    return(result);
}

static void
clock_init(Clock* t_clock){
    t_clock->frequency = clock_get_os_timer_frequency();

    t_clock->get_os_timer = clock_get_os_timer;
    t_clock->get_seconds_elapsed = clock_get_seconds_elapsed;
    t_clock->get_ms_elapsed = clock_get_ms_elapsed;
    t_clock->get_cpu_timer = clock_get_cpu_timer;
}

#endif
