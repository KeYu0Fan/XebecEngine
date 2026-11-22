#ifndef SYSTEM_BASE_H
#define SYSTEM_BASE_H


#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"

#if PLATFORM_WINDOWS
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#include <winsock2.h>
#include <windows.h>
#include <pdhmsg.h>
#include <pdh.h>
#include <securitybaseapi.h>
#elif PLATFORM_POSIX
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>          /* sysconf() 在这里 */
#include <sys/utsname.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <poll.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sysinfo.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#else
/**/
#endif

#if PLATFORM_WINDOWS
/**/
/* 基础 */
COM_STATIC_ASSERT(sizeof(HANDLE) <= sizeof(system_handle_h), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(long long) <= sizeof(system_off_t), systemtypebaseassert);
/* thread */
COM_STATIC_ASSERT(sizeof(DWORD) <= sizeof(system_thread_main_ret), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(CRITICAL_SECTION) <= sizeof(system_mutex_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(CONDITION_VARIABLE) <= sizeof(system_cond_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(SRWLOCK) <= sizeof(system_rwlock_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(DWORD) <= sizeof(system_handle_h), systemtypebaseassert);   /* tls_key */
COM_STATIC_ASSERT(sizeof(INIT_ONCE) <= sizeof(system_once_init), systemtypebaseassert);
/* net */
COM_STATIC_ASSERT(sizeof(SOCKET) <= sizeof(system_handle_h), systemtypebaseassert);
/* dynlib */
COM_STATIC_ASSERT(sizeof(HMODULE) <= sizeof(system_handle_h), systemtypebaseassert);
#elif PLATFORM_POSIX
/**/
/* 基础 */
COM_STATIC_ASSERT(sizeof(int) <= sizeof(system_handle_h), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(off_t) <= sizeof(system_off_t), systemtypebaseassert);
/* thread */
COM_STATIC_ASSERT(sizeof(pthread_t) <= sizeof(system_handle_h), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(void*) <= sizeof(system_thread_main_ret), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(pthread_mutex_t) <= sizeof(system_mutex_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(pthread_cond_t) <= sizeof(system_cond_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(pthread_rwlock_t) <= sizeof(system_rwlock_t), systemtypebaseassert);
COM_STATIC_ASSERT(sizeof(pthread_key_t) <= sizeof(system_handle_h), systemtypebaseassert);   /* tls_key */
COM_STATIC_ASSERT(sizeof(pthread_once_t) <= sizeof(system_once_init), systemtypebaseassert);
/* process */
COM_STATIC_ASSERT(sizeof(pid_t) <= sizeof(system_handle_h), systemtypebaseassert);
#endif

#endif