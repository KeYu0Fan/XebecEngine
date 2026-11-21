#include "hash.h"
#if COM_USE(CONFIG_STRUCT_HASH)


#include "core/mem.h"

PORT_HIDE static qty_t hash_idx(struct hash* hash, qty_t key)
{
	return hash->pow ? (key & (hash->cap - 1)) : (key % hash->cap);
}

/* 初始化哈希表头 ptr为表头 默认无表 */
PORT_API struct hash* hash_init(void* ptr, qty_t off, struct hash_ops* ops)
{
	COM_RETURN(ptr == 0 || ops == 0, 0);
	if ((ops->cmp && ops->get && ops->key) == 0) return 0;
	struct hash* hash = (struct hash*)ptr;
	hash->off = off;
	hash->pow = 0;
	hash->cap = 0;
	hash->data = 0;
	hash->count = 0;
	hash->ops = *ops;
	return hash;
}
PORT_HIDE static bool_t hash_ext_set(void* user_data, void* data)
{
	struct hash* hash = (struct hash*)user_data;
	hash_insert(hash, (struct hash_node*)((char*)data + hash->off));
	return 0;
}
/* 扩展hash表元素容量 */
PORT_API bool_t hash_ext(struct hash* hash, void** old, void* data, qty_t cap)
{
	COM_RETURN(hash == 0 || data == 0 || cap == 0, 0);
	qty_t pow = cap & (cap - 1);
	mem_set(data, 0, sizeof(void*) * cap);
	if (old)
		*old = hash->data;
	if (hash->cap)
	{
		/* 保存旧哈希 */
		struct hash oldhash = { 0 };
		oldhash = *hash;
		/* 使用新哈希 */
		hash->data = data;
		hash->cap = cap;
		hash->count = 0;
		hash->pow = pow ? 0 : 1;
		/* 重新挂载节点 */
		hash_adj(&oldhash, hash_ext_set, hash);
		return 1;
	}
	else
	{
		hash->data = data;
		hash->cap = cap;
		hash->count = 0;
		hash->pow = pow ? 0 : 1;
		return 1;
	}
}
/* 清空哈希表头 不释放节点 */
PORT_API void hash_clear(struct hash* hash)
{
	COM_RETURN(hash == 0 || hash->data == 0, );
	hash->count = 0;
	mem_set(hash->data, 0, sizeof(void*) * hash->cap);
}

/* 插入一个哈希元素 */
PORT_API bool_t hash_insert(struct hash* hash, struct hash_node* node)
{
	COM_RETURN(hash == 0 || hash->data == 0 || node == 0, 0);
	struct hash_node** idx = 0;
	{
		void* parent = (void*)((char*)node - hash->off);
		qty_t key = hash->ops.key(hash->ops.get(parent));
		key = hash_idx(hash, key);
		idx = &hash->data[key];
	}
	node->prev = 0;
	node->next = 0;
	if (*idx)
	{
		node->next = *idx;
		(*idx)->prev = node;
	}
	*idx = node;
	hash->count++;
	return 1;
}
/* 删除一个哈希元素 */
PORT_API bool_t hash_remove(struct hash* hash, void* key)
{
	COM_RETURN(hash == 0 || hash->data == 0 || key == 0, 0);
	struct hash_node* node = 0;
	struct hash_node** idx = 0;
	void* parent = 0;
	{
		qty_t tmp = hash->ops.key(key);
		tmp = hash_idx(hash, tmp);
		idx = &hash->data[tmp];
		if (*idx == 0) return 0;
		node = *idx;
	}
	while (node)
	{
		parent = (void*)((char*)node - hash->off);
		void* pkey = hash->ops.get(parent);
		if (hash->ops.cmp(pkey, key) == 0)
		{
			if (node->prev && node->next)
			{
				node->prev->next = node->next;
				node->next->prev = node->prev;
			}
			else if (node->prev == 0)
			{
				*idx = node->next;
				if (node->next) node->next->prev = 0;
			}
			else
			{
				node->prev->next = 0;
			}
			node->prev = 0;
			node->next = 0;
			hash->count--;
			return 1;
		}
		node = node->next;
	}
	return 0;
}

/* 查找哈希表元素 返回父体 */
PORT_API void* hash_find(struct hash* hash, void* key)
{
	COM_RETURN(hash == 0 || hash->data == 0 || key == 0, 0);
	struct hash_node* node = 0;
	void* parent = 0;
	{
		struct hash_node* idx = 0;
		qty_t tmp = hash->ops.key(key);
		tmp = hash_idx(hash, tmp);
		idx = hash->data[tmp];
		if (idx == 0) return 0;
		node = idx;
	}
	while (node)
	{
		parent = (void*)((char*)node - hash->off);
		void* pkey = hash->ops.get(parent);
		if (hash->ops.cmp(pkey, key) == 0)
			return parent;
		node = node->next;
	}
	return 0;
}

/* 获取哈希元素数量 */
PORT_API qty_t hash_count(struct hash* hash)
{
	COM_RETURN(hash == 0 || hash->data == 0, 0);
	return hash->count;
}
/* 是否不存在哈希元素 */
PORT_API bool_t hash_empty(struct hash* hash)
{
	COM_RETURN(hash == 0, 0);
	return (hash->data == 0 || hash->count == 0) ? 1 : 0;
}

/*
* 遍历hash表的每一个有效元素
* call_main回调 用于自定义操作 当返回值不为零时返回
* 参数1 用于承载user_data数据
* 参数2 用于获取有效元素父体
* 返回true 正常退出或提前退出 返回fasle 失败
*/
PORT_API bool_t hash_adj(struct hash* hash, bool_t(*call_main)(void* user_data, void* ptr), void* user_data)
{
	COM_RETURN(hash == 0 || call_main == 0, 0);
	if (hash->cap == 0 || hash->data == 0) return 1;
	qty_t i = 0;
	while (i < hash->cap)
	{
		struct hash_node* n = 0;
		struct hash_node* next = 0;
		n = hash->data[i];
		next = n ? n->next : 0;
		while (n)
		{
			struct hash_node* tmp = n;
			n = next;
			next = n ? n->next : 0;
			if (call_main(user_data, (void*)((char*)tmp - hash->off)) != 0)
				return 1;
		}
		i++;
	}
	return 1;
}


#else


PORT_API struct hash* hash_init(void* ptr, qty_t off, struct hash_ops* ops) { ; }
PORT_API bool_t hash_ext(struct hash* hash, void** old, void* data, qty_t cap) { ; }
PORT_API void hash_clear(struct hash* hash) { ; }
PORT_API bool_t hash_insert(struct hash* hash, struct hash_node* node) { ; }
PORT_API bool_t hash_remove(struct hash* hash, void* key) { ; }
PORT_API void* hash_find(struct hash* hash, void* key) { ; }
PORT_API qty_t hash_count(struct hash* hash) { ; }
PORT_API bool_t hash_empty(struct hash* hash) { ; }
PORT_API bool_t hash_adj(struct hash* hash, bool_t(*call_main)(void*, void*), void* user_data) { ; }


#endif