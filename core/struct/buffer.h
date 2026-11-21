#ifndef STRUCT_BUFFER_H
/* 缓冲区 */
#define STRUCT_BUFFER_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 缓冲区 */
struct buffer
{
	/* 是否为2的幂次方 */
	unsigned char pow;
	/* 是否允许覆盖过往信息 */
	unsigned char cover;
	/* 当前容量 */
	qty_t cap;
	/* 读偏移 */
	qty_t read;
	/* 写偏移 */
	qty_t write;
	/* 缓冲区区域 */
	char* buff;
	/* 当前未读取数量 */
	qty_t count;
};

/* 初始化缓冲区 设置缓冲区模式 默认无缓冲区区域 */
PORT_API struct buffer* buffer_init(void* ptr, qty_t cover);
/* 扩展缓冲区区域 */
PORT_API bool_t buffer_ext(struct buffer* buffer, void** old, char* buff, qty_t cap);
/* 清空缓冲区区域 */
PORT_API void buffer_clear(struct buffer* buffer);
/* 开启或关闭覆盖 不保证后续操作导致的数据安全 */
PORT_API bool_t buffer_cover(struct buffer* buffer, qty_t cover);

/* 压入一个单位数据 */
PORT_API bool_t buffer_push(struct buffer* buffer, unsigned char value);
/* 弹出一个单位数据到指定位置 */
PORT_API bool_t buffer_pop(struct buffer* buffer, unsigned char* value);
/* 写入一组数据 */
PORT_API bool_t buffer_write(struct buffer* buffer, const char* src, qty_t len);
/* 读取一组数据 返回实际读取单位 */
PORT_API qty_t buffer_read(struct buffer* buffer, char* dst, qty_t len);

/* 跳过一段可读数据 */
PORT_API bool_t buffer_skip(struct buffer* buffer, qty_t len);

/* 获取可读区域大小 */
PORT_API qty_t buffer_rsize(struct buffer* buffer);
/* 获取可写区域大小 */
PORT_API qty_t buffer_wsize(struct buffer* buffer);
/* 获取容量上限 */
PORT_API qty_t buffer_cap(struct buffer* buffer);

/* 获取当前最长的连续可读区域数据 不修改元数据 */
PORT_API void* buffer_data(struct buffer* buffer, qty_t* len);
/* 整理当前缓冲区区域 */
PORT_API bool_t buffer_flat(struct buffer* buffer);


EXTERN_C_END
#endif