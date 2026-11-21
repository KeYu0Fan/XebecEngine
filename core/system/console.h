#ifndef SYSTEM_CONSOLE_H
/* ----控制台----
* 依赖可能有不一致行为
----控制台---- */
#define SYSTEM_CONSOLE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


/* 初始化控制台 */
PORT_API bool_t system_console_init(void);
/*
* 重置控制台句柄
* 0read 1write 2error
*/
PORT_API bool_t system_console_reset(qty_t which);
/* 设置控制台句柄 */
PORT_API bool_t system_console_set(qty_t which, system_handle_h handle);
/* 获取控制台句柄 */
PORT_API system_handle_h system_console_get(qty_t which);
/* 读取句柄 buf输出输入流,count字节数量 buf_size读取的字节数 */
PORT_API bool_t system_console_read(system_handle_h fp, void* buf, qty_t count, qty_t* buf_size);
/* 写入句柄 */
PORT_API bool_t system_console_write(system_handle_h fp, const void* buf, qty_t count, qty_t* buf_size);


EXTERN_C_END
#endif