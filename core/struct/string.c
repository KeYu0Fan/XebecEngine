#include "string.h"
#if COM_USE(CONFIG_STRUCT_STRING)


#define STRUCT_STRING_SIZE (((unsigned char)-1) + 1)

PORT_HIDE static qty_t high = COM_REPEAT_PATTERN(COM_BYTE_HIGH, qty_t, COM_BYTE_REPEAT);
PORT_HIDE static qty_t low = COM_REPEAT_PATTERN(COM_BYTE_LOW, qty_t, COM_BYTE_REPEAT);

/* 判断机器字中是否存在 '\0' 字节 */
#define STR_HAS_ZERO(x) (((x) - low) & ~(x) & high)

PORT_HIDE static void SetArr(char dst[STRUCT_STRING_SIZE], const char* src)
{
	COM_RETURN(dst == 0 || src == 0, );
	char value = *src;
	while (value)
	{
		if (value)
		{
			dst[(unsigned char)value] = 1;
		}
		src++;
		value = *src;
	}
}
PORT_HIDE static qty_t GetOff(const char* dst, qty_t dir)
{
	if (dir == 0)
	{
		return (~(qty_t)dst + 1) & (sizeof(qty_t) - 1);
	}
	else
	{
		return (qty_t)dst & (sizeof(qty_t) - 1);
	}
}

/* 返回字符串的长度 不包括结尾的'\0' 最多检查n个字节 */
PORT_API qty_t str_len(const char* dst, qty_t n)
{
	COM_RETURN(dst == 0 || n == 0, 0);
	/* 对齐 */
	qty_t size = n < COM_SMALL_SIZE ? n : GetOff(dst, 0);
	if (size)
	{
		qty_t i = 0;
		while (*dst && i < size)
		{
			dst++;
			i++;
		}
		if(i == n || *dst == 0) return i;
	}
	/* 优化 */
	if (n - size >= sizeof(qty_t))
	{
		qty_t i = size;
		qty_t* ptr = (qty_t*)dst;
		while (n - i >= sizeof(qty_t))
		{
			qty_t value = *ptr;
			if ((((value - low) & (~value)) & high))
				break;
			ptr = ptr + 1;
			i = i + sizeof(qty_t);
		}
		if (i != size)
		{
			dst = (const char*)(ptr - 1);
			size = i - sizeof(qty_t);
		}
	}
	/* 末尾处理 */
	if(size < n)
	{
		qty_t i = size;
		while (*dst && i < n)
		{
			dst++;
			i++;
		}
		return i;
	}
	return size;
}
/* 只比较前n个字符或直到出现'\0' 返回目标与源之差 */
PORT_API cmp_t str_cmp(const char* dst, const char* src, qty_t n)
{
	COM_RETURN(dst == 0 || src == 0 || n == 0, 0);
	/* 对齐 */
	qty_t size = n < COM_SMALL_SIZE ? n : GetOff(dst, 0);
	if (size)
	{
		qty_t i = 0;
		while (i < size && *dst == *src)
		{
			if (*dst == '\0') return (cmp_t)0;
			dst++;
			src++;
			i++;
		}
		if (i == n) return (cmp_t)0;
	}
	/* 优化 */
	if (n - size >= sizeof(qty_t))
	{
		qty_t i = size;
		qty_t off = GetOff(src, 1);
		qty_t* D = (qty_t*)dst;
		qty_t* S = (qty_t*)src;
		if ((PLATFORM_ALIGN && off) == 0)
		{
			while (n - i >= sizeof(qty_t) && *D == *S && STR_HAS_ZERO(*D) == 0)
			{
				D++;
				S++;
				i = i + sizeof(qty_t);
			}
		}
		else
		{
			S = (qty_t*)((char*)S - off);
			qty_t value = *S++;
			qty_t L = off * COM_BYTE_BIT;
			qty_t R = sizeof(qty_t) * COM_BYTE_BIT - L;
			while (n - i >= sizeof(qty_t) && *D == (value << L | *S >> R) && STR_HAS_ZERO(value << L | *S >> R) == 0)
			{
				value = *S;
				D++;
				S++;
				i = i + sizeof(qty_t);
			}
			S--;
			S = (qty_t*)((char*)S + off);
		}
		if (i != size)
		{
			dst = (const char*)(D - 1);
			src = (const char*)(S - 1);
			size = i - sizeof(qty_t);
		}
	}
	/* 末尾处理 */
	if (size < n)
	{
		qty_t i = size;
		while (i < n && *dst == *src)
		{
			if (*dst == '\0') return (cmp_t)0;
			dst++;
			src++;
			i++;
		}
		if(i == n)return (cmp_t)0;
	}
	return (cmp_t)(*dst - *src);
}

