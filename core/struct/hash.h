#ifndef STRUCT_HASH_H
/* 哈希表 */
#define STRUCT_HASH_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 侵入式哈希节点 */
struct hash_node
{
	/* 上一个 */
	struct hash_node* prev;
	/* 下一个 */
	struct hash_node* next;
};
/* 哈希策略函数表 */
struct hash_ops
{
	/* 生成哈希值 */
	qty_t(*key)(void* key);
	/* 通过父体获取哈希 */
	void*(*get)(void* host);
	/* 判断哈希是否等于哈希 */
	cmp_t(*cmp)(void* key1, void* key2);
};
/* 哈希表头 */
struct hash
{
	/* 是否为2的幂次方 */
	qty_t pow;
	/* 侵入式节点在父体中的偏移 */
	qty_t off;
	/* 哈希表的指针容量*/
	qty_t cap;
	/* 哈希元素指针的指针 */
	struct hash_node** data;
	/* 当前元素数量 */
	qty_t count;
	/* 哈希函数表 */
	struct hash_ops ops;
};

/* 初始化哈希表头 ptr为表头 默认无表 */
PORT_API struct hash* hash_init(void* ptr, qty_t off, struct hash_ops* ops);
/* 扩展hash表元素容量 */
PORT_API bool_t hash_ext(struct hash* hash,void** old, void* data, qty_t cap);
/* 清空哈希表头 不释放节点 */
PORT_API void hash_clear(struct hash* hash);

/* 插入一个哈希元素 */
PORT_API bool_t hash_insert(struct hash* hash, struct hash_node* node);
/* 删除一个哈希元素 */
PORT_API bool_t hash_remove(struct hash* hash, void* key);

/* 查找哈希表元素 返回父体 */
PORT_API void* hash_find(struct hash* hash, void* key);

/* 获取哈希元素数量 */
PORT_API qty_t hash_count(struct hash* hash);
/* 是否不存在哈希元素 */
PORT_API bool_t hash_empty(struct hash* hash);

/*
* 遍历hash表的每一个有效元素
* call_main回调 用于自定义操作 当返回值不为零时返回
* 参数1 用于承载user_data数据
* 参数2 用于获取有效元素父体
* 返回true 正常退出或提前退出 返回fasle 失败
*/
PORT_API bool_t hash_adj(struct hash* hash, bool_t(*call_main)(void* user_data,void* ptr), void* user_data);
/* 宏版本 用法: HASH_FOREACH(&my_hash, host, struct my_type) { ... } */
#define HASH_FOREACH(hash, host, type) \
    for (qty_t _i = 0; _i < (hash)->cap; _i++) \
        for (struct hash_node *_n = (hash)->data[_i], *_next = (_n ? _n->next : 0); \
             _n && (((host) = (type*)((char*)_n - (hash)->off)), 1); \
             _n = _next, _next = (_n ? _n->next : 0))


EXTERN_C_END
#endif