#include "time.h"
#if COM_USE(CONFIG_SYSTEM_TIME)


#include "core/system/STATIC/base.h"
#include "core/system/thread.h"
#include "core/system/mmap.h"

/*  时间获取 Get time */

/*  获取挂钟时间秒 */
PORT_API system_float_t system_time_now_s(void)
{
#if PLATFORM_WINDOWS
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ui = { 0 };
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    return (double)ui.QuadPart / 10000000.0 - (double)SYSTEM_TIME_OFFSET;
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#else
    return 0;
#endif
}
/*  获取挂钟时间毫秒 */
PORT_API qty_t system_time_now_ms(void)
{
#if PLATFORM_WINDOWS
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ui = { 0 };
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    return (qty_t)(ui.QuadPart / 10000) - SYSTEM_TIME_OFFSET_MS;
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (qty_t)ts.tv_sec * 1000 + (qty_t)ts.tv_nsec / 1000000;
#else
    return 0;
#endif
}
/*  获取挂钟时间微秒 */
PORT_API qty_t system_time_now_us(void)
{
#if PLATFORM_WINDOWS
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ui = { 0 };
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    return (qty_t)(ui.QuadPart / 10) - SYSTEM_TIME_OFFSET_US;
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (qty_t)ts.tv_sec * 1000000 + (qty_t)ts.tv_nsec / 1000;
#else
    return 0;
#endif
}
/*  获取运行时间毫秒 */
PORT_API qty_t system_time_timing_ms(void)
{
#if PLATFORM_WINDOWS
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (qty_t)((counter.QuadPart * 1000) / freq.QuadPart);
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (qty_t)ts.tv_sec * 1000 + (qty_t)ts.tv_nsec / 1000000;
#else
    return 0;
#endif
}
/*  获取运行时间微秒 */
PORT_API qty_t system_time_timing_us(void)
{
#if PLATFORM_WINDOWS
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (qty_t)((counter.QuadPart * 1000000) / freq.QuadPart);
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
#else
    return 0;
#endif
}

/*  睡眠 Sleep */

/*  睡眠毫秒 */
PORT_API bool_t system_sleep_ms(qty_t ms)
{
#if PLATFORM_WINDOWS
    Sleep((DWORD)ms);
    return 1;
#elif PLATFORM_POSIX
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&req, NULL);
    return 1;
#else
    return 0;
#endif
}
/*  睡眠微秒 */
PORT_API bool_t system_sleep_us(qty_t us)
{
#if PLATFORM_WINDOWS
    DWORD ms = (DWORD)(us / 1000);
    if (us % 1000) ms++;
    Sleep(ms);
    return 1;
#elif PLATFORM_POSIX
    struct timespec req;
    req.tv_sec = us / 1000000;
    req.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&req, NULL);
    return 1;
#else
    return 0;
#endif
}


/*  定时器 Timer */

/*  定时器线程 */
PORT_HIDE static system_thread_main_ret PLATFORM_CALL system_timer_thread_func(void* arg)
{
    /*  初始化 */
    struct system_timer_t* t = (struct system_timer_t*)arg;
    if (system_mutex_lock(&t->lock) == 0) return (system_thread_main_ret)0;
    if (t->undel && t->running == 0)
        system_cond_wait(&t->cond, &t->lock);
    t->timing_ms = system_time_timing_ms();
    while (t->running && t->undel)
    {
        system_mutex_unlock(&t->lock);
        /*  执行回调 */
        t->callback(t->arg);
        /*  计算睡眠 */
        qty_t time_tmp = system_time_timing_ms() - t->timing_ms;
        if (time_tmp <= t->interval_ms && t->interval_ms - time_tmp > t->busy_threshold_ms)
        {
            if (system_mutex_lock(&t->lock) == 0) return (system_thread_main_ret)0;
            qty_t wait_ms = t->interval_ms - time_tmp;
            system_cond_timedwait(&t->cond, &t->lock, wait_ms);
            t->timing_ms += t->interval_ms;
            qty_t tmp = system_time_timing_ms();
            if (t->timing_ms <= tmp)
                t->timing_ms = tmp;
        }
        else
        {
            if (system_mutex_lock(&t->lock) == 0) return (system_thread_main_ret)0;
            t->timing_ms += t->interval_ms;
            qty_t tmp = system_time_timing_ms();
            if (t->timing_ms <= tmp)
                t->timing_ms = tmp;
        }
        /*  计算循环 */
        if (t->count != 0)
        {
            t->count--;
            if (t->count == 0)
                t->running = 0;
        }
    }
    system_mutex_unlock(&t->lock);
    return (system_thread_main_ret)0;
}
/*  创建定时器 */
PORT_API struct system_timer_t* system_timer_new(qty_t interval_ms, qty_t busy_threshold_ms, qty_t count, void (PLATFORM_CALL *callback)(void*), void* arg)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (callback == 0) return 0;
    /*  初始化定时器 */
    struct system_mmap_prot_t prot = { 0 };
    struct system_mmap_flags_t flags = { 0 };
    prot.read = 1;
    prot.write = 1;
    struct system_timer_t* t = system_mmap_new(0, sizeof(struct system_timer_t), prot, flags);
    if (t == 0) return 0;
    t->interval_ms = interval_ms;
    t->busy_threshold_ms = busy_threshold_ms;
    t->count = count;
    t->callback = callback;
    t->arg = arg;
    t->running = 0;
    t->undel = 1;
    /*  初始化锁和条件变量 */
    if (system_mutex_init(&t->lock) == 0)
    {
        system_mmap_del(t,sizeof(struct system_timer_t));
        return 0;
    }
    if (system_cond_init(&t->cond) == 0)
    {
        system_mutex_del(&t->lock);
        system_mmap_del(t, sizeof(struct system_timer_t));
        return 0;
    }
    /*  创建线程 */
    t->thread = system_thread_new((system_thread_main)system_timer_thread_func, t);
    if (t->thread == 0)
    {
        system_cond_del(&t->cond);
        system_mutex_del(&t->lock);
        system_mmap_del(t, sizeof(struct system_timer_t));
        return 0;
    }
    return t;