/*
* 复制src到dst 并保证在最终字符串末尾以'\0'结尾
* n为复制长度 n包括'\0' 剩余保持原样
*/
PORT_API char* str_cpy(char* dst, const char* src, qty_t n)
{
	COM_RETURN(dst == 0 || src == 0 || n == 0, 0);
	char* ret = dst;
	/* 对齐 */
	qty_t size = n < COM_SMALL_SIZE ? n : GetOff(dst, 0);
	if (size)
	{
		qty_t i = 0;
		while (i < size)
		{
			*dst = *src;
			dst++;
			src++;
			i++;
		}
		if (i == n)
		{
			dst--;
			*dst = '\0';
			return ret;
		}
	}
	/* 优化 */
	if (n - size >= sizeof(qty_t))
	{
		qty_t i = size;
		qty_t off = GetOff(src, 1);
		qty_t* D = (qty_t*)dst;
		qty_t* S = (qty_t*)src;
		if ((PLATFORM_ALIGN && off) == 0)
		{
			while (n - i >= sizeof(qty_t))
			{
				*D = *S;
				D++;
				S++;
				i = i + sizeof(qty_t);
			}
		}
		else
		{
			S = (qty_t*)((char*)S - off);
			qty_t value = *S++;
			qty_t L = off * COM_BYTE_BIT;
			qty_t R = sizeof(qty_t) * COM_BYTE_BIT - L;
			while (n - i >= sizeof(qty_t))
			{
				qty_t tmp = *S;
				*D = (value << L | tmp >> R);
				value = tmp;
				D++;
				S++;
				i = i + sizeof(qty_t);
			}
			S--;
			S = (qty_t*)((char*)S + off);
		}
		if (i != size)
		{
			dst = (char*)(D - 1);
			src = (const char*)(S - 1);
			size = i - sizeof(qty_t);
		}
	}
	/* 末尾处理 */
	if (size < n)
	{
		qty_t i = size;
		while (i < n)
		{
			*dst = *src;
			dst++;
			src++;
			i++;
		}
		dst--;
		*dst = '\0';
		return ret;
	}
	return ret;
}
/*
* src追加到dst末尾 并保证在最终字符串末尾添加'\0'
* n为追加长度 n包括'\0'
*/
PORT_API char* str_cat(char* dst, const char* src, qty_t n)
{
	COM_RETURN(dst == 0 || src == 0 || n == 0, 0);
	dst = dst + str_len(dst, (qty_t)-1);
	return str_cpy(dst, src, n);
}

