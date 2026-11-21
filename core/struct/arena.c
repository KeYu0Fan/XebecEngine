#include "arena.h"
#if COM_USE(CONFIG_STRUCT_ARENA)


PORT_HIDE static void arena_node(struct arena_bump* bump, qty_t count, char* data, qty_t size)
{
	qty_t i = 0;
	while (i < count)
	{
		if (i)
			bump[i].prev = &bump[i - 1];
		if (i + 1 < count)
			bump[i].next = &bump[i + 1];
		bump[i].data = data;
		bump[i].epoch = 0;
		bump[i].life = 0;
		bump[i].used = 0;
		data += size;
		i++;
	}
	bump[0].prev = &bump[count - 1];
	bump[count - 1].next = &bump[0];
}
/* 初始化内存管理 ptr内存 cap大小 size段大小 最少容纳一个段才能创建 */
PORT_API struct arena* arena_init(void* ptr, qty_t cap, qty_t size)
{
	size = (size + sizeof(qty_t) - 1) & ~(sizeof(qty_t) - 1);
	COM_RETURN(ptr == 0 || cap == 0 || size == 0, 0);
	if (cap < sizeof(struct arena) + sizeof(struct arena_bump) + size) return 0;
	qty_t count = (cap - sizeof(struct arena)) / (sizeof(struct arena_bump) + size);
	struct arena* arena = (struct arena*)((char*)ptr);
	struct arena_bump* new_bump = (struct arena_bump*)((char*)ptr + sizeof(struct arena));
	char* data = ((char*)ptr + sizeof(struct arena) + sizeof(struct arena_bump) * count);

	arena->cap = cap;
	arena->size = size;
	arena->count = count;
	arena->epoch = 0;
	arena->bump = new_bump;
	arena->shift = 0;
	if ((size & (size - 1)) == 0)
	{
		qty_t i = 1;
		qty_t j = 0;
		while (i < size)
		{
			i = i << 1;
			j++;
		}
		arena->shift = j;
	}
	arena->stat.curt = size;
	arena->stat.peak = size;
	arena->stat.total = size;
	arena->stat.live = 0;

	arena_node(new_bump, count, data, size);
	
	return arena;
}
/* 清空内存管理区域 */
PORT_API void arena_clear(struct arena* arena)
{
	COM_RETURN(arena == 0, );
	arena->epoch++;
	arena->bump->epoch = arena->epoch;
	arena->bump->used = 0;
	arena->bump->life = 0;

	arena->stat.curt = arena->size;
	arena->stat.live = 0;
}