#else
    return 0;
#endif
}
/*  删除定时器 */
PORT_API bool_t system_timer_del(struct system_timer_t* timer)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (timer == 0) return 0;
    if (timer->undel == 0) return 1;
    /*  标记销毁并唤醒线程 */
    if (system_mutex_lock(&timer->lock) == 0) return 0;
    timer->undel = 0;
    timer->running = 0;
    system_mutex_unlock(&timer->lock);
    system_cond_wake_all(&timer->cond);
    /*  等待线程结束 */
    system_thread_join(timer->thread, (void*)0);
    /*  释放资源 */
    system_cond_del(&timer->cond);
    system_mutex_del(&timer->lock);
    system_mmap_del(timer, sizeof(struct system_timer_t));
    return 1;
#else
    return 0;
#endif
}
/*  运行定时器 */
PORT_API bool_t system_timer_run(struct system_timer_t* timer)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (timer == 0 || timer->undel == 0) return 0;
    if (system_mutex_lock(&timer->lock) == 0) return 0;
    if (timer->running)
    {
        system_mutex_unlock(&timer->lock);
        return 1;
    }
    timer->running = 1;
    system_cond_wake_all(&timer->cond);
    system_mutex_unlock(&timer->lock);
    return 1;
#else
    return 0;
#endif
}
/*  停止定时器 */
PORT_API bool_t system_timer_stop(struct system_timer_t* timer)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (timer == 0 || timer->undel == 0) return 0;
    if (system_mutex_lock(&timer->lock) == 0) return 0;
    if (timer->running == 0)
    {
        system_mutex_unlock(&timer->lock);
        return 1;
    }
    timer->running = 0;
    system_cond_wake_all(&timer->cond);
    system_mutex_unlock(&timer->lock);
    return 1;
#else
    return 0;
#endif
}
/*  重新设置睡眠和阈值时间 */
PORT_API bool_t system_timer_set_reconfig(struct system_timer_t* timer, qty_t interval_ms, qty_t new_threshold_ms)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (timer == 0) return 0;
    if (system_mutex_lock(&timer->lock) == 0) return 0;
    timer->interval_ms = interval_ms;
    timer->busy_threshold_ms = new_threshold_ms;
    system_mutex_unlock(&timer->lock);
    return 1;
#else
    return 0;
#endif
}
/*  获取定时器是否运行 */
PORT_API bool_t system_timer_get_running(struct system_timer_t* timer)
{
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (timer == 0) return 0;
    bool_t tmp = 0;
    if (system_mutex_lock(&timer->lock) == 0) return 0;
    tmp = timer->running && timer->undel;
    system_mutex_unlock(&timer->lock);
    return tmp;
#else
    return 0;
#endif
}


#else


PORT_API system_float_t system_time_now_s(void) { ; }
PORT_API qty_t system_time_now_ms(void) { ; }
PORT_API qty_t system_time_now_us(void) { ; }
PORT_API qty_t system_time_timing_ms(void) { ; }
PORT_API qty_t system_time_timing_us(void) { ; }
PORT_API bool_t system_sleep_ms(qty_t ms) { ; }
PORT_API bool_t system_sleep_us(qty_t us) { ; }
PORT_API struct system_timer_t* system_timer_new(qty_t interval_ms, qty_t busy_threshold_ms, qty_t count, void (PLATFORM_CALL* callback)(void*), void* arg) { ; }
PORT_API bool_t system_timer_del(struct system_timer_t* timer) { ; }
PORT_API bool_t system_timer_run(struct system_timer_t* timer) { ; }
PORT_API bool_t system_timer_stop(struct system_timer_t* timer) { ; }
PORT_API bool_t system_timer_set_reconfig(struct system_timer_t* timer, qty_t interval_ms, qty_t new_threshold_ms) { ; }
PORT_API bool_t system_timer_get_running(struct system_timer_t* timer) { ; }


#endif