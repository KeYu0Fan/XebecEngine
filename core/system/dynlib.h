#ifndef SYSTEM_DYNLIN_H
/* ----动态库加载----
*
* system_dynlib_error
* windows 未映射全部错误 linux 标准实现
*
----动态库加载---- */
#define SYSTEM_DYNLIB_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_dynlib_t;

/* 加载动态库 */
PORT_API system_dynlib_t system_dynlib_load(const char* path);
/* 查看是否已加载动态库 */
PORT_API system_dynlib_t system_dynlib_is_loaded(const char* path);
/* 获取符号对应地址 */
PORT_API void* system_dynlib_symbol(system_dynlib_t lib, const char* name);
/* 卸载动态库 */
PORT_API bool_t system_dynlib_unload(system_dynlib_t lib);
/* 获取动态库错误 */
PORT_API const char* system_dynlib_error(void);


EXTERN_C_END
#endif