#ifndef ALGO_SORT_H
#define ALGO_SORT_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/*
 * 快速排序(原地, 非稳定)
 * @param src      数据缓冲区首地址
 * @param count     元素个数
 * @param elem 单个元素字节大小
 * @param cmp       比较函数, 当第一个参数大于第二个参数时返回 true
 * @warning 平均 O(n log n), 最坏 O(n²), 不保证相等元素的相对顺序
 */
PORT_API void sort_quick(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*));

/*
 * 插入排序(原地, 稳定)
 * @param src      数据缓冲区首地址
 * @param count     元素个数
 * @param elem 单个元素字节大小
 * @param cmp       比较函数, 当第一个参数大于第二个参数时返回 true
 * @warning 时间复杂度 O(n²), 适合小规模(<32)或近乎有序的数据
 */
PORT_API void sort_insert(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*));

/*
 * 归并排序(稳定, 需要外部提供等大小的临时缓冲区)
 * @param src      数据缓冲区首地址
 * @param count     元素个数
 * @param elem 单个元素字节大小
 * @param cmp       比较函数, 当第一个参数大于第二个参数时返回 true
 * @param temp      外部提供的临时缓冲区, 大小至少 count * elem
 * @return 成功返回 ERR_OK；若 temp 为 NULL 且 count > 1 返回错误码
 * @warning 时间复杂度稳定 O(n log n), 需要额外 O(n) 空间
 */
PORT_API bool_t sort_merge(void* src, qty_t count, qty_t elem, cmp_t(*cmp)(void*, void*), void* temp);


EXTERN_C_END
#endif