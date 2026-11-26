#ifndef DIAG_LOG_H
#define DIAG_LOG_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 初始化日志 */
PORT_API bool_t log_init(qty_t interval_ms, qty_t busy_threshold_ms, void(*call)(void* user, char* data, qty_t size), void* user);
/* 写入信息 未有长度时计算 */
PORT_API void log_write(char* data, qty_t len);
/* 提前回调 */
PORT_API void log_flush(void);
/* 销毁日志 */
PORT_API void log_del(void);


EXTERN_C_END
#endif