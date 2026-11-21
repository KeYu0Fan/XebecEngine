#include "mem.h"
#if COM_USE(CONFIG_CORE_MEM)


/* 复制内存块 返回目标内存块 源不能覆盖目标 如果覆盖关闭COMMON_RESTRICT优化 */
PORT_API void* mem_cpy(void* PLATFORM_RESTRICT dst, const void* PLATFORM_RESTRICT src, qty_t size)
{
	COM_RETURN(dst == 0 || src == 0 || size == 0, 0);
#if (COM_PREF)
	void* ret = dst;
	qty_t cmp = ((qty_t)dst & (sizeof(qty_t) - 1));
	cmp = cmp ? sizeof(qty_t) - cmp : 0;
	qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
	if (m)
	{
		unsigned char* d = dst;
		const unsigned char* s = src;
		qty_t n = m;
		while (n >= sizeof(char))
		{

			*d = *s;
			d++;
			s++;
			n--;
		}
		size = size - m;
		if (size == 0) return ret;
		dst = d;
		src = s;
#if	(PLATFORM_ALIGN)
		cmp = ((qty_t)src & (sizeof(qty_t) - 1));
#else
		cmp = 0;
#endif
	}
	if (cmp == 0)
	{
		qty_t* d = dst;
		const qty_t* s = src;
		while (size >= sizeof(qty_t))
		{
			*d = *s;
			d++;
			s++;
			size = size - sizeof(qty_t);
		}
		dst = d;
		src = s;
	}
	else
	{
		qty_t off = sizeof(qty_t) - ((qty_t)src & (sizeof(qty_t) - 1));
		qty_t offl = off * COM_BYTE_BIT;
		qty_t offh = (sizeof(qty_t) - off) * COM_BYTE_BIT;
		qty_t* d = dst;
		const qty_t* s = (const qty_t*)((const char*)src - off);
		qty_t tmp = 0;
		{
			qty_t i = off;
			while (i < sizeof(qty_t))
			{
				tmp = (tmp << COM_BYTE_BIT) | ((const unsigned char*)s)[i];
				i++;
			}
		}
		while (size >= sizeof(qty_t))
		{
			s++;
			qty_t a = *s;
			*d = (tmp << offh) | (a >> offl);
			d++;
			tmp = a;
			size -= sizeof(qty_t);
		}
		dst = d;
		src = ((const unsigned char*)s + off);
	}
	if (size)
	{
		unsigned char* d = dst;
		const unsigned char* s = src;
		while (size >= sizeof(char))
		{
			*d = *s;
			d++;
			s++;
			size = size - sizeof(char);
		}
	}
	return ret;
#elif (COM_STD)
	return memcpy(dst, src, (size_t)size);
#else
	return 0;
#endif
}
/* 复制内存块 返回目标内存块 源与目标允许重叠 */
PORT_API void* mem_mov(void* dst, const void* src, qty_t size)
{
	COM_RETURN(dst == 0 || src == 0 || size == 0, 0);
#if (COM_PREF)
	if (dst < src ? 1 : (void*)((const char*)src + size) < dst ? 1 : 0)
	{
		return mem_cpy(dst, src, size);
	}
	else
	{
		void* ret = dst;
		dst = (unsigned char*)dst + size;
		src = (const unsigned char*)src + size;
		qty_t cmp = ((qty_t)dst & (sizeof(qty_t) - 1));
		qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
		if (m)
		{

			unsigned char* d = dst;
			const unsigned char* s = src;
			qty_t n = m;
			while (n >= sizeof(char))
			{
				d--;
				s--;
				n--;
				*d = *s;
			}
			size = size - m;
			if (size == 0) return ret;
			dst = d;
			src = s;
#if	(PLATFORM_ALIGN)
			cmp = ((qty_t)src & (sizeof(qty_t) - 1));
#else
			cmp = 0;
#endif
		}
		if (cmp == 0)
		{
			qty_t* d = dst;
			const qty_t* s = src;
			while (size >= sizeof(qty_t))
			{
				d--;
				s--;
				size = size - sizeof(qty_t);
				*d = *s;
			}
			dst = d;
			src = s;
		}
		else
		{
			qty_t off = ((qty_t)src & (sizeof(qty_t) - 1));
			qty_t offl = off * COM_BYTE_BIT;
			qty_t offh = (sizeof(qty_t) - off) * COM_BYTE_BIT;
			qty_t* d = dst;
			const qty_t* s = (const qty_t*)((const char*)src - off);
			qty_t tmp = 0;
			{
				qty_t i = 0;
				while (i < off)
				{
					tmp = (tmp << COM_BYTE_BIT) | ((const unsigned char*)s)[i];
					i++;
				}
				tmp <<= offh;
			}
			while (size >= sizeof(qty_t))
			{
				s--;
				qty_t a = *s;
				*d = (tmp >> offl) | (a << offh);
				d--;
				tmp = a;
				size -= sizeof(qty_t);
			}
			dst = d;
			src = ((const unsigned char*)s + off);
		}
		if (size)
		{
			unsigned char* d = dst;
			const unsigned char* s = src;
			while (size >= sizeof(char))
			{
				d--;
				s--;
				size = size - sizeof(char);
				*d = *s;
			}
		}
		return ret;
	}
#elif (COM_STD)
	return memmove(dst, src, (size_t)size);
#else
	return 0;
#endif
}
/* 将目标内存块设置为字节源 设置大小为size */
PORT_API void* mem_set(void* dst, unsigned char value, qty_t size)
{
	COM_RETURN(dst == 0 || size == 0, 0);
#if (COM_PREF)
	void* ret = dst;
	qty_t cmp = ((qty_t)dst) & (sizeof(qty_t) - 1);
	cmp = cmp ? sizeof(qty_t) - cmp : 0;
	qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
	if (m)
	{
		char* d = dst;
		qty_t n = m;
		while (n)
		{
			*d = value;
			d++;
			n--;
		}
		size = size - m;
		if (size == 0) return ret;
		dst = d;
		cmp = 0;
	}
	if (cmp == 0)
	{
		qty_t* d = dst;
		qty_t v = COM_REPEAT_PATTERN(value, qty_t, COM_BYTE_REPEAT);
		while (size >= sizeof(qty_t))
		{
			*d = v;
			d++;
			size = size - sizeof(qty_t);
		}
		dst = d;
	}
	if (size)
	{
		char* d = dst;
		while (size >= sizeof(char))
		{
			*d = value;
			d++;
			size = size - sizeof(char);
		}
	}
	return ret;
#elif (COM_STD)
	return memset(dst, (int)value, (size_t)size);
#else
	return 0;
#endif
}
/* 比较内存块 返回字节的目标与源之差 */
PORT_API cmp_t mem_cmp(const void* dst, const void* src, qty_t size)
{
	COM_RETURN(dst == 0 || src == 0 || size == 0, 0);
#if (COM_PREF)
	qty_t cmp = ((qty_t)dst & (sizeof(qty_t) - 1));
	cmp = cmp ? sizeof(qty_t) - cmp : 0;
	qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
	if (m)
	{
		const unsigned char* d = dst;
		const unsigned char* s = src;
		qty_t n = m;
		while (n >= sizeof(char) && *d == *s)
		{
			d++;
			s++;
			n--;
		}
		if (n != 0) return (*d - *s);
		size = size - m;
		if (size == 0) return 0;
		dst = d;
		src = s;
#if	(PLATFORM_ALIGN)
		cmp = ((qty_t)src & (sizeof(qty_t) - 1));
#else
		cmp = 0;
#endif
	}
	if (cmp == 0)
	{
		const qty_t* d = dst;
		const qty_t* s = src;
		while (size >= sizeof(qty_t) && *d == *s)
		{
			d++;
			s++;
			size = size - sizeof(qty_t);
		}
		dst = d;
		src = s;
	}
	else
	{
		qty_t off = sizeof(qty_t) - ((qty_t)src & (sizeof(qty_t) - 1));
		qty_t offl = off * COM_BYTE_BIT;
		qty_t offh = (sizeof(qty_t) - off) * COM_BYTE_BIT;
		const qty_t* d = dst;
		const qty_t* s = (const qty_t*)((const char*)src - off);
		qty_t tmp = 0;
		{
			qty_t i = off;
			while (i < sizeof(qty_t))
			{
				tmp = (tmp << COM_BYTE_BIT) | ((const unsigned char*)s)[i];
				i++;
			}
		}
		while (size >= sizeof(qty_t))
		{
			s++;
			qty_t a = *s;
			if (*d != ((tmp << offl) | (a >> offh)))
			{
				s--;
				break;
			}
			d++;
			tmp = a;
			size -= sizeof(qty_t);
		}
		dst = d;
		src = (void*)((const unsigned char*)s + off);
	}
	if (size)
	{
		const unsigned char* d = dst;
		const unsigned char* s = src;
		while (size >= sizeof(char) && *d == *s)
		{
			d++;
			s++;
			size = size - sizeof(char);
		}
		if (size == 0) return 0;
		return (*d - *s);
	}
	return 0;
#elif (COM_STD)
	return memcmp(dst, src, (size_t)size);
#else
	return 0;
#endif
}
/* 在目标中查找字节源 范围为size */
PORT_API void* mem_chr(const void* dst, unsigned char value, qty_t size)
{
	COM_RETURN(dst == 0 || size == 0, 0);
#if (COM_PREF)
	void* ret = 0;
	qty_t cmp = ((qty_t)dst) & (sizeof(qty_t) - 1);
	cmp = cmp ? sizeof(qty_t) - cmp : 0;
	qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
	if (m)
	{
		const unsigned char* d = dst;
		qty_t n = m;
		while (n >= sizeof(char) && *d != value)
		{
			d++;
			n--;
		}
		if (n != 0) return (void*)d;
		size = size - m;
		if (size == 0) return ret;
		dst = d;
		cmp = 0;
	}
	if (cmp == 0)
	{
		qty_t high = COM_REPEAT_PATTERN(COM_BYTE_HIGH, qty_t, COM_BYTE_REPEAT);
		qty_t low = COM_REPEAT_PATTERN(COM_BYTE_LOW, qty_t, COM_BYTE_REPEAT);
		qty_t src = value * low;
		const qty_t* d = dst;
		while (size >= sizeof(qty_t))
		{
			if ((((*d ^ src) - low) & ~(*d ^ src)) & high)
			{
				const unsigned char* a = (const unsigned char*)d;
				qty_t i = 0;
				while (i < sizeof(qty_t))
				{
					if (*a == value) return (void*)a;
					a++;
					i++;
				}
			}
			d++;
			size = size - sizeof(qty_t);
		}
		dst = d;
	}
	if (size)
	{
		const unsigned char* d = dst;
		while (size >= sizeof(char) && *d != value)
		{
			d++;
			size = size - sizeof(char);
		}
		ret = (void*)d;
		if (size == 0) return 0;
		return ret;
	}
	return 0;
#elif (COM_STD)
	return memchr(dst, (int)value, (size_t)size);
#else
	return 0;
#endif
}


#else


PORT_API void* mem_cpy(void* PLATFORM_RESTRICT dst, const void* PLATFORM_RESTRICT src, qty_t size) { ; }
PORT_API void* mem_mov(void* dst, const void* src, qty_t size) { ; }
PORT_API void* mem_set(void* dst, unsigned char value, qty_t size) { ; }
PORT_API cmp_t mem_cmp(const void* dst, const void* src, qty_t size) { ; }
PORT_API void* mem_chr(const void* dst, unsigned char value, qty_t size) { ; }


#endif