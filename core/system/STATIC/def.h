#ifndef SYSTEM_DEF_H
#define SYSTEM_DEF_H


#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"

/* 系统状态 */
typedef int system_status_t;
/* 系统浮点数 */
typedef double system_float_t;
/* 系统句柄 */
typedef void* system_handle_h;
/* 系统偏移数 */
typedef long long system_off_t;

/* 线程名最大长度 */
#define SYSTEM_THREAD_NAME_MAX      16
/* 文件路径缓冲区 */
#define SYSTEM_PATH_MAX             4096
/* 文件路径缓冲区限制 缓冲区大于时使用堆内存 */
#define SYSTEM_PATH_SAFE_MAX        4096
/* 环境变量缓冲区 */
#define SYSTEM_EVN_MAX              4096
/* 通用字符串安全缓冲区 环境变量 主机名 等 */
#define SYSTEM_STRING_SAFE_MAX      256

#if PLATFORM_WINDOWS
/* 时间基准差 */
#define SYSTEM_TIME_OFFSET     11644473600ULL
/* 时间基准差毫秒 */
#define SYSTEM_TIME_OFFSET_MS  (SYSTEM_TIME_OFFSET * 1000ULL)
/* 时间基准差微秒 */
#define SYSTEM_TIME_OFFSET_US  (SYSTEM_TIME_OFFSET * 1000000ULL)
#elif PLATFORM_POSIX
/* 时间基准差 */
#define SYSTEM_TIME_OFFSET     0
/* 时间基准差毫秒 */
#define SYSTEM_TIME_OFFSET_MS  0
/* 时间基准差微秒 */
#define SYSTEM_TIME_OFFSET_US  0
#endif

/* thread */
#if PLATFORM_WINDOWS
typedef int system_thread_main_ret;
#elif PLATFORM_POSIX
typedef void* system_thread_main_ret;
#endif
typedef void* (system_mutex_t)[COM_UNITS(64)];
typedef void* (system_cond_t)[COM_UNITS(48)];
typedef void* (system_rwlock_t)[COM_UNITS(56)];
typedef void* (system_once_init)[COM_UNITS(8)];


#endif