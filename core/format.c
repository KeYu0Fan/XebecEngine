#include "format.h"
#if COM_USE(CONFIG_CORE_FORMAT)


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/mem.h"

PORT_HIDE static bool_t format_copy(void* buff, qty_t bsize, const char* src, qty_t len)
{
	if (len + 1 > bsize) return 0;
	if (len) mem_cpy(buff, src, len);
	((char*)buff)[len] = '\0';
	return 1;
}

PORT_HIDE static bool_t format_u64_dec(char* out, qty_t cap, u64_t v)
{
	char tmp[24];
	qty_t n = 0;
	qty_t i = 0;
	do
	{
		tmp[n++] = (char)('0' + (v % 10));
		v /= 10;
	} while (v);
	if (n + 1 > cap) return 0;
	while (n) out[i++] = tmp[--n];
	out[i] = '\0';
	return 1;
}

PORT_HIDE static bool_t format_s64_dec(char* out, qty_t cap, s64_t v)
{
	char tmp[24];
	qty_t n = 0;
	qty_t i = 0;
	bool_t neg = v < 0;
	u64_t mag;
	if (neg)
		mag = (u64_t)(-(v + 1)) + 1;
	else
		mag = (u64_t)v;
	do
	{
		tmp[n++] = (char)('0' + (mag % 10));
		mag /= 10;
	} while (mag);
	if (n + (neg ? 1 : 0) + 1 > cap) return 0;
	if (neg) out[i++] = '-';
	while (n) out[i++] = tmp[--n];
	out[i] = '\0';
	return 1;
}

PORT_HIDE static bool_t format_float_text(void* buff, qty_t bsize, double v, bool_t single)
{
	char tmp[64];
	int maxp = single ? 9 : 17;
	int p = 1;
	qty_t len = 0;
	if (isnan(v)) return format_copy(buff, bsize, "nan", 3);
	if (isinf(v))
		return format_copy(buff, bsize, signbit(v) ? "-inf" : "inf", signbit(v) ? 4 : 3);
	for (; p <= maxp; p++)
	{
		int n = snprintf(tmp, sizeof(tmp), "%.*g", p, v);
		if (n < 0 || n >= (int)sizeof(tmp)) return 0;
		if (single)
		{
			if ((float)strtod(tmp, 0) == (float)v) break;
		}
		else
		{
			if (strtod(tmp, 0) == v) break;
		}
	}
	/* 统一小数点, 避免依赖 locale 的逗号输出 */
	while (tmp[len])
	{
		if (tmp[len] == ',') tmp[len] = '.';
		len++;
	}
	return format_copy(buff, bsize, tmp, len);
}

PORT_API bool_t format_dtos(void* buff, qty_t bsize, const void* value, qty_t vsize)
{
	COM_RETURN(buff == 0 || bsize == 0 || value == 0 || vsize == 0, 0);
	switch (vsize)
	{
	case 1:
	{
		s8_t v;
		mem_cpy(&v, value, 1);
		return format_s64_dec((char*)buff, bsize, (s64_t)v);
	}
	case 2:
	{
		s16_t v;
		mem_cpy(&v, value, 2);
		return format_s64_dec((char*)buff, bsize, (s64_t)v);
	}
	case 4:
	{
		s32_t v;
		mem_cpy(&v, value, 4);
		return format_s64_dec((char*)buff, bsize, (s64_t)v);
	}
	case 8:
	{
		s64_t v;
		mem_cpy(&v, value, 8);
		return format_s64_dec((char*)buff, bsize, v);
	}
	default:
		return 0;
	}
}

PORT_API bool_t format_utos(void* buff, qty_t bsize, const void* value, qty_t vsize)
{
	COM_RETURN(buff == 0 || bsize == 0 || value == 0 || vsize == 0, 0);
	switch (vsize)
	{
	case 1:
	{
		u8_t v;
		mem_cpy(&v, value, 1);
		return format_u64_dec((char*)buff, bsize, (u64_t)v);
	}
	case 2:
	{
		u16_t v;
		mem_cpy(&v, value, 2);
		return format_u64_dec((char*)buff, bsize, (u64_t)v);
	}
	case 4:
	{
		u32_t v;
		mem_cpy(&v, value, 4);
		return format_u64_dec((char*)buff, bsize, (u64_t)v);
	}
	case 8:
	{
		u64_t v;
		mem_cpy(&v, value, 8);
		return format_u64_dec((char*)buff, bsize, v);
	}
	default:
		return 0;
	}
}

PORT_API bool_t format_ftos(void* buff, qty_t bsize, const void* value, qty_t vsize)
{
	COM_RETURN(buff == 0 || bsize == 0 || value == 0 || vsize == 0, 0);
	switch (vsize)
	{
	case 4:
	{
		f32_t v;
		mem_cpy(&v, value, 4);
		return format_float_text(buff, bsize, (double)v, 1);
	}
	case 8:
	{
		f64_t v;
		mem_cpy(&v, value, 8);
		return format_float_text(buff, bsize, (double)v, 0);
	}
	default:
		return 0;
	}
}

PORT_API bool_t format_mtos(void* buff, qty_t bsize, const void* value, qty_t vsize)
{
	static const char hex[] = "0123456789abcdef";
	qty_t i = 0;
	COM_RETURN(buff == 0 || bsize == 0 || value == 0 || vsize == 0, 0);
	if (vsize > (bsize - 1) / 2) return 0;
	for (; i < vsize; i++)
	{
		unsigned char v = ((const unsigned char*)value)[i];
		((char*)buff)[i * 2] = hex[v >> 4];
		((char*)buff)[i * 2 + 1] = hex[v & 0x0F];
	}
	((char*)buff)[vsize * 2] = '\0';
	return 1;
}


#else


PORT_API bool_t format_dtos(void* buff, qty_t bsize, const void* value, qty_t vsize) { ; }
PORT_API bool_t format_utos(void* buff, qty_t bsize, const void* value, qty_t vsize) { ; }
PORT_API bool_t format_ftos(void* buff, qty_t bsize, const void* value, qty_t vsize) { ; }
PORT_API bool_t format_mtos(void* buff, qty_t bsize, const void* value, qty_t vsize) { ; }


#endif