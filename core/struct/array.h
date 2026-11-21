#ifndef STRUCT_ARRAY_H
/* 数组 */
#define STRUCT_ARRAY_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
EXTERN_C_BEGIN

/* 数组 */
struct array
{
	/* 大小 */
	qty_t elem;
	/* 容量 */
	qty_t cap;
	/* 元素 */
	void* data;
	/* 当前数量 */
	qty_t count;
};

/*
* 初始化ptr为array
* elem元素大小 cap元素容量
* 当使用柔性数组特性时 ptr应有array+data大小
*/
PORT_API struct array* array_init(void* ptr, qty_t elem, qty_t cap);
/*
* 扩展array 当采用柔性数组特性时不应使用当前函数
* 当传递old时获取旧指针
* 需要传递新data的指针和元素容量
*/
PORT_API bool_t array_ext(struct array* array, void** old, void* ptr, qty_t cap);
/* 清空array */
PORT_API void array_clear(struct array* array);

/* 压入一个元素 */
PORT_API bool_t array_push(struct array* array, const void* src);
/* 弹出一个元素 */
PORT_API bool_t array_pop(struct array* array);
/* 根据索引删除一个元素 当为非尾时触发向前补位 */
PORT_API bool_t array_remove(struct array* array,qty_t index);

/* 根据索引设置元素 成功true 失败false */
PORT_API bool_t array_set(struct array* array, const void* src, qty_t index);
/* 返回索引元素指针 当传递src时复制 返回指针可能无效 */
PORT_API void* array_get(struct array* array, void* dst, qty_t index);

/* 返回查找到元素索引 失败返回cap */
PORT_API qty_t array_find(struct array* array, const void* value, cmp_t(*cmp)(void*, void*));
/* 根据函数返回排序 函数当上元素大于下元素时应返回正数 小于等于时返回负数或零 */
PORT_API bool_t array_sort(struct array* array, cmp_t(*cmp)(void*, void*));


EXTERN_C_END
#endif