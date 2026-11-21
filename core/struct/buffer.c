#include "buffer.h"
#if COM_USE(CONFIG_STRUCT_BUFFER)


#include "core/mem.h"

PORT_HIDE static qty_t buffer_idx(struct buffer* buffer, qty_t off)
{
	return buffer->pow ? (off & (buffer->cap - 1)) : (off % buffer->cap);
}

/* 初始化缓冲区 设置缓冲区模式 默认无缓冲区区域 */
PORT_API struct buffer* buffer_init(void* ptr, qty_t cover)
{
	COM_RETURN(ptr == 0, 0);
	struct buffer* buffer = (struct buffer*)ptr;
	buffer->pow = 0;
	buffer->cover = (unsigned char)cover;
	buffer->read = 0;
	buffer->write = 0;
	buffer->count = 0;
	buffer->cap = 0;
	buffer->buff = 0;
	return buffer;
}
/* 扩展缓冲区区域 */
PORT_API bool_t buffer_ext(struct buffer* buffer, void** old, char* buff, qty_t cap)
{
	COM_RETURN(buffer == 0 || buff == 0 || cap == 0, 0);
	qty_t pow = cap & (cap - 1);
	qty_t size = buffer_rsize(buffer);
	size = size > cap ? size - cap : 0;
	if (size && buffer->cover == 0)
		return 0;
	if (size) buffer_skip(buffer, size);
	qty_t write = buffer_read(buffer, buff, cap);
	if (old)
		*old = buffer->buff;
	buffer->cap = cap;
	buffer->buff = buff;
	buffer->read = 0;
	buffer->write = write;
	buffer->count = write;
	buffer->pow = pow ? 0 : 1;
	return 1;
}
/* 清空缓冲区区域 */
PORT_API void buffer_clear(struct buffer* buffer)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || buffer->cap == 0, );
	buffer->read = 0;
	buffer->write = 0;
	buffer->count = 0;
}
/* 开启或关闭覆盖 不保证后续操作导致的数据安全 */
PORT_API bool_t buffer_cover(struct buffer* buffer, qty_t cover)
{
	COM_RETURN(buffer == 0, 0);
	buffer->cover = (unsigned char)cover;
	return 1;
}

/* 压入一个单位数据 */
PORT_API bool_t buffer_push(struct buffer* buffer, unsigned char value)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || buffer->cap == 0, 0);
	if (buffer->count == buffer->cap)
	{
		if (buffer->cover == 0)
			return 0;
		buffer->read = buffer_idx(buffer, buffer->read + 1);
	}
	else
		buffer->count++;
	buffer->buff[buffer->write] = value;
	buffer->write = buffer_idx(buffer, buffer->write + 1);
	return 1;
}
/* 弹出一个单位数据到指定位置 */
PORT_API bool_t buffer_pop(struct buffer* buffer, unsigned char* value)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || buffer->count == 0 || value == 0, 0);
	*value = buffer->buff[buffer->read];
	buffer->read = buffer_idx(buffer, buffer->read + 1);
	buffer->count--;
	return 1;
}
/* 写入一组数据 */
PORT_API bool_t buffer_write(struct buffer* buffer, const char* src, qty_t len)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || src == 0 || len == 0, 0);
	qty_t wsize = buffer_wsize(buffer);
	if (len > wsize && buffer->cover == 0) return 0;
	if (len >= buffer->cap)
	{
		mem_mov((void*)buffer->buff, (void*)(src + (len - buffer->cap)), buffer->cap);
		buffer->write = 0;
		buffer->read = 0;
		buffer->count = buffer->cap;
		return 1;
	}
	qty_t rpart = buffer->write + len > buffer->cap ? buffer->cap - buffer->write : len;
	qty_t lpart = len - rpart;
	mem_mov((void*)&buffer->buff[buffer->write], (void*)src, rpart);
	if (lpart)
		mem_mov((void*)buffer->buff, (void*)(src + rpart), lpart);
	buffer->read = buffer_idx(buffer, buffer->read + (len > wsize ? len - wsize : 0));
	buffer->write = buffer_idx(buffer, buffer->write + len);
	if (len < wsize)
		buffer->count = buffer->count + len;
	else
		buffer->count = buffer->cap;
	return 1;
}
/* 读取一组数据 返回实际读取单位 */
PORT_API qty_t buffer_read(struct buffer* buffer, char* dst, qty_t len)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || buffer->count == 0 || dst == 0 || len == 0, 0);
	if (len > buffer->count) len = buffer->count;
	qty_t part = buffer->read + len > buffer->cap ? buffer->cap - buffer->read : len;
	mem_mov((void*)dst, (void*)&buffer->buff[buffer->read], part);
	if(len - part)
		mem_mov((void*)(dst + part), (void*)buffer->buff, len - part);
	buffer->read = buffer_idx(buffer, buffer->read + len);
	buffer->count = buffer->count - len;
	return len;
}

