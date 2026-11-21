#ifndef SYSTEM_INFO_H
/* ----系统信息----
*
* system_info_set_hostname
* 需要root权限
* windows 需要重启系统 linux 需要重启网络服务
*
* system_info_get_loadavg
* windows 使用system_info_aux_loadavg实现 linux 使用系统接口sysinfo
*
* system_info_get_hostname
* win 确保版本Vista+
*
----系统信息---- */
#define SYSTEM_INFO_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


/* 获取操作系统名称 静态存储, 无需释放 */
PORT_API const char* system_info_get_os_name(void);
/* 获取操作系统版本号,静态存储,无需释放 */
PORT_API const char* system_info_get_os_version(void);
/* 获取逻辑 CPU 核心数 */
PORT_API qty_t system_info_get_cpu_cores(void);
/*
 * 获取系统累计 CPU 时间(自开机) 单位 100ms
 * user:   用户态时间
 * kernel: 内核态时间 不含idle
 * idle:   空闲时间
 * 总时间 = user + kernel + idle
 * 忙碌时间 = user + kernel
 * 使用率 = 1 - (idle_new - idle_old) / (总时间_new - 总时间_old)
 */
PORT_API bool_t system_info_get_cpu_times(qty_t* user, qty_t* kernel, qty_t* idle);
/* 获取物理内存总量 字节 */
PORT_API qty_t system_info_get_total_ram(void);
/* 获取空闲物理内存总量 字节 */
PORT_API qty_t system_info_get_free_ram(void);
/* 获取内存页面大小 */
PORT_API qty_t system_info_get_page_size(void);
/* 获取主机名 */
PORT_API bool_t system_info_get_hostname(char* buf, qty_t size);
/* 获取系统启动后的运行时间s */
PORT_API qty_t system_info_get_uptime(void);
/* 获取系统负载平均值 1|5|15 分钟 */
PORT_API bool_t system_info_get_loadavg(system_float_t* load1, system_float_t* load5, system_float_t* load15);
/* 系统主机名设置 需特权 */
PORT_API bool_t system_info_set_hostname(const char* name);


EXTERN_C_END
#endif