#ifndef CORE_MEM_H
#define CORE_MEM_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 复制内存块 返回目标内存块 源不能覆盖目标 如果覆盖关闭COMMON_RESTRICT优化 */
PORT_API void* mem_cpy(void* PLATFORM_RESTRICT dst, const void* PLATFORM_RESTRICT src, qty_t size);
/* 复制内存块 返回目标内存块 源与目标允许重叠 */
PORT_API void* mem_mov(void* dst, const void* src, qty_t size);
/* 将目标内存块设置为字节源 设置大小为size */
PORT_API void* mem_set(void* dst, unsigned char value, qty_t size);
/* 比较内存块 返回字节的目标与源之差 */
PORT_API cmp_t mem_cmp(const void* dst, const void* src, qty_t size);
/* 在目标中查找字节源 范围为size */
PORT_API void* mem_chr(const void* dst, unsigned char value, qty_t size);



EXTERN_C_END
#endif