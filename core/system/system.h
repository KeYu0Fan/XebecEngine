#ifndef SYSTEM_SYSTEM_H
#define SYSTEM_SYSTEM_H


#include <stdio.h>
#include <stdlib.h>


/* ----全局----
* 强制依赖base
* 版本需求 Win7 SP1 + 更新  /  Linux 3.10+ (glibc 2.17+)
----全局----*/


/* ----线程----
* 有不一致行为,具体请查看
----线程---- */
#include "core/system/thread.h"


/* ----进程----
* 有不一致行为,具体请查看
----进程---- */
#include "core/system/process.h"


/* ----匿名管道----
* 有不一致行为,具体请查看
----匿名管道---- */
#include "core/system/pipe.h"


/* ----命令----
* 依赖 process pipe
----命令---- */
#include "core/system/cmd.h"


/* ----特权----
* 有不一致行为,具体请查看
* 强制依赖 <securitybaseapi.h>
----特权---- */
#include "core/system/priv.h"


/* ----网络----
* 依赖 thread
* 强制依赖 <winsock2.h> ws2_32.lib
----网络---- */
#include "core/system/net.h"


/* ----内存映射----
* 有不一致行为,具体请查看
* 依赖 thread
* 强制依赖 kernel32.dll 以dll形式链接
* 强制依赖 <psapi.h> 以dll形式链接
----内存映射---- */
#include "core/system/mmap.h"


/* ----文件----
* 有不一致行为,具体请查看
* 部分强制依赖mmap 非mmap环境有溢出风险
----文件---- */
#include "core/system/file.h"


/* ----系统信息----
* 有不一致行为,具体请查看
* 依赖 thread
* 强制依赖 ntdll.dll 以dll形式链接
* 强制依赖 <pdhmsg.h> <pdh.h>
----系统信息---- */
#include "core/system/info.h"


/* ----动态库加载----
* 有不一致行为,具体请查看
* 依赖 thread mmap
----动态库加载---- */
#include "core/system/dynlib.h"


/* ----时间----
* timer 与其他库实现不一致
* timer 强制依赖 thread mmap
----时间---- */
#include "core/system/time.h"


/* ----环境变量----
* 依赖 thread mmap
----环境变量---- */
#include "core/system/env.h"


#endif