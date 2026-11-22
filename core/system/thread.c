#include "thread.h"
#if COM_USE(CONFIG_SYSTEM_THREAD)


#include "core/system/STATIC/base.h"
#include "core/system/STATIC/string.h"

#if PLATFORM_WINDOWS
#if PLATFORM_MSVC
#pragma warning(disable: 26110)
#endif
#elif PLATFORM_POSIX
/**/
#else
/**/
#endif

/* 线程生命周期 Thread */

/* 创建线程 */
PORT_API system_thread_t system_thread_new(system_thread_main main, void* arg)
{
    if (main == 0) return 0;
#if PLATFORM_WINDOWS
    HANDLE tid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, arg, 0, NULL);
     return tid ? tid : 0;
#elif PLATFORM_POSIX
    pthread_t tid = 0;
    return (pthread_create(&tid, NULL, (void* (*)(void*))main, arg) == 0) ? (system_thread_t)tid : 0;
#else
    return 0;
#endif
}
/* 等待线程结束 */
PORT_API bool_t system_thread_join(system_thread_t thread, void** retval)
{
    if (thread == 0) return 0;
#if PLATFORM_WINDOWS
    DWORD exit_code = 0;
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
    {
        CloseHandle(thread);
        return 0;
    }
    if (GetExitCodeThread(thread, &exit_code) == 0)
    {
        CloseHandle(thread);
        return 0;
    }
    if (retval)
        *retval = (void*)(uintptr_t)exit_code;
    return CloseHandle(thread) ? 1 : 0;
#elif PLATFORM_POSIX
    return (pthread_join((pthread_t)thread, retval) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 分离线程 */
PORT_API bool_t system_thread_detach(system_thread_t thread)
{
    if (thread == 0) return 0;
#if PLATFORM_WINDOWS
    return CloseHandle(thread) ? 1 : 0;
#elif PLATFORM_POSIX
    return (pthread_detach((pthread_t)thread) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 获取当前线程 ID */
PORT_API system_thread_t system_thread_self(void)
{
#if PLATFORM_WINDOWS
    HANDLE current = GetCurrentThread();
    HANDLE dup = INVALID_HANDLE_VALUE;
    DuplicateHandle(
        GetCurrentProcess(), current, GetCurrentProcess(), &dup,
        SYNCHRONIZE | THREAD_QUERY_INFORMATION, FALSE, DUPLICATE_SAME_ACCESS);
    return (dup == INVALID_HANDLE_VALUE) ? 0 : dup;
#elif PLATFORM_POSIX
    return (system_thread_t)pthread_self();
#else
    return 0;
#endif
}
/* 线程退出 */
PORT_API void system_thread_exit(void* retval)
{
#if PLATFORM_WINDOWS
    ExitThread((DWORD)(uintptr_t)retval);
#elif PLATFORM_POSIX
    pthread_exit(retval);
#else
#endif
}
/* 让出 CPU */
PORT_API bool_t system_thread_yield(void)
{
#if PLATFORM_WINDOWS
    return SwitchToThread() ? 1 : 0;
#elif PLATFORM_POSIX
    return (sched_yield() == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 设置调试名称 */
#if PLATFORM_WINDOWS
/* SetThreadDescription 从 Windows 10 1607 开始提供 动态加载以兼容旧系统 */
typedef HRESULT(WINAPI* SetThreadDescription_t)(HANDLE, PCWSTR);
PORT_HIDE static SetThreadDescription_t system_thread_set_desc = NULL;
PORT_HIDE static system_once_init system_thread_set_desc_init = { 0 };
PORT_HIDE static void system_thread_set_desc_aux(void)
{
    HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
    if (hKernel)
        system_thread_set_desc = (SetThreadDescription_t)GetProcAddress(hKernel, "SetThreadDescription");
}
#endif
/* 
 * 设置调试名称
 * Windows 7/8 及 Windows 10 1607 之前不支持 返回0
 * Windows 10 1607+ 正常支持
 */
PORT_API bool_t system_thread_set_name(system_thread_t thread, const char* name)
{
    if (thread == 0 || name == 0) return 0;
#if PLATFORM_WINDOWS
    wchar_t wbuf[SYSTEM_THREAD_NAME_MAX] = { 0 };
    if (system_string_to_native(wbuf, name, sizeof(wbuf)) == 0)
        return 0; /* 转换失败或缓冲区不足 */
    system_once_run(&system_thread_set_desc_init, system_thread_set_desc_aux);
    if (system_thread_set_desc == NULL)
        return 0; /* 系统不支持 SetThreadDescription */
    return SUCCEEDED(system_thread_set_desc(thread, wbuf)) ? 1 : 0;
#elif PLATFORM_POSIX
    char safe_name[SYSTEM_THREAD_NAME_MAX] = { 0 };
    if (system_string_to_native(safe_name, name, sizeof(safe_name)) == 0)
        return 0;
    return (pthread_setname_np((pthread_t)thread, safe_name) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 设置优先级 */
PORT_API bool_t system_thread_set_priority(system_thread_t thread, qty_t priority_pertage)
{
    if (thread == 0) return 0;
    priority_pertage = priority_pertage % 101;
#if PLATFORM_WINDOWS
    int priority = 0;
    switch (priority_pertage / 14)
    {
    case 0:
        { priority = THREAD_PRIORITY_IDLE; break; }
    case 1:
        { priority = THREAD_PRIORITY_LOWEST; break; }
    case 2:
        { priority = THREAD_PRIORITY_BELOW_NORMAL; break; }
    default:
    case 3:
        { priority = THREAD_PRIORITY_NORMAL; break; }
    case 4:
        { priority = THREAD_PRIORITY_ABOVE_NORMAL; break; }
    case 5:
        { priority = THREAD_PRIORITY_HIGHEST; break; }
    case 6:
    case 7: 
        { priority = THREAD_PRIORITY_TIME_CRITICAL; break;}
    }
    return SetThreadPriority(thread, priority) ? 1 : 0;
#elif PLATFORM_POSIX
    int sched_prio = 1 + (int)((priority_pertage * 96) / 100);
    struct sched_param param = { 0 };
    param.sched_priority = sched_prio;
    return (pthread_setschedparam((pthread_t)thread, SCHED_RR, &param) == 0) ? 1 : 0;
#else
    return 0;
#endif
}

/* 互斥锁 Mutex */

/* 初始化 */
PORT_API bool_t system_mutex_init(system_mutex_t* mutex)
{
    if (mutex == 0) return 0;
#if PLATFORM_WINDOWS
    InitializeCriticalSection((LPCRITICAL_SECTION)mutex);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_mutex_init((pthread_mutex_t*)mutex, NULL) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 加锁 */
PORT_API bool_t system_mutex_lock(system_mutex_t* mutex)
{
    if (mutex == 0) return 0;
#if PLATFORM_WINDOWS
    EnterCriticalSection((LPCRITICAL_SECTION)mutex);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_mutex_lock((pthread_mutex_t*)mutex) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 尝试加锁 */
PORT_API bool_t system_mutex_trylock(system_mutex_t* mutex)
{
    if (mutex == 0) return 0;
#if PLATFORM_WINDOWS
    return TryEnterCriticalSection((LPCRITICAL_SECTION)mutex) ? 1 : 0;
#elif PLATFORM_POSIX
    return (pthread_mutex_trylock((pthread_mutex_t*)mutex) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 解锁 */
PORT_API bool_t system_mutex_unlock(system_mutex_t* mutex)
{
    if (mutex == 0) return 0;
#if PLATFORM_WINDOWS
    LeaveCriticalSection((LPCRITICAL_SECTION)mutex);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_mutex_unlock((pthread_mutex_t*)mutex) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 销毁 */
PORT_API bool_t system_mutex_del(system_mutex_t* mutex)
{
    if (mutex == 0) return 0;
#if PLATFORM_WINDOWS
    DeleteCriticalSection((LPCRITICAL_SECTION)mutex);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_mutex_destroy((pthread_mutex_t*)mutex) == 0) ? 1 : 0;
#else
    return 0;
#endif
}

/* 条件变量 Condition Variable */

/* 初始化 */
PORT_API bool_t system_cond_init(system_cond_t* cond)
{
    if (cond == 0) return 0;
#if PLATFORM_WINDOWS
    InitializeConditionVariable((PCONDITION_VARIABLE)cond);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_cond_init((pthread_cond_t*)cond, NULL) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 等待 等待期间释放锁 结束后重新加锁 */
PORT_API bool_t system_cond_wait(system_cond_t* cond, system_mutex_t* mutex)
{
    if ((cond && mutex) == 0) return 0;
#if PLATFORM_WINDOWS
    return SleepConditionVariableCS((PCONDITION_VARIABLE)cond, (LPCRITICAL_SECTION)mutex, INFINITE) ? 1 : 0;
#elif PLATFORM_POSIX
    return (pthread_cond_wait((pthread_cond_t*)cond, (pthread_mutex_t*)mutex) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 定时等待 等待期间释放锁 结束后重新加锁 */
PORT_API bool_t system_cond_timedwait(system_cond_t* cond, system_mutex_t* mutex, qty_t ms)
{
    if ((cond && mutex) == 0) return 0;
#if PLATFORM_WINDOWS
    return SleepConditionVariableCS((PCONDITION_VARIABLE)cond, (LPCRITICAL_SECTION)mutex, (DWORD)ms) ? 1 : 0;
#elif PLATFORM_POSIX
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }
    return (pthread_cond_timedwait((pthread_cond_t*)cond, (pthread_mutex_t*)mutex, &ts) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 唤醒一个 */
PORT_API bool_t system_cond_wake_one(system_cond_t* cond)
{
    if (cond == 0) return 0;
#if PLATFORM_WINDOWS
    WakeConditionVariable((PCONDITION_VARIABLE)cond);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_cond_signal((pthread_cond_t*)cond) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 唤醒全部 */
PORT_API bool_t system_cond_wake_all(system_cond_t* cond)
{
    if (cond == 0) return 0;
#if PLATFORM_WINDOWS
    WakeAllConditionVariable((PCONDITION_VARIABLE)cond);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_cond_broadcast((pthread_cond_t*)cond) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 销毁 */
PORT_API bool_t system_cond_del(system_cond_t* cond)
{
    if (cond == 0) return 0;
#if PLATFORM_WINDOWS
    /* 无需显式销毁,无资源 */
    return 1;
#elif PLATFORM_POSIX
    return (pthread_cond_destroy((pthread_cond_t*)cond) == 0) ? 1 : 0;
#else
    return 0;
#endif
}

/* 读写锁 RWLock */

/* 初始化 */
PORT_API bool_t system_rwlock_init(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    InitializeSRWLock((PSRWLOCK)rw);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_init((pthread_rwlock_t*)rw, NULL) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 读加锁 */
PORT_API bool_t system_rwlock_rdlock(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    AcquireSRWLockShared((PSRWLOCK)rw);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_rdlock((pthread_rwlock_t*)rw) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 写加锁 */
PORT_API bool_t system_rwlock_wrlock(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    AcquireSRWLockExclusive((PSRWLOCK)rw);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_wrlock((pthread_rwlock_t*)rw) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 解锁读锁 */
PORT_API bool_t system_rwlock_unlock_rd(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    ReleaseSRWLockShared((PSRWLOCK)rw);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_unlock((pthread_rwlock_t*)rw) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 解锁写锁 */
PORT_API bool_t system_rwlock_unlock_wr(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    ReleaseSRWLockExclusive((PSRWLOCK)rw);
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_unlock((pthread_rwlock_t*)rw) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 销毁 */
PORT_API bool_t system_rwlock_del(system_rwlock_t* rw)
{
    if (rw == 0) return 0;
#if PLATFORM_WINDOWS
    /* SRWLOCK 无需销毁 */
    return 1;
#elif PLATFORM_POSIX
    return (pthread_rwlock_destroy((pthread_rwlock_t*)rw) == 0) ? 1 : 0;
#else
    return 0;
#endif
}

/* 线程局部存储 TLS */

/* 初始化 */
PORT_API bool_t system_tls_key_init(system_tls_key* key, void (PLATFORM_CALL *destructor)(void*))
{
    if (key == 0) return 0;
#if PLATFORM_WINDOWS
    system_tls_key tmp_key = FlsAlloc(destructor);
    if ((DWORD)tmp_key == FLS_OUT_OF_INDEXES)
        return 0;
    *key = tmp_key;
    return 1;
#elif PLATFORM_POSIX
    return (pthread_key_create((pthread_key_t*)key, destructor) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 删除键 */
PORT_API bool_t system_tls_key_del(system_tls_key key)
{/* 释放 TLS 键,不会触发现有值的析构 */
#if PLATFORM_WINDOWS
    return FlsFree((DWORD)key);
#elif PLATFORM_POSIX
    return (pthread_key_delete((pthread_key_t)key) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 设置值 */
PORT_API bool_t system_tls_key_set(system_tls_key key, void* value)
{/* 设置当前线程中 key 关联的值 */
#if PLATFORM_WINDOWS
    return FlsSetValue((DWORD)key, value);
#elif PLATFORM_POSIX
    return (pthread_setspecific((pthread_key_t)key, value) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 获取值 */
PORT_API void* system_tls_key_get(system_tls_key key)
{/* 获取当前线程中 key 关联的值 */
#if PLATFORM_WINDOWS
    return FlsGetValue((DWORD)key);
#elif PLATFORM_POSIX
    return pthread_getspecific((pthread_key_t)key);
#else
    return 0;
#endif
}

/* 单次初始化 Once */

#if PLATFORM_WINDOWS
PORT_HIDE static BOOL system_once_run_aux_00(PINIT_ONCE o, PVOID p, PVOID* c)
{
    union { void* ptr; void (*fn)(void); } conv;
    (void)o;
    (void)c;
    conv.ptr = p;
    conv.fn();
    return 1;
}
#endif
/* 安全初始化 */
PORT_API bool_t system_once_run(system_once_init* once, void (*fn)(void))
{
    if (once == 0 || fn == 0)return 0;
#if PLATFORM_WINDOWS
    union { void (*fn)(void); void* ptr; } conv;
    conv.fn = fn;
    return InitOnceExecuteOnce((PINIT_ONCE)once, system_once_run_aux_00, conv.ptr, (void*)0);
#elif PLATFORM_POSIX
    return (pthread_once(once, fn) == 0) ? 1 : 0;
#else
    return 0;
#endif
}


#else


PORT_API system_thread_t system_thread_new(system_thread_main main, void* arg) { ; }
PORT_API bool_t system_thread_join(system_thread_t thread, void** retval) { ; }
PORT_API bool_t system_thread_detach(system_thread_t thread) { ; }
PORT_API system_thread_t system_thread_self(void) { ; }
PORT_API void system_thread_exit(void* retval) { ; }
PORT_API bool_t system_thread_yield(void) { ; }
PORT_API bool_t system_thread_set_name(system_thread_t thread, const char* name) { ; }
PORT_API bool_t system_thread_set_priority(system_thread_t thread, qty_t priority_percentage) { ; }
PORT_API bool_t system_mutex_init(system_mutex_t* mutex) { ; }
PORT_API bool_t system_mutex_lock(system_mutex_t* mutex) { ; }
PORT_API bool_t system_mutex_trylock(system_mutex_t* mutex) { ; }
PORT_API bool_t system_mutex_unlock(system_mutex_t* mutex) { ; }
PORT_API bool_t system_mutex_del(system_mutex_t* mutex) { ; }
PORT_API bool_t system_cond_init(system_cond_t* cond) { ; }
PORT_API bool_t system_cond_wait(system_cond_t* cond, system_mutex_t* mutex) { ; }
PORT_API bool_t system_cond_timedwait(system_cond_t* cond, system_mutex_t* mutex, qty_t ms) { ; }
PORT_API bool_t system_cond_wake_one(system_cond_t* cond) { ; }
PORT_API bool_t system_cond_wake_all(system_cond_t* cond) { ; }
PORT_API bool_t system_cond_del(system_cond_t* cond) { ; }
PORT_API bool_t system_rwlock_init(system_rwlock_t* rw) { ; }
PORT_API bool_t system_rwlock_rdlock(system_rwlock_t* rw) { ; }
PORT_API bool_t system_rwlock_wrlock(system_rwlock_t* rw) { ; }
PORT_API bool_t system_rwlock_unlock_rd(system_rwlock_t* rw) { ; }
PORT_API bool_t system_rwlock_unlock_wr(system_rwlock_t* rw) { ; }
PORT_API bool_t system_rwlock_del(system_rwlock_t* rw) { ; }
PORT_API bool_t system_tls_key_init(system_tls_key* key, void (PLATFORM_CALL* destructor)(void*)) { ; }
PORT_API bool_t system_tls_key_del(system_tls_key key) { ; }
PORT_API bool_t system_tls_key_set(system_tls_key key, void* value) { ; }
PORT_API void* system_tls_key_get(system_tls_key key) { ; }
PORT_API bool_t system_once_run(system_once_init* once, void (*fn)(void)) { ; }


#endif