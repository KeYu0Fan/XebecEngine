#ifndef MORY_MORY_H
/* ----内存----
* 申请释放操作内存
----内存---- */
#define MORY_MORY_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/mem.h"
#include "core/system/thread.h"
#include "core/system/mmap.h"
#include "core/system/time.h"
EXTERN_C_BEGIN

/* 初始化内部 设置空闲多少时长真释放 是否需要锁保证安全 注入分配器时由注入分配器决定 */
PORT_API void mory_init(void*(*i_malloc)(qty_t size), void(*i_free)(void* ptr), qty_t time_ms, bool_t lock);
/* 无注入时可查询当前持有信息 */
PORT_API bool_t mory_has(qty_t* curt, qty_t* free);
/* 分配内存 */
PORT_API void* mory_new(qty_t size);
/* 释放内存 */
PORT_API void mory_del(void* ptr);


EXTERN_C_END
#endif