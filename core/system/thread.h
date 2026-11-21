#ifndef SYSTEM_THREAD_H
/*  ----线程----
*
* system_thread_set_priority
* win映射为7个优先级 linux需要root权限
*
* system_thread_set_name
* 限制为16字符(假设仅用ascll,包括\0)
*
* 使用 #pragma 去除警告
*
----线程---- */
#define SYSTEM_THREAD_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_thread_t;
typedef system_thread_main_ret system_thread_main_ret;
typedef system_thread_main_ret(PLATFORM_CALL* system_thread_main)(void*);
typedef system_mutex_t system_mutex_t;
typedef system_cond_t system_cond_t;
typedef system_rwlock_t system_rwlock_t;
typedef qty_t system_tls_key;
typedef system_once_init system_once_init;

/* 线程生命周期 Thread */

/* 创建线程 */
PORT_API system_thread_t system_thread_new(system_thread_main main, void* arg);
/* 等待线程结束 */
PORT_API bool_t system_thread_join(system_thread_t thread, void** retval);
/* 分离线程 */
PORT_API bool_t system_thread_detach(system_thread_t thread);
/* 获取当前线程 ID */
PORT_API system_thread_t system_thread_self(void);
/* 线程退出 */
PORT_API void system_thread_exit(void* retval);
/* 让出 CPU 让出当前时间片的剩余部分 */
PORT_API bool_t system_thread_yield(void);
/* 
 * 设置调试名称
 * Windows 7/8 及 Windows 10 1607 之前不支持 返回0
 * Windows 10 1607+ 正常支持
 */
PORT_API bool_t system_thread_set_name(system_thread_t thread, const char* name);
/* 设置优先级 */
PORT_API bool_t system_thread_set_priority(system_thread_t thread, qty_t priority_pertage);

/* 互斥锁 Mutex */

/* 初始化 */
PORT_API bool_t system_mutex_init(system_mutex_t* mutex);
/* 加锁 */
PORT_API bool_t system_mutex_lock(system_mutex_t* mutex);
/* 尝试加锁 */
PORT_API bool_t system_mutex_trylock(system_mutex_t* mutex);
/* 解锁 */
PORT_API bool_t system_mutex_unlock(system_mutex_t* mutex);
/* 销毁 */
PORT_API bool_t system_mutex_del(system_mutex_t* mutex);

/* 条件变量 Condition Variable */

/* 初始化 */
PORT_API bool_t system_cond_init(system_cond_t* cond);
/* 等待 等待期间释放锁 结束后重新加锁 */
PORT_API bool_t system_cond_wait(system_cond_t* cond, system_mutex_t* mutex);
/* 定时等待 等待期间释放锁 结束后重新加锁 */
PORT_API bool_t system_cond_timedwait(system_cond_t* cond, system_mutex_t* mutex, qty_t ms);
/* 唤醒一个 */
PORT_API bool_t system_cond_wake_one(system_cond_t* cond);
/* 唤醒全部 */
PORT_API bool_t system_cond_wake_all(system_cond_t* cond);
/* 销毁 */
PORT_API bool_t system_cond_del(system_cond_t* cond);

/* 读写锁 RWLock */

/* 初始化 */
PORT_API bool_t system_rwlock_init(system_rwlock_t* rw);
/* 读加锁 */
PORT_API bool_t system_rwlock_rdlock(system_rwlock_t* rw);
/* 写加锁 */
PORT_API bool_t system_rwlock_wrlock(system_rwlock_t* rw);
/* 解锁读锁 */
PORT_API bool_t system_rwlock_unlock_rd(system_rwlock_t* rw);
/* 解锁写锁 */
PORT_API bool_t system_rwlock_unlock_wr(system_rwlock_t* rw);
/* 销毁 */
PORT_API bool_t system_rwlock_del(system_rwlock_t * rw);

/* 线程局部存储 TLS */

/* 初始化 键地址 键的值释放地址 */
PORT_API bool_t system_tls_key_init(system_tls_key* key, void (PLATFORM_CALL *destructor)(void*));
/* 删除键 */
PORT_API bool_t system_tls_key_del(system_tls_key key);
/* 设置值 */
PORT_API bool_t system_tls_key_set(system_tls_key key, void* value);
/* 获取值 */
PORT_API void* system_tls_key_get(system_tls_key key);

/* 单次初始化 Once */

/* 安全初始化 */
PORT_API bool_t system_once_run(system_once_init* once, void (*fn)(void));


EXTERN_C_END
#endif