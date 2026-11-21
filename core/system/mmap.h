#ifndef SYSTEM_MMAP_H
/* ----内存映射----
*
* 请确保操作为页对齐操作
* system_mmap_new
* huge_tlb 需要权限 请确保拥有
* populate 忽略 非阻塞 情况
* windows 不支持shared=1且huge_tlb=1 linux支持
* windows populate失败不会返回失败 linux会返回
* fixed Linux 需要 4.17+ / glibc 2.28+ 旧内核返回失败
* system_mmap_del
* 调用者必须传入原始基址 传入偏移地址将导致失败
* 全部释放需要原大小
* system_mmap_resize
* windows
* 不支持 1允许移动 2固定新地址 文件映射调整
* 支持 4保留旧映射 不释放
* linux
* flags 4 保留旧映射 需要 Linux 5.7+ / glibc 2.32+
* 旧内核传入 flags 4 会返回失败
* system_mmap_sync
* windows 异步/无效化无害返回成功
* system_mmap_advise
* FREE 在不支持的系统上静默成功 不保证实际生效
* windows
* 需要 Win8.1+ DiscardVirtualMemory 找不到函数时返回成功
* system_mmap_file
* windows 文件映射不支持大页huge_tlb
* Windows populate == 2 忽略 没有非阻塞映射
* fixed 地址冲突时失败
*
----内存映射---- */
#define SYSTEM_MMAP_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


/* 内存权限 */
struct system_mmap_prot_t
{
	/* 读 */
	char read; 
	/* 写 */
	char write;
	/* 执行 */
	char exec;
	/* 无 */
	char none;
};
/* 内存标志 */
struct system_mmap_flags_t
{
	/* 0私有映射 修改不写回文件 写时复制 1共享映射 修改写回文件 */
	char shared;
	/* 0系统决定 1安全固定地址 */
	char fixed;
	/* 0无 1使用大页映射 高性能场景 */
	char huge_tlb;
	/* 0默认 1预读 2非阻塞 */
	char populate;
};

enum system_mmap_advise_value
{
	/* 默认 */
	SYSTEM_MMAP_ADVISE_NORMAL = 0,
	/* 随机访问 */
	SYSTEM_MMAP_ADVISE_RANDOM = 1,
	/* 顺序访问 */
	SYSTEM_MMAP_ADVISE_SEQUENTIAL = 2,
	/* 预加载 */
	SYSTEM_MMAP_ADVISE_WILLNEED = 3,
	/* 释放不再使用的缓冲区 */
	SYSTEM_MMAP_ADVISE_DONTNEED = 4,
	/* 系统紧张时可回收 */
	SYSTEM_MMAP_ADVISE_FREE = 8
};

/* 分配内存 prot权限 flags标志位 */
PORT_API void* system_mmap_new(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags);
/* 释放内存 */
PORT_API bool_t system_mmap_del(void* ptr, qty_t size);
/* 修改内存权限 */
PORT_API bool_t system_mmap_protect(void* ptr, qty_t size, struct system_mmap_prot_t prot);
/* 调整内存大小 flags == 1允许移动 2固定新地址|返回新地址 4保留旧映射 允许叠加flags */
PORT_API void* system_mmap_resize(void* ptr, qty_t old_size, qty_t new_size, qty_t flags);
/* 内存文件映射写回 flags == 1无效化缓存 2同步写回 4异步写回 允许叠加1 */
PORT_API bool_t system_mmap_sync(void* ptr, qty_t size, qty_t flags);
/* 内存加锁 */
PORT_API bool_t system_mmap_lock(void* ptr, qty_t size);
/* 内存解锁 */
PORT_API bool_t system_mmap_unlock(void* ptr, qty_t size);
/* 设置内存的策略行为 advice建议类型 */
PORT_API bool_t system_mmap_advise(void* ptr, qty_t size, enum system_mmap_advise_value advice);
/* 检查内存的策略行为 vec == (size + page_size - 1) / page_size */
PORT_API bool_t system_mmap_residency(void* ptr, qty_t size, unsigned char* vec);
/* 映射文件 fd句柄 offset偏移 */
PORT_API void* system_mmap_file(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags, \
	system_handle_h fd, system_off_t offset);


EXTERN_C_END
#endif