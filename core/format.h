#ifndef CORE_FORMAT_H
#define CORE_FORMAT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/*
 * 有符号整数转十进制字符串
 * buff 输出缓冲区, bsize 容量(含 '\0')
 * value 指向 1/2/4/8 字节的整数, vsize 为字节数
 * 成功返回 1 并写入 '\0'; 缓冲区不足/参数非法返回 0
 */
PORT_API bool_t format_dtos(void* buff, qty_t bsize, const void* value, qty_t vsize);
/*
 * 无符号整数转十进制字符串
 * 参数与 format_dtos 相同, 按无符号解释
 */
PORT_API bool_t format_utos(void* buff, qty_t bsize, const void* value, qty_t vsize);
/*
 * 浮点数转最短可往返的十进制字符串
 * value 指向 4 字节 float 或 8 字节 double, vsize 为 4/8
 * 输出为 %g 风格(必要时带指数), 特殊值为 nan/inf/-inf
 * 缓冲区不足/参数非法返回 0
 */
PORT_API bool_t format_ftos(void* buff, qty_t bsize, const void* value, qty_t vsize);
/*
 * 内存字节转小写十六进制字符串
 * 输出 2*vsize 个字符, bsize 需 >= 2*vsize + 1
 * 成功返回 1 并写入 '\0'
 */
PORT_API bool_t format_mtos(void* buff, qty_t bsize, const void* value, qty_t vsize);


EXTERN_C_END
#endif