#ifndef CORE_TYPEDEF_H
#define CORE_TYPEDEF_H

/* ---- 定宽基础类型 ---- */
typedef signed char        s8_t;
typedef unsigned char      u8_t;
typedef short              s16_t;
typedef unsigned short     u16_t;
typedef int                s32_t;
typedef unsigned int       u32_t;
typedef signed long long   s64_t;
typedef unsigned long long u64_t;
typedef float              f32_t;
typedef double             f64_t;

/* ---- 语义类型 ---- */
/* 布尔: 非 0 为真 */
typedef s32_t bool_t;
/* 哈希键 */
typedef s32_t key_t;
/* 错误码 */
typedef s32_t err_t;
/* 比较结果 */
typedef s32_t cmp_t;
/* 标量(参考 Godot; 整体切双精度时改为 f64_t) */
typedef f32_t real_t;
/* 有符号数量/差值 */
typedef s64_t sqty_t;
/* 数量/大小/下标 */
typedef u64_t qty_t;
/* 不透明句柄 */
typedef void* handle;
/* 泛型指针 */
typedef void* ptr_t;


#endif