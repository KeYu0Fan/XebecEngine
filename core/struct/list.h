#ifndef STRUCT_LIST_H
/*
* 链表
* 参数 front back
* 当值为true时方向为参数名
* 当值为false时方向为相反方向
*/
#define STRUCT_LIST_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN

/* 侵入式链表节点 */
struct list_node
{
	/* 上一个 */
	struct list_node* prev;
	/* 下一个 */
	struct list_node* next;
};
/* 链表头 */
struct list
{
	/*
	* 环路 false无 true有
	* 无环路时
	* head必指向头节点且修改head指向非头的操作会失败
	* 有环路时
	* head指向这个环且head可以指向环的任意节点
	*/
	qty_t loop;
	/* 侵入式节点在父体中的偏移 */
	qty_t off;
	/* 侵入式链表节点头指针 */
	struct list_node* head;
	/* 当前数量 */
	qty_t count;
};

/* 初始化链表头 void*用于消除显示转换 */
PORT_API struct list* list_init(void* ptr, qty_t off, qty_t loop);
/* 清空链表头 不释放节点 */
PORT_API void list_clear(struct list* list);
/* 开启或关闭环路 开启需要遍历 关闭不需要 */
PORT_API bool_t list_loop(struct list* list, qty_t loop);

/* 压入一个链表节点 默认头插 back尾插 成功返回true 否则false */
PORT_API bool_t list_push(struct list* list, struct list_node* node, bool_t back);
/* 弹出一个链表节点 默认头出 back尾出 成功返回true 否则false */
PORT_API struct list_node* list_pop(struct list* list, bool_t back);
/* 将一个节点插入pos前后 默认back 否则front 成功返回true 否则false */
PORT_API bool_t list_insert(struct list* list, struct list_node* pos, struct list_node* node, bool_t front);
/* 摘除任意节点 摘除节点清空节点 */
PORT_API bool_t list_remove(struct list* list, struct list_node* node);

/* 获取节点 默认back 否则front */
PORT_API struct list_node* list_end(struct list* list, bool_t front);
/*
* 获取相邻节点节点 默认back 否则front 遍历遇到head停止
* node为零时默认从head开始 步进时下一个为head停止
* 无环逆序有隐式回绕行为
*/
PORT_API struct list_node* list_adj(struct list* list, struct list_node* node, bool_t front);
/*
* 获取相邻节点节点 默认back 否则front 轮转循环
* 当为非环时遇到尽头返回另一端 当为尾部时需要遍历
* node为零时默认从head开始
*/
PORT_API struct list_node* list_for(struct list* list, struct list_node* node, bool_t front);
/* 获取节点数量 */
PORT_API qty_t list_count(struct list* list);
/* 判断是否不存在节点 */
PORT_API bool_t list_empty(struct list* list);
/* 获取父节点 */
PORT_API void* list_parent(struct list* list, struct list_node* node);

/* 查找第一个满足cmp的节点 返回节点指针 未找到返回0 */
PORT_API struct list_node* list_find(struct list* list, const void* value, cmp_t(*cmp)(void*, void*));
/*
* 把other的所有节点拼接到list尾部 other变空
* off需要相同 other不能为空
* loop不同时会采用前者 后者会开启或关闭loop
*/
PORT_API bool_t list_concat(struct list* list, struct list* other);
/* 反转链表 */
PORT_API void list_rev(struct list* list);


EXTERN_C_END
#endif