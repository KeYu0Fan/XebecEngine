#include "list.h"
#if COM_USE(CONFIG_STRUCT_LIST)

/* 初始化链表头 void*用于消除显示转换 */
PORT_API struct list* list_init(void* ptr, qty_t off, qty_t loop)
{
	COM_RETURN(ptr == 0, 0);
	struct list* list = (struct list*)ptr;
	list->off = off;
	list->loop = loop;
	list->head = 0;
	list->count = 0;
	return list;
}
/* 清空链表头 不释放节点 */
PORT_API void list_clear(struct list* list)
{
	COM_RETURN(list == 0, );
	list->head = 0;
	list->count = 0;
}
/* 开启或关闭环路 开启需要遍历 关闭不需要 */
PORT_API bool_t list_loop(struct list* list, qty_t loop)
{
	COM_RETURN(list == 0, 0);
	loop = loop ? 1 : 0;
	if (list->loop == loop) return 1;
	if (list->head == 0)
	{
		list->loop = loop;
		return 1;
	}
	struct list_node* node = list->head;
	if (loop == 0)
	{
		node->prev->next = 0;
		node->prev = 0;
	}
	else
	{
		struct list_node* tmp = list_end(list, 0);
		node->prev = tmp;
		tmp->next = node;
	}
	list->loop = loop;
	return 1;
}