/*
* 在字符串dst中查找src中任意一个字符第一次出现的位置 返回指向该位置的指针 若未找到则返回0
* dir为查询方向 0为正向 1为反向
*/
PORT_API char* str_chr(const char* dst, const char* src, qty_t dir)
{
	COM_RETURN(dst == 0 || src == 0, 0);
	char* ret = 0;
	char arr[STRUCT_STRING_SIZE] = { 0 };
	SetArr(arr, src);
	if (dir == 0)
	{
		while (*dst)
		{
			if (arr[(unsigned char)*dst])
				break;
			dst++;
		}
		if (*dst) ret = (char*)dst;
	}
	else
	{
		qty_t n = str_len(dst, (qty_t)-1);
		if (n == 0) return 0;
		dst =dst + n - 1;
		while (n)
		{
			if (arr[(unsigned char)*dst])
				break;
			n--;
			dst--;
		}
		if (n) ret = (char*)dst;
	}
	return ret;
}
/*
* 在字符串dst中查找子串src第一次出现的位置 若src为空字符串则返回dst 若未找到则返回0
* n为子串src的查询长度
*/
PORT_API char* str_str(const char* dst, const char* src, qty_t n)
{
	COM_RETURN(dst == 0 || src == 0, 0);
	qty_t srclen = 0;
	qty_t dstlen = 0;
	/* 限制边界 */
	{
		if (*src == 0) return (char*)dst;
		if (n == 0) return (char*)dst;
		srclen = str_len(src, n);
		if (srclen == 1)
		{
			char arr[] = { *src, 0 };
			return str_chr(dst, (const char*)&arr, 0);
		}
		dstlen = str_len(dst, (qty_t)-1);
		if (srclen > dstlen) return 0;
	}
	/* 主体逻辑 */
	qty_t slen = srclen;
	qty_t dlen = dstlen;
	if (dlen < slen) return 0;
	if (slen <= 4)
	{
		dlen = dlen - slen + 1;
		qty_t i = 0;
		while (i < dlen)
		{
			qty_t j = 0;
			while (j < slen)
			{
				if (dst[j] != src[j])
					break;
				j++;
			}
			if (j == slen)
				return (char*)dst;
			i++;
			dst++;
		}
		return 0;
	}
	if (slen <= (((unsigned char)-1) + 1))
	{
		dlen = dlen - slen + 1;
		/* 创建 初始化 坏字符表 */
		unsigned char arr[(((unsigned char)-1) + 1)] = { 0 };
		{
			qty_t tmp = 2;
			const char* s = src;
			/* 循环slen - 2次 */
			while (tmp < slen)
			{
				/* 使用双字节异或值做为判断 */
				unsigned char a = (unsigned char)s[0] ^ (unsigned char)s[1];
				arr[a] = (unsigned char)(slen - tmp);
				tmp++;
				s++;
			}
		}
		qty_t i = 0;
		while (i < dlen)
		{
			/* 循环判断 */
			qty_t cmp = i;
			qty_t tmp = slen;
			while (tmp >= sizeof(char))
			{
				/* 计算 偏移 目标字符值 */
				qty_t off = tmp - 1;
				unsigned char offd = dst[off];
				if (offd != (unsigned char)src[off])
				{
					qty_t a = arr[(unsigned char)dst[slen - 1] ^ (unsigned char)dst[slen - 2]];
					if (a == 0)
						a = slen;
					dst += a;
					i += a;
					break;
				}
				tmp--;
			}
			if (cmp == i)
				return (char*)dst;
		}
		return 0;
	}
	else
	{
		qty_t plen = dlen;
		qty_t m = slen;
		/* 计算跳转周期 */
		qty_t l = 0;
		qty_t p = 0;
		{
			/* l 最大后缀 */
			{
				qty_t len = m;
				const char* s = src;
				signed char order = 1;
				while (order >= 0)
				{
					qty_t i = 0, j = 1, k = 0;
					while (j + k < len)
					{
						unsigned char a = s[i + k];
						unsigned char b = s[j + k];
						if (a == b)
							k++;
						else if (order ? a < b : a > b)
						{
							j += k + 1;
							k = 0;
						}
						else
						{
							i = j;
							j++;
							k = 0;
						}
					}
					if (l < i)l = i;
					order--;
				}
			}
			/* p 周期计算 最小为1 */
			{
				qty_t len = m - l;
				const char* s = src + l;
				p = 1;
				while (p < len && s[p] == s[p - 1])
					p++;
			}
		}
		const unsigned char* pos = (const unsigned char*)dst;
		const unsigned char* end = pos + plen - m;
		while (pos <= end)
		{
			/* 匹配右半部分 */
			sqty_t j = (sqty_t)(m - sizeof(char));
			while (j >= (sqty_t)l && pos[j] == (unsigned char)src[j])
				j--;
			if (j >= (sqty_t)l)
			{
				pos++;
				continue;
			}
			/* 匹配左半部分 */
			qty_t i = 0;
			while (i < l && pos[i] == (unsigned char)src[i])
				i++;
			if (i == l) return (char*)pos;
			pos += p;
		}
		return 0;
	}
}
/*
* 计算dst中是否包含src中任意字符的初始段长度 若未找到则返回0
* is为包含关系 0为包含 1为不包含
*/
PORT_API qty_t str_span(const char* dst, const char* src, qty_t is)
{
	COM_RETURN(dst == 0 || src == 0, 0);
	qty_t ret = 0;
	char arr[STRUCT_STRING_SIZE] = { 0 };
	SetArr(arr, src);
	if (is == 0)
	{
		while (*dst)
		{
			if (arr[(unsigned char)*dst])
				ret++;
			else
				break;
			dst++;
		}
	}
	else
	{
		while (*dst)
		{
			if (!arr[(unsigned char)*dst])
				ret++;
			else
				break;
			dst++;
		}
	}
	return ret;
}

