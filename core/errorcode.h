#ifndef CORE_ERRORCODE_H
#define CORE_ERRORCODE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


enum error
{
	/* 未知错误 */
	ERROR_UNKNOWN = -1,
	/* 没有发现错误 */
	ERROR_NULL = 0,
	/* 违法参数传入 */
	ERROR_PARAMTER = 1,
	/* 语法错误 */
	ERROR_SYNTAX = 2,
	/* 功能暂无实现 */
	ERROR_UNFULFILL,
	/* 无法申请内存 */
	ERROR_MEMORY_LACK,
	/* 内存覆盖行为 */
	ERROR_MEMORY_OVER,

};


EXTERN_C_END
#endif