/* 压入一个链表节点 默认头插 back尾插 成功返回true 否则false */
PORT_API bool_t list_push(struct list* list, struct list_node* node, bool_t back)
{
	COM_RETURN(list == 0 || node == 0, 0);
	bool_t dir = back ? 1 : 0;
	struct list_node* tmp = 0;
	if (list->head)
	{
		if (dir == 0)
		{
			tmp = list->head;
			node->next = tmp;
			node->prev = tmp->prev;
			if (tmp->prev)
				tmp->prev->next = node;
			tmp->prev = node;
			list->head = node;
		}
		else
		{
			tmp = list_end(list, 0);
			node->prev = tmp;
			node->next = tmp->next;
			if (tmp->next)
				tmp->next->prev = node;
			tmp->next = node;
		}
		list->count++;
		return 1;
	}
	else
	{
		list->head = node;
		if (list->loop == 0)
		{
			node->prev = 0;
			node->next = 0;
		}
		else
		{
			node->prev = node;
			node->next = node;
		}
	}
	list->count++;
	return 1;
}
/* 弹出一个链表节点 默认头出 back尾出 成功返回true 否则false */
PORT_API struct list_node* list_pop(struct list* list, bool_t back)
{
	COM_RETURN(list == 0 || list->head == 0, 0);
	bool_t dir = back ? 1 : 0;
	struct list_node* tmp = 0;
	if (list->head->next && list->head != list->head->next)
	{
		if (dir == 0)
		{
			tmp = list->head;
			tmp->next->prev = tmp->prev;
			if (tmp->prev)
				tmp->prev->next = tmp->next;
			list->head = tmp->next;
		}
		else
		{
			tmp = list_end(list, 0);
			tmp->prev->next = tmp->next;
			if (tmp->next)
				tmp->next->prev = tmp->prev;
		}
		tmp->prev = 0;
		tmp->next = 0;
		list->count--;
		return tmp;
	}
	else
	{
		tmp = list->head;
		tmp->prev = 0;
		tmp->next = 0;
		list->head = 0;
		list->count = 0;
		return tmp;
	}
}
/* 将一个节点插入pos前后 默认back 否则front 成功返回true 否则false */
PORT_API bool_t list_insert(struct list* list, struct list_node* pos, struct list_node* node, bool_t front)
{
	COM_RETURN(list == 0 || list->head == 0 || pos == 0 || node == 0, 0);
	bool_t dir = front ? 0 : 1;
	struct list_node* tmp = 0;
	if (dir == 0)
	{
		tmp = pos->prev;
		if (tmp)
			tmp->next = node;
		pos->prev = node;
		node->next = pos;
		node->prev = tmp;
		if (pos == list->head) list->head = node;
	}
	else
	{
		tmp = pos->next;
		if (tmp)
			tmp->prev = node;
		pos->next = node;
		node->prev = pos;
		node->next = tmp;
	}
	list->count++;
	return 1;
}
/* 摘除任意节点 摘除节点清空节点 */
PORT_API bool_t list_remove(struct list* list, struct list_node* node)
{
	COM_RETURN(list == 0 || list->head == 0 || node == 0, 0);
	if (list->head->next && list->head != list->head->next)
	{
		if (node->prev && node->next)
		{
			if (node == list->head) list->head = node->next;
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		else
		{
			if (node->prev == 0)
			{
				list->head = node->next;
				node->next->prev = 0;
			}
			else
			{
				node->prev->next = 0;
			}
		}
		node->prev = 0;
		node->next = 0;
		list->count--;
		return 1;
	}
	else
	{
		if (node != list->head) return 0;
		node->prev = 0;
		node->next = 0;
		list->head = 0;
		list->count = 0;
		return 1;
	}
}

/* 获取节点 默认back 否则front */
PORT_API struct list_node* list_end(struct list* list, bool_t front)
{
	COM_RETURN(list == 0 || list->head == 0, 0);
	bool_t dir = front ? 0 : 1;
	struct list_node* ret = list->head;
	if(dir == 1)
	{
		if (list->loop) return ret->prev;
		while (ret->next)
		{
			ret = ret->next;
		}
	}
	return ret;
}
/*
* 获取相邻节点节点 默认back 否则front 遍历遇到head停止
* node为零时默认从head开始 步进回head时停止
* 无环逆序有隐式回绕行为
*/
PORT_API struct list_node* list_adj(struct list* list, struct list_node* node, bool_t front)
{
	COM_RETURN(list == 0 || list->head == 0, 0);
	bool_t dir = front ? 0 : 1;
	struct list_node* ret = 0;
	if (node == 0) return list->head;
	if (dir == 0)
		ret = node->prev;
	else
		ret = node->next;
	/* 用于支持head开头无环逆序 */
	if (ret == 0)
		ret = list_end(list, !front);
	if (ret != list->head)
		return ret;
	return 0;
}
/*
* 获取相邻节点节点 默认back 否则front 轮转循环
* 当为非环时遇到尽头返回另一端 当为尾部时需要遍历
* node为零时默认从head开始
*/
PORT_API struct list_node* list_for(struct list* list, struct list_node* node, bool_t front)
{
	COM_RETURN(list == 0 || list->head == 0, 0);
	bool_t dir = front ? 0 : 1;
	struct list_node* ret = 0;
	if (node == 0) return list->head;
	if (dir == 0)
	{
		if (node->prev)
			ret = node->prev;
		else
			ret = list_end(list, 0);
	}
	else
	{
		if (node->next)
			ret = node->next;
		else
			ret = list->head;
	}
	return ret;
}
/* 获取节点数量 */
PORT_API qty_t list_count(struct list* list)
{
	COM_RETURN(list == 0, 0);
	return list->count;
}
/* 判断是否不存在节点 */
PORT_API bool_t list_empty(struct list* list)
{
	COM_RETURN(list == 0, 0);
	return list->head == 0 ? 1 : 0;
}
/* 获取父节点 */
PORT_API void* list_parent(struct list* list, struct list_node* node)
{
	COM_RETURN(list == 0 || node == 0, 0);
	return (void*)((char*)node - list->off);
}

/* 查找第一个满足cmp的节点 返回节点指针 未找到返回0 */
PORT_API struct list_node* list_find(struct list* list, const void* value, cmp_t(*cmp)(void*, void*))
{
	COM_RETURN(list == 0 || list->head == 0 || value == 0 || cmp == 0, 0);
	qty_t count = list->count;
	struct list_node* node = list->head;
	while (count && node)
	{
		if(cmp(list_parent(list,node), value) == 0)
			return node;
		node = node->next;
		count--;
	}
	return 0;
}
/*
* 把other的所有节点拼接到list尾部 other变空
* off需要相同
* loop不同时会采用前者 后者会开启或关闭loop
*/
PORT_API bool_t list_concat(struct list* list, struct list* other)
{
	COM_RETURN(list == 0 || other == 0 || other->head == 0 || list->off != other->off, 0);
	if(list_loop(other, list->loop) == 0) return 0;
	if (list->head == 0)
	{
		list->head = other->head;
		list->count = other->count;
		other->head = 0;
		other->count = 0;
		return 1;
	}
	struct list_node* lptr = list_end(list, 0);
	struct list_node* optr = list_end(other, 0);
	lptr->next = other->head;
	other->head->prev = lptr;
	if(list->loop)
	{
		optr->next = list->head;
		list->head->prev = optr;
	}
	list->count += other->count;
	other->head = 0;
	other->count = 0;
	return 1;
}
/* 反转链表 */
PORT_API void list_rev(struct list* list)
{
	COM_RETURN(list == 0 || list->head == 0, );
	qty_t count = list->count;
	struct list_node* node = list->head;
	struct list_node* new_head = 0;
	while (count && node)
	{
		struct list_node* tmp = node->next;
		node->next = node->prev;
		node->prev = tmp;
		new_head = node;
		node = tmp;
		count--;
	}
	if (list->loop == 0)
		list->head = new_head;
}


#else


PORT_API struct list* list_init(void* ptr, qty_t off, qty_t loop){ ; }
PORT_API void list_clear(struct list* list){ ; }
PORT_API bool_t list_loop(struct list* list, qty_t loop){ ; }
PORT_API bool_t list_push(struct list* list, struct list_node* node, bool_t back){ ; }
PORT_API struct list_node* list_pop(struct list* list, bool_t back){ ; }
PORT_API bool_t list_insert(struct list* list, struct list_node* pos, struct list_node* node, bool_t front){ ; }
PORT_API bool_t list_remove(struct list* list, struct list_node* node){ ; }
PORT_API struct list_node* list_end(struct list* list, bool_t front){ ; }
PORT_API struct list_node* list_adj(struct list* list, struct list_node* node, bool_t front){ ; }
PORT_API struct list_node* list_for(struct list* list, struct list_node* node, bool_t front){ ; }
PORT_API qty_t list_count(struct list* list){ ; }
PORT_API bool_t list_empty(struct list* list){ ; }
PORT_API void* list_parent(struct list* list, struct list_node* node){ ; }
PORT_API void* list_find(struct list* list, const void* value, cmp_t(*cmp)(void*, void*)){ ; }
PORT_API bool_t list_concat(struct list* list, struct list* other) { ; }
PORT_API void list_rev(struct list* list){ ; }


#endif