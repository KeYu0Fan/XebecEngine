#ifndef STRUCT_ARENA_H
/* 
* 内存管理 arena
* 初始化时占用一池 统计数据略微偏大
*/
#define STRUCT_ARENA_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 内存管理单元 段节点 */
struct arena_bump
{
	/* 上一个 */
	struct arena_bump* prev;
	/* 下一个 */
	struct arena_bump* next;
	/* 代际 */
	qty_t epoch;
	/* 使用偏移 */
	qty_t used;
	/* 存活数量 */
	qty_t life;
	/* 段 */
	char* data;
};
/* 内存管理 统计 */
struct arena_stat
{
	/* 当前字节数 */
	qty_t curt;
	/* 峰值字节数 */
	qty_t peak;
	/* 累计字节数 */
	qty_t total;
	/* 总存活数 */
	qty_t live;
};
/* 内存管理 arena */
struct arena
{
	/* 内存管理总大小 */
	qty_t cap;
	/* 段数量 */
	qty_t count;
	/* 段大小 */
	qty_t size;
	/* 代际 */
	qty_t epoch;
	/* 可能左移 */
	qty_t shift;
	/* 使用中与未使用的中间段 */
	struct arena_bump* bump;
	/* 信息*/
	struct arena_stat  stat;
};

/* 初始化内存管理 ptr内存 cap大小 size段大小 最少容纳一个段才能创建 */
PORT_API struct arena* arena_init(void* ptr, qty_t cap, qty_t size);
/* 清空内存管理区域 */
PORT_API void arena_clear(struct arena* arena);

/* 分配内存 */
PORT_API void* arena_new(struct arena* arena, qty_t size);
/* 分配特殊对齐内存 */
PORT_API void* arena_anew(struct arena* arena, qty_t size, qty_t align);
/* 释放内存 */
PORT_API void arena_del(struct arena* arena, void* ptr);

/* 获取当前 */
PORT_API qty_t arena_curt(struct arena* arena);
/* 获取峰值 */
PORT_API qty_t arena_peak(struct arena* arena);
/* 获取累计 */
PORT_API qty_t arena_total(struct arena* arena);
/* 获取总存活数 */
PORT_API qty_t arena_live(struct arena* arena);
/* 获取剩余可分配 inner决定是否需要当前池的剩余 */
PORT_API qty_t arena_remain(struct arena* arena, qty_t inner);
/* 获取内存管理 arena 可能大小 */
PORT_API qty_t arena_size(qty_t size, qty_t count);


EXTERN_C_END
#endif