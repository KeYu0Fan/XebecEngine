#ifndef SYSTEM_TIME_H
/* ----时间----
*
* system_timer_t
* 请不要在回调中调用 system_timer_del 会造成死锁
*
----时间---- */
#define SYSTEM_TIME_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
#include "core/system/thread.h"
EXTERN_C_BEGIN


struct system_timer_t
{
    /*  保护内部状态 */
    system_mutex_t lock;
    /*  条件变量 用于等待和唤醒 */
    system_cond_t cond;
    /*  定时器线程 */
    system_thread_t thread;
    /*  1=正在运行 0=已停止/结束 */
    bool_t running;
    /*  1=有效 0=已标记销毁 */
    bool_t undel;
    /*  触发间隔 */
    qty_t interval_ms;
    /*  忙等阈值<=interval_ms */
    qty_t busy_threshold_ms;
    /*  上次的触发时间 决定这次的睡眠时间 */
    qty_t timing_ms;
    /*  剩余触发次数 0=无限 >0=剩余次数 */
    qty_t count;
    /*  回调程序 */
    void (*callback)(void*);
    /*  回调参数 */
    void* arg;
};

/*  时间获取 Get time */

/*  获取挂钟时间秒 */
PORT_API system_float_t system_time_now_s(void);
/*  获取挂钟时间毫秒 */
PORT_API qty_t system_time_now_ms(void);
/*  获取挂钟时间微秒 */
PORT_API qty_t system_time_now_us(void);
/*  获取运行时间毫秒 */
PORT_API qty_t system_time_timing_ms(void);
/*  获取运行时间微秒 */
PORT_API qty_t system_time_timing_us(void);

/*  睡眠 Sleep */

/*  睡眠毫秒 */
PORT_API bool_t system_sleep_ms(qty_t ms);
/*  睡眠微秒 */
PORT_API bool_t system_sleep_us(qty_t us);

/*  定时器 Timer */

/*  创建定时器 */
PORT_API struct system_timer_t* system_timer_new(qty_t interval_ms, qty_t busy_threshold_ms, qty_t count, void (PLATFORM_CALL *callback)(void*), void* arg);
/*  删除定时器 */
PORT_API bool_t system_timer_del(struct system_timer_t* timer);
/*  运行定时器 */
PORT_API bool_t system_timer_run(struct system_timer_t* timer);
/*  停止定时器 */
PORT_API bool_t system_timer_stop(struct system_timer_t* timer);
/*  重新设置睡眠和阈值时间 */
PORT_API bool_t system_timer_set_reconfig(struct system_timer_t* timer, qty_t interval_ms, qty_t new_threshold_ms);
/*  获取定时器是否运行 */
PORT_API bool_t system_timer_get_running(struct system_timer_t* timer);


EXTERN_C_END
#endif