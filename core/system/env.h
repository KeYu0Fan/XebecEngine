#ifndef SYSTEM_ENV_H
/* ----环境变量----
*
* windows 使用system实现
* linux 使用glibc实现
*
----环境变量---- */
#define SYSTEM_ENV_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


/* 获取环境变量 */
PORT_API bool_t system_env_get(char* buf, const char* name, qty_t size);
/* 设置环境变量 若已存在则覆盖 */
PORT_API bool_t system_env_set(const char* name, const char* value);
/* 删除环境变量 */
PORT_API bool_t system_env_unset(const char* name);


EXTERN_C_END
#endif