/* 跳过一段可读数据 */
PORT_API bool_t buffer_skip(struct buffer* buffer, qty_t len)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || (len > buffer->count) || len == 0, 0);
	buffer->read = buffer_idx(buffer, buffer->read + len);
	buffer->count = buffer->count - len;
	return 1;
}

/* 获取可读区域大小 */
PORT_API qty_t buffer_rsize(struct buffer* buffer)
{
	COM_RETURN(buffer == 0, 0);
	return buffer->count;
}
/* 获取可写区域大小 */
PORT_API qty_t buffer_wsize(struct buffer* buffer)
{
	COM_RETURN(buffer == 0, 0);
	return buffer->cap - buffer->count;
}
/* 获取容量上限 */
PORT_API qty_t buffer_cap(struct buffer* buffer)
{
	COM_RETURN(buffer == 0, 0);
	return buffer->cap;
}

/* 获取当前最长的连续可读区域数据 不修改元数据 */
PORT_API void* buffer_data(struct buffer* buffer, qty_t* len)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0 || buffer->count == 0 || len == 0, 0);
	if (buffer->read + buffer->count <= buffer->cap)
		*len = buffer->count;
	else
		*len = buffer->cap - buffer->read;
	return &buffer->buff[buffer->read];
}
PORT_HIDE static void buffer_rev(char* head, char* tail)
{
	char tmp = 0;
	while (head < tail)
	{
		tmp = *head;
		*head = *tail;
		*tail = tmp;
		head++;
		tail--;
	}
}
/*
PORT_HIDE static void buffer_swap(char* p1, char* p2, qty_t size)
{
	char tmp = 0;
	while (size)
	{
		tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2++;
		size--;
	}
}
*/
/* 整理当前缓冲区区域 */
PORT_API bool_t buffer_flat(struct buffer* buffer)
{
	COM_RETURN(buffer == 0 || buffer->buff == 0, 0);
	if (buffer->count == 0 || buffer->read == 0) return 1;
	char* buff = buffer->buff;
	qty_t loff = 0;
	qty_t roff = buffer->read;
	qty_t rpart = buffer->cap - buffer->read;
	qty_t lpart = buffer->count > rpart ? buffer->count - rpart : 0;;
	qty_t wsize = buffer_wsize(buffer);
	if (buffer->read + buffer->count <= buffer->cap)
	{
		mem_mov((void*)buff, (void*)&buff[roff], buffer->count);
	}
	else if (rpart < lpart ? (wsize >= rpart) : (wsize >= lpart))
	{
		if (rpart < lpart)
		{
			mem_mov((void*)&buff[rpart], (void*)&buff[loff], lpart);
			mem_mov((void*)buff, (void*)&buff[roff], rpart);
		}
		else
		{
			mem_mov((void*)&buff[roff - lpart], (void*)&buff[roff], rpart);
			mem_mov((void*)&buff[roff + rpart - lpart], (void*)buff, lpart);
			mem_mov((void*)buff, (void*)&buff[roff - lpart], rpart + lpart);
		}
	}
	else
	{
		buffer_rev(buff, buff + roff - 1);
		buffer_rev(buff + roff, buff + roff + rpart - 1);
		buffer_rev(buff, buff + roff + rpart - 1);
	}
	buffer->read = 0;
	buffer->write = buffer->count;
	return 1;
}


#else


PORT_API struct buffer* buffer_init(void* ptr, qty_t cover) { ; }
PORT_API bool_t buffer_ext(struct buffer* buffer, void** old, char* buff, qty_t cap) { ; }
PORT_API void buffer_clear(struct buffer* buffer) { ; }
PORT_API bool_t buffer_cover(struct buffer* buffer, qty_t cover) { ; }
PORT_API bool_t buffer_push(struct buffer* buffer, unsigned char value) { ; }
PORT_API bool_t buffer_pop(struct buffer* buffer, unsigned char* value) { ; }
PORT_API bool_t buffer_write(struct buffer* buffer, const char* src, qty_t len) { ; }
PORT_API qty_t buffer_read(struct buffer* buffer, char* dst, qty_t len) { ; }
PORT_API bool_t buffer_skip(struct buffer* buffer, qty_t len) { ; }
PORT_API qty_t buffer_rsize(struct buffer* buffer) { ; }
PORT_API qty_t buffer_wsize(struct buffer* buffer) { ; }
PORT_API qty_t buffer_cap(struct buffer* buffer) { ; }
PORT_API void* buffer_data(struct buffer* buffer, qty_t* len) { ; }
PORT_API bool_t buffer_flat(struct buffer* buffer) { ; }


#endif