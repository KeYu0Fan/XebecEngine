#ifndef STRUCT_REALM_H
/* 
* 内存管理 realm
*/
#define STRUCT_REALM_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN

/* 内存管理 realm */
struct realm
{
	/* 上一个 */
	struct realm* prev;
	/* 下一个 */
	struct realm* next;
	/* 引用方 */
	struct realm** ref;
	/* 大小 */
	qty_t size;
	/* 是否使用 */
	qty_t life;
	/* 内存 */
	char* data;
};

/* 初始化内存管理 将一段内存以更详细安全的方式管理 空闲内存节点必须存在引用方 */
PORT_API bool_t realm_init(struct realm* realm, void* ptr, qty_t size, struct realm** ref);
/* 分配内存 为剩余内存节点添加引用方 无剩余内存节点时引用放置空 res为外部提供的节点站 */
PORT_API void* realm_new(struct realm* realm, qty_t size, struct realm** ref, struct realm** res);
/* 释放内存 为释放的节点添加引用方 合并的节点回收并去除引用方 */
PORT_API bool_t realm_del(void* ptr, struct realm** ref, struct realm** res);

/* 获取当前节点的内存大小 */
PORT_API qty_t realm_size(struct realm* realm);


EXTERN_C_END
#endif