/* 分配内存 */
PORT_API void* arena_new(struct arena* arena, qty_t size)
{
	size = (size + sizeof(qty_t) - 1) & ~(sizeof(qty_t) - 1);
	COM_RETURN(arena == 0 || size == 0 || size > arena->size, 0);
	if (arena->bump->used + size > arena->size)
	{
		struct arena_bump* next_bump = arena->bump->next;
		if (next_bump->epoch == arena->epoch && next_bump->life)
			return 0;
		if (next_bump->epoch != arena->epoch)
		{
			next_bump->life = 0;
			next_bump->used = 0;
			next_bump->epoch = arena->epoch;
		}
		arena->bump = next_bump;
		arena->stat.curt += arena->size;
		if (arena->stat.curt > arena->stat.peak)
			arena->stat.peak = arena->stat.curt;
		arena->stat.total += arena->size;
	}
	void* ret = arena->bump->data + arena->bump->used;
	arena->bump->used += size;
	arena->bump->life++;
	arena->stat.live++;
	return ret;
}
PORT_HIDE static qty_t arena_align(struct arena_bump* bump, qty_t align)
{
	qty_t abs = (qty_t)bump->data + bump->used;
	return ((0 - abs) & (align - 1));
}
/* 分配特殊对齐内存 */
PORT_API void* arena_anew(struct arena* arena, qty_t size, qty_t align)
{
	size = (size + sizeof(qty_t) - 1) & ~(sizeof(qty_t) - 1);
	COM_RETURN(arena == 0 || size == 0 || (align & (align - 1)) != 0, 0);
	if (align <= sizeof(qty_t)) return arena_new(arena, size);
	qty_t new_used = (arena->bump->used + arena_align(arena->bump, align));
	if (new_used + size > arena->size)
	{
		struct arena_bump* next_bump = arena->bump->next;
		if (next_bump->epoch == arena->epoch && next_bump->life)
			return 0;
		if (next_bump->epoch != arena->epoch)
		{
			next_bump->life = 0;
			next_bump->used = 0;
			next_bump->epoch = arena->epoch;
		}
		new_used = (next_bump->used + arena_align(next_bump, align));
		if (new_used + size > arena->size) return 0;
		arena->bump = next_bump;
		arena->stat.curt += arena->size;
		if (arena->stat.curt > arena->stat.peak)
			arena->stat.peak = arena->stat.curt;
		arena->stat.total += arena->size;
	}
	void* ret = arena->bump->data + new_used;
	arena->bump->used = new_used + size;
	arena->bump->life++;
	arena->stat.live++;
	return ret;
}
/* 释放内存 */
PORT_API void arena_del(struct arena* arena, void* ptr)
{
	COM_RETURN(arena == 0 || ptr == 0, );
	struct arena_bump* bump = 0;
	{
		qty_t offset = (qty_t)((char*)ptr - ((char*)arena + sizeof(struct arena) + sizeof(struct arena_bump) * arena->count));
		qty_t idx = 0;
		if (arena->shift != 0)
			idx = offset >> arena->shift;
		else
			idx = offset / arena->size;
		if (idx >= arena->count) return;
		bump = &((struct arena_bump*)((char*)arena + sizeof(struct arena)))[idx];
	}
	if (bump->epoch != arena->epoch || bump->life == 0)
		return;
	bump->life--;
	arena->stat.live--;
	if (bump->life == 0)
	{
		bump->used = 0;
		if (bump != arena->bump)
			arena->stat.curt -= arena->size;
		else
			return;
		bump->next->prev = bump->prev;
		bump->prev->next = bump->next;
		bump->next = arena->bump->next;
		bump->prev = arena->bump;
		arena->bump->next->prev = bump;
		arena->bump->next = bump;
	}
}

/* 获取当前 */
PORT_API qty_t arena_curt(struct arena* arena)
{
	COM_RETURN(arena == 0, 0);
	return arena->stat.curt;
}
/* 获取峰值 */
PORT_API qty_t arena_peak(struct arena* arena)
{
	COM_RETURN(arena == 0, 0);
	return arena->stat.peak;
}
/* 获取累计 */
PORT_API qty_t arena_total(struct arena* arena)
{
	COM_RETURN(arena == 0, 0);
	return arena->stat.total;
}
/* 获取总存活数 */
PORT_API qty_t arena_live(struct arena* arena)
{
	COM_RETURN(arena == 0, 0);
	return arena->stat.live;
}
/* 获取内存管理可能大小 */
PORT_API qty_t arena_size(qty_t size, qty_t count)
{
	COM_RETURN(size == 0 || count == 0, 0);
	size = (size + sizeof(qty_t) - 1) & ~(sizeof(qty_t) - 1);
	return sizeof(struct arena) + (sizeof(struct arena_bump) + size) * count;
}


#else


PORT_API struct arena* arena_init(void* ptr, qty_t cap, qty_t size) { ; }
PORT_API void arena_clear(struct arena* arena) { ; }
PORT_API void* arena_new(struct arena* arena, qty_t size) { ; }
PORT_API void* arena_anew(struct arena* arena, qty_t size, qty_t align) { ; }
PORT_API void arena_del(struct arena* arena, void* ptr) { ; }
PORT_API qty_t arena_curt(struct arena* arena) { ; }
PORT_API qty_t arena_peak(struct arena* arena) { ; }
PORT_API qty_t arena_total(struct arena* arena) { ; }
PORT_API qty_t arena_live(struct arena* arena) { ; }
PORT_API qty_t arena_size(qty_t size, qty_t count) { ; }


#endif