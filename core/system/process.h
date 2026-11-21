#ifndef SYSTEM_PROCESS_H
/*  ----进程----
* system_process_spawn attr.window linux无相关概念
----进程---- */
#define SYSTEM_PROCESS_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_process_t;
struct system_process_attr_t
{
    /* 参数列表 以((void*)0)结尾 */
    const char* const* argv;
    /* 环境变量 以((void*)0)结尾 ((void*)0)表示继承 */
    const char* const* envp;
    /* 工作目录 ((void*)0)表示继承 */
    const char* workdir;
    /* 标准输入重定向 */
    system_handle_h stdin_h;
    /* 标准输出重定向 */
    system_handle_h stdout_h;
    /* 标准错误重定向 */
    system_handle_h stderr_h;
    /* 额外要继承的句柄数组 */
    system_handle_h* h_arr;
    /* 数组长度 */
    qty_t h_len;
    /* 进程分离 让子进程在父进程退出后继续运行 */
    bool_t detach;
    /* -1不创建窗口 0继承窗口 1创建新窗口 */
    bool_t window;
    /* 生命周期绑定父进程 0不开启 1开启 */
    /* 和进程分离互斥 优先级更低 当都设置时只有进程分离生效 */
    bool_t exit_extends;
};
struct system_process_info_t
{
    /* 进程 ID */
    qty_t pid;
    /* 1=已退出 0=运行中 */
    bool_t exited;
    /* 退出码 仅在 exited=1 时有效 */
    system_status_t exit_code;
    /* 导致退出的信号 POSIX有效 Windows为0 */
    system_status_t signal;
};

/* 创建新的进程 */
PORT_API system_process_t system_process_spawn(const char* path,const struct system_process_attr_t* attr);
/* 获取当前进程ID */
PORT_API qty_t system_process_getpid(void);
/*
* 等待进程退出 支持超时
* proc:进程句柄 info:输出进程信息|退出码和状态 timeout_ms:超时毫秒数|-1表示无限等待|0表示立即返回
* 返回 { 1=进程已退出|info有效 0=超时|pid和exited=0有效|其他字段无效 -1=错误 }
*/
PORT_API bool_t system_process_wait(system_process_t proc, struct system_process_info_t* info, qty_t timeout_ms);
/* 终止进程 强制结束 */
PORT_API bool_t system_process_kill(system_process_t proc);
/* 终止当前进程 */
PORT_API bool_t system_process_kill_self(void);
/* 清理进程 回收资源 */
PORT_API bool_t system_process_close(system_process_t proc);


EXTERN_C_END
#endif