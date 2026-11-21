#include "realm.h"
#if COM_USE(CONFIG_STRUCT_REALM)


PORT_HIDE static void realm_push(struct realm* dst, struct realm* src)
{
	if (dst->next)
		dst->next->prev = src;
	src->next = dst->next;
	dst->next = src;
	src->prev = dst;
}
PORT_HIDE static void realm_pop(struct realm** dst, struct realm* src)
{
	if (src->prev)
		src->prev->next = src->next;
	if (src->next)
		src->next->prev = src->prev;
	if (src->ref)
		*src->ref = 0;
	src->ref = 0;
	src->prev = 0;
	src->next = 0;

	if (*dst)
	{
		(*dst)->prev = src;
		src->next = (*dst);
	}
	(*dst) = src;
}
/* 初始化内存管理 将一段内存以更详细安全的方式管理 */
PORT_API bool_t realm_init(struct realm* realm, void* ptr, qty_t size, struct realm** ref)
{
	COM_RETURN(realm == 0 || ptr == 0 || size == 0, 0);
	realm->prev = 0;
	realm->next = 0;
	realm->ref = 0;
	realm->size = size;
	realm->life = 0;
	realm->data = ptr;
	return 1;
}
/* 分配内存 ref用于给当前节点剩余内存添加引用方 res为节点资源站 用于获取释放节点 */
PORT_API void* realm_new(struct realm* realm, qty_t size, struct realm** ref, struct realm** res)
{
	size = ((size + sizeof(qty_t) - 1) & ~(sizeof(qty_t) - 1)) + sizeof(void*);
	COM_RETURN(realm == 0 ||size == 0 || ref == 0 || res == 0, 0);
	if (realm->size < size) return 0;
	if (realm->ref)
		*realm->ref = 0;
	realm->ref = 0;
	realm->life = 1;
	*(struct realm**)realm->data = realm;
	if (*res && realm->size > size)
	{
		struct realm* node = *res;
		(*res)->next->prev = 0;
		(*res) = (*res)->next;
		realm_init(node, realm->data + size, realm->size - size, ref);
		realm_push(realm, node);
		node->life = 0;
		node->ref = ref;
		*ref = node;
		realm->size = size;
	}
	else
		*ref = 0;
	return realm->data + sizeof(struct realm*);
}
/* 释放内存 ref用于为释放的节点添加引用方 */
PORT_API bool_t realm_del(void* ptr, struct realm** ref, struct realm** res)
{
	COM_RETURN(ptr == 0 || ref == 0 || res == 0, 0);
	struct realm* node = ptr = (void*)((char*)ptr - sizeof(struct realm*));
	node->life = 0;
	node->ref = ref;
	*ref = node;
	if (node->next && node->next->life == 0)
	{
		node->size += node->next->size;
		realm_pop(res, node->next);
	}
	if (node->prev && node->prev->life == 0)
	{
		node->data = node->prev->data;
		node->size += node->prev->size;
		realm_pop(res, node->prev);
	}
	return 1;
}

/* 获取当前节点的内存大小 */
PORT_API qty_t realm_size(struct realm* node)
{
	COM_RETURN(node == 0, 0);
	return node->size;
}


#else


PORT_API bool_t realm_init(struct realm* realm, void* ptr, qty_t size, struct realm** ref) { ; }
PORT_API void* realm_new(struct realm* realm, qty_t size, struct realm** ref, struct realm** res) { ; }
PORT_API bool_t realm_del(void* ptr, struct realm** ref, struct realm** res) { ; }
PORT_API qty_t realm_size(struct realm* realm) { ; }


#endif