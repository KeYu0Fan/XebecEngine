#ifndef SYSTEM_PRIV_H
/* ----特权----
* 修改权限 请确保拥有管理员/root
* 部分权限直接依赖管理员/root 无法禁用 视为成功
* 部分枚举在linux/windows下对应多个权限
----特权---- */
#define SYSTEM_PRIV_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


enum system_priv_t
{
    /* N/A 不适用占位符 */
    SYSTEM_PRIV_NA_MIN,
    /* #普通 */
    /* 创建符号链接 软链接 */
    SYSTEM_PRIV_SYMLINK,
    /* 绑定特权端口 <1024 */
    SYSTEM_PRIV_NET_BIND,
    /* 实时调度线程优先级 */
    SYSTEM_PRIV_PRIORITY,
    /* 调试进程 */
    SYSTEM_PRIV_DEBUG,
    /* 加载驱动 / 内核模块 */
    SYSTEM_PRIV_LOAD_DRIVER,
    /* 备份文件 绕过读权限 */
    SYSTEM_PRIV_BACKUP,
    /* 恢复文件 绕过写权限 */
    SYSTEM_PRIV_RESTORE,
    /* 关机 / 重启系统 */
    SYSTEM_PRIV_SHUTDOWN,
    /* 锁定物理内存 */
    SYSTEM_PRIV_MEMLOCK,
    /* 管理卷 如磁盘操作 */
    SYSTEM_PRIV_VOLUME,
    /* #特殊 */
    /* 仅用于释放所有特权 */
    SYSTEM_PRIV_ALL,
    /* 仅用于释放已知特权 */
    SYSTEM_PRIV_KNOWN,
    /* #危险 */
    /* 原始套接字 */
    SYSTEM_PRIV_NET_RAW,
    /* 文件/对象所有权修改 */
    SYSTEM_PRIV_TAKE_OWNERSHIP,
    /* 固件uefi/blos环境变量修改 */
    SYSTEM_PRIV_FIRMWARE,
    /* N/A 不适用占位符 */
    SYSTEM_PRIV_NA_MAX,
};

/* 启用指定的特权 */
PORT_API bool_t system_priv_open(enum system_priv_t priv);
/* 禁用指定的特权 */
PORT_API bool_t system_priv_close(enum system_priv_t priv);
/* 检查当前进程是否拥有指定的特权 */
PORT_API bool_t system_priv_has(enum system_priv_t priv);


EXTERN_C_END
#endif