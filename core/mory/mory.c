#include "mory.h"
#ifdef COM_USE(CONFIG_MORY_MORY)


PORT_HIDE static bool_t Minit = 0;
PORT_HIDE static qty_t Mtime = 0;
PORT_HIDE static bool_t Mlock = 0;
PORT_HIDE static system_mutex_t Mmutex = { 0 };

PORT_HIDE static void* (*Mmalloc)(qty_t size) = 0;
PORT_HIDE static void(*Mfree)(void* ptr) = 0;

PORT_HIDE static Scurt = 0;
PORT_HIDE static Sfree = 0;

#include "stdlib.h"

/* 初始化内部 设置空闲多少时长真释放 是否需要锁保证安全 注入分配器时由注入分配器决定 */
PORT_API void mory_init(void* (*i_malloc)(qty_t size), void(*i_free)(void* ptr), qty_t time_ms, bool_t lock)
{
	if (Minit) return;
	Mmalloc = i_malloc ? i_malloc : malloc;
	Mfree = i_free ? i_free : free;
	Mlock = lock;
	Mtime = time_ms;
	Minit = 1;
}
/* 无注入时可查询当前持有信息 */
PORT_API bool_t mory_has(qty_t* curt, qty_t* free)
{
	if (Minit == 0) return 0;
	*curt = Scurt;
	*free = Sfree;
	return 1;
}
/* 分配内存 */
PORT_API void* mory_new(qty_t size)
{
	if (Mmalloc == 0) return 0;
	return Mmalloc(size);
}
/* 释放内存 */
PORT_API void mory_del(void* ptr)
{
	if (Mfree == 0) return;
	Mfree(ptr);
}


#else


PORT_API void mory_init(void* (*i_malloc)(qty_t size), void(*i_free)(void* ptr), qty_t time_ms, bool_t lock) { ; }
PORT_API bool_t mory_has(qty_t* curt, qty_t* free) { ; }
PORT_API void* mory_new(qty_t size) { ; }
PORT_API void mory_del(void* ptr) { ; }


#endif