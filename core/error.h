#ifndef CORE_ERROR_H
#define CORE_ERROR_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/errorcode.h"
EXTERN_C_BEGIN


/*
 * 将错误码转译为可读的字符串
 * 静态存储 无需销毁
 */
PORT_API const char* error_to_string(err_t error);


EXTERN_C_END
#endif