/*
* 传入dst目标字符串和tok分隔符字符串 不修改 返回token的长度
* 每个分隔符都是独立边界 连续分隔符会产出长度为0的token
* dst是指向目标字符串指针的指针 会将其更新为返回token之后的起始地址
* table为tok是否为表用于跳过内部创建 0为不是 1为是
* 若没有分隔符 则返回剩余字符串长度 并将dst置为0
*/
PORT_API qty_t str_tok(const char** dst, const char* tok, qty_t table)
{
	COM_RETURN(dst == 0 || *dst == 0 || tok == 0, 0);
	qty_t ret = 0;
	char* arr = 0;
	char tmp[STRUCT_STRING_SIZE] = { 0 };
	if (table == 0)
	{
		arr = tmp;
		SetArr(tmp, tok);
	}
	else
	{
		arr = (char*)tok;
	}
	const char* dst2 = *dst;
	while (*dst2)
	{
		if (arr[(unsigned char)*dst2])
		{
			ret = dst2 - *dst;
			dst2++;
			*dst = dst2;
			return ret;
		}
		dst2++;
	}
	ret = dst2 - *dst;
	*dst = 0;
	return ret;
}
/*
* 反转并修改字符串头尾顺序 返回字符串地址
* n为反转长度 反转长度不会超过'\0'
*/
PORT_API char* str_rev(char* dst, qty_t n)
{
	COM_RETURN(dst == 0 || n == 0, 0);
	if (*dst == 0) return dst;
	char* left = dst;
	char* right = dst + n - 1;
	char value = 0;
	while (left < right)
	{
		value = *left;
		*left = *right;
		*right = value;
		left++;
		right--;
	}
	return dst;
}
/*
* 传入dst和src用于建造跳过内建表 dst会先清空
* dst大于单字符表示上限时 大于部分不动
* size表明dst表的大小 当src有字符大于等于表size时 返回0
* 成功时返回表
*/
PORT_API char* str_table(char* dst, const char* src, qty_t size)
{
	COM_RETURN(dst == 0 || src == 0 || size == 0, 0);
	qty_t cap = size < STRUCT_STRING_SIZE ? size : STRUCT_STRING_SIZE;
	qty_t i = 0;
	while (i < cap) { dst[i] = 0; i++; }
	while (*src)
	{
		if ((unsigned char)*src >= size) return 0;
		dst[(unsigned char)*src] = 1;
		src++;
	}
	return dst;
}

#else


PORT_API qty_t str_len(const char* dst, qty_t n) { ; }
PORT_API cmp_t str_cmp(const char* dst, const char* src, qty_t n) { ; }
PORT_API char* str_cpy(char* dst, const char* src, qty_t n) { ; }
PORT_API char* str_cat(char* dst, const char* src, qty_t n) { ; }
PORT_API char* str_chr(const char* dst, const char* src, qty_t dir) { ; }
PORT_API char* str_str(const char* dst, const char* src, qty_t n) { ; }
PORT_API qty_t str_span(const char* dst, const char* src, qty_t is) { ; }
PORT_API qty_t str_tok(const char** dst, const char* tok) { ; }
PORT_API char* str_rev(char* dst, qty_t n) { ; }
PORT_API char* str_table(char* dst, const char* src, qty_t size) { ; }


#endif