#include "array.h"
#if COM_USE(CONFIG_STRUCT_ARRAY)


#include "core/mem.h"
#include "core/algo/sort.h"

/*
* 初始化ptr为array
* elem元素大小 cap元素容量
* 当使用柔性数组特性时 ptr应有array+data大小
*/
PORT_API struct array* array_init(void* ptr, qty_t elem, qty_t cap)
{
	COM_RETURN(ptr == 0 || elem == 0, 0);
	struct array* array = (struct array*)ptr;
	array->elem = elem;
	array->cap = cap;
	array->data = 0;
	if (cap > 0)
		array->data = (void*)((char*)array + sizeof(struct array));
	array->count = 0;
	return array;
}
/*
* 扩展array 当采用柔性数组特性时不应使用当前函数
* 当传递old时获取旧指针
* 需要传递新data的指针和元素容量
*/
PORT_API bool_t array_ext(struct array* array, void** old, void* ptr, qty_t cap)
{
	COM_RETURN(array == 0 || ptr == 0 || cap == 0, 0);
	void* tmp = array->data;
	if (old)
		*old = tmp;
	array->data = ptr;
	array->cap = cap;
	array->count = (array->count < cap ? array->count : cap);
	if (array->count) mem_mov(ptr, tmp, array->elem * array->count);
	return 1;
}
/* 清空array */
PORT_API void array_clear(struct array* array)
{
	COM_RETURN(array == 0, );
	array->count = 0;
}

/* 压入一个元素 */
PORT_API bool_t array_push(struct array* array, const void* src)
{
	COM_RETURN(array == 0 || src == 0, 0);
	if (array->count >= array->cap)
		return 0;
	void* dst = (void*)((char*)array->data + array->count * array->elem);
	mem_mov(dst, src, array->elem);
	array->count++;
	return 1;
}
/* 弹出一个元素 */
PORT_API bool_t array_pop(struct array* array)
{
	COM_RETURN(array == 0 || array->count == 0, 0);
	array->count--;
	return 1;
}
/* 根据索引删除一个元素 当为非尾时触发向前补位 */
PORT_API bool_t array_remove(struct array* array, qty_t index)
{
	COM_RETURN(array == 0 || index >= array->count, 0);
	void* dst = (void*)((char*)array->data + array->elem * index);
	void* src = (void*)((char*)dst + array->elem);
	array->count--;
	qty_t count = array->elem * (array->count - index);
	if (count) mem_mov(dst, src, count);
	return 1;
}

/* 根据索引设置元素 成功true 失败false */
PORT_API bool_t array_set(struct array* array, const void* src, qty_t index)
{
	COM_RETURN(array == 0 || src == 0 || index >= array->count, 0);
	void* dst = (void*)((char*)array->data + array->elem * index);
	return mem_mov(dst, src, array->elem) ? 1 : 0;
}
/* 返回索引元素指针 当传递src时复制 返回指针可能无效 */
PORT_API void* array_get(struct array* array, void* dst, qty_t index)
{
	COM_RETURN(array == 0 || index >= array->count, 0);
	void* src = (void*)((char*)array->data + array->elem * index);
	if (dst)
		mem_mov(dst, src, array->elem);
	return src;
}

/* 返回查找到元素索引 失败返回cap */
PORT_API qty_t array_find(struct array* array, const void* value, cmp_t(*cmp)(void*, void*))
{
	COM_RETURN(array == 0 || value == 0 || cmp == 0 || array->count == 0, (array ? array->cap : 0));
	void* dst = array->data;
	qty_t i = 0;
	while (i < array->count)
	{
		if (cmp(dst, (void*)value) == 0) return i;
		dst = (void*)((char*)dst + array->elem);
		i++;
	}
	return array->cap;
}
/* 根据函数返回排序 函数当上元素大于下元素时应返回正数 小于等于时返回负数或零 */
PORT_API bool_t array_sort(struct array* array, cmp_t(*cmp)(void*, void*))
{
	COM_RETURN(array == 0 || cmp == 0, 0);
	sort_quick(array->data, array->count, array->elem, cmp);
	return 1;
}


#else


PORT_API struct array* array_init(void* ptr, qty_t elem, qty_t cap) { ; }
PORT_API bool_t array_ext(struct array* array, void** old, void* ptr, qty_t cap) { ; }
PORT_API void array_clear(struct array* array) { ; }
PORT_API bool_t array_push(struct array* array, const void* src) { ; }
PORT_API bool_t array_pop(struct array* array) { ; }
PORT_API bool_t array_remove(struct array* array, qty_t index) { ; }
PORT_API bool_t array_set(struct array* array, const void* src, qty_t index) { ; }
PORT_API void* array_get(struct array* array, void* dst, qty_t index) { ; }
PORT_API qty_t array_find(struct array* array, const void* value, cmp_t(*cmp)(void*, void*)) { ; }
PORT_API bool_t array_sort(struct array* array, cmp_t(*cmp)(void*, void*)) { ; }


#endif