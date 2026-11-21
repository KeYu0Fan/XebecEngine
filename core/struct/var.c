#include "var.h"
#if COM_USE(CONFIG_STRUCT_VAR)


#include "core/mem.h"
#include "core/struct/string.h"

/*
* 初始化ptr为var泛类型变量
* token为类型令牌 cap为data的大小
* 当使用柔性数组特性时 ptr应有var+data大小
* data拥有大小时应当满足token 否则失败
*/
PORT_API struct var* var_init(void* ptr, struct var_token* token, qty_t cap)
{
	COM_RETURN(ptr == 0 || token == 0, 0);
	struct var* var = (struct var*)ptr;
	if (cap != 0 && cap < token->size) return 0;
	var->token = token;
	var->cap = cap;
	var->data = cap ? (void*)((char*)var + sizeof(struct var)) : 0;
	return var;
}
/*
* 扩展var 当采用柔性数组特性时不应使用当前函数
* 当传递old时获取旧指针
* 需要传递新data的指针和大小
*/
PORT_API bool_t var_ext(struct var* var, void** old, void* ptr, qty_t cap)
{
	COM_RETURN(var == 0 || ptr == 0 || cap == 0, 0);
	if (cap < var->token->size) return 0;
	qty_t cpy = cap;
	if (old)
		*old = var->data;
	if (var->cap < cap)
		cpy = var->cap;
	if (var->data)
		mem_mov(ptr, var->data, cpy);
	var->data = ptr;
	var->cap = cap;
	return 1;
}
/* 仅重新设置变量的类型 当无法承载新类型时失败 数据亲和性不检查 */
PORT_API bool_t var_retag(struct var* var, const struct var_token* token)
{
	COM_RETURN(var == 0 || token == 0 || var->cap < token->size, 0);
	var->token = (struct var_token*)token;
	return 1;
}
/* 清空变量内容 */
PORT_API void var_clear(struct var* var)
{
	COM_RETURN(var == 0 || var->cap == 0 || var->data == 0, );
	if (var->token->ops.des)
		var->token->ops.des(var);
	mem_set(var->data, 0, var->cap);
}

/* 获取变量成员的下标 失败返回成员数量及以上 */
PORT_API qty_t var_find(struct var* var, const char* name)
{
	COM_RETURN(var == 0, (qty_t)-1);
	qty_t index = 0;
	qty_t memsize = var->token->count;
	const struct var_member* member = var_member(var->token, 0);
	while (index < memsize)
	{
		char* memname = member[index].name;
		if (name && memname)
		{
			if (str_cmp(name, memname, (qty_t)-1) == 0)
				return index;
		}
		else
			if (name == memname) return index;
		index++;
	}
	return index;
}
/* 获取变量成员的指针 失败返回空指针 */
PORT_API void* var_ptr(struct var* var, qty_t index)
{
	COM_RETURN(var == 0 || var->data == 0 || index >= var->token->count, 0);
	const struct var_member* member = var_member(var->token, index);
	return (void*)((char*)var->data + member->off);
}
/* 获取变量成员的值 要求dst容量不小于成员大小 否则失败 */
PORT_API bool_t var_get(struct var* var, qty_t index, void* dst, qty_t size)
{
	COM_RETURN(var == 0 || var->data == 0 || dst == 0, 0);
	const struct var_member* member = var_member(var->token, index);
	if (member == 0) return 0;
	if (member->size)
	{
		if (size < member->size) return 0;
		size = member->size;
	}
	else
	{
		qty_t len = var->cap - var->token->member[index].off;
		if (len == 0 || size < len) return 0;
		size = len;
	} 
	return mem_mov(dst, (void*)((char*)var->data + member->off), size) ? 1 : 0;
}
/* 设置变量成员的值 仅写入src的前size字节 要求size不大于成员大小 否则失败 */
PORT_API bool_t var_set(struct var* var, qty_t index, const void* src, qty_t size)
{
	COM_RETURN(var == 0 || var->data == 0 || src == 0, 0);
	const struct var_member* member = var_member(var->token, index);
	if (member == 0) return 0;
	if (member->size)
	{
		if (size > member->size) return 0;
	}
	else
	{
		qty_t len = var->cap - var->token->member[index].off;
		if (len == 0 || size > len) return 0;
	}
	return mem_mov((void*)((char*)var->data + member->off), src, size) ? 1 : 0;
}

/* 获取泛变量的大小 */
PORT_API qty_t var_size(const struct var_token* token)
{
	COM_RETURN(token == 0, 0);
	return token->size;
}
/* 获取成员数量 成功返回正整数 */
PORT_API qty_t var_count(const struct var_token* token)
{
	COM_RETURN(token == 0, 0);
	return token->count;
}
/* 获取成员元信息 修改会导致token损坏 */
PORT_API const struct var_member* var_member(const struct var_token* token, qty_t index)
{
	COM_RETURN(token == 0 || index >= token->count, 0);
	return &token->member[index];
}

/* 复制变量 返回目标 无实现浅拷贝 */
PORT_API struct var* var_cpy(struct var* dst, struct var* src)
{
	COM_RETURN(dst == 0 || src == 0, 0);
	if (dst->token != src->token && str_cmp(dst->token->name, src->token->name, (qty_t)-1))
		return 0;
	if (dst->token->ops.cpy)
	{
		if (dst->token->ops.cpy(dst, src))
			return dst;
		else
			return 0;
	}
	if (dst->cap >= src->cap)
	{
		mem_mov(dst->data, src->data, src->cap);
		return dst;
	}
	return 0;
}
/* 比较两个变量 返回差值 */
PORT_API cmp_t var_cmp(struct var* dst, struct var* src)
{
	COM_RETURN(dst == 0 || src == 0, 0);
	if (dst->token != src->token && str_cmp(dst->token->name, src->token->name, (qty_t)-1))
		return 1;
	if (dst->token->ops.cmp)
		return dst->token->ops.cmp(dst, src);
	if (dst->cap == src->cap)
		return mem_cmp(dst->data, src->data, dst->cap);
	else
		return (cmp_t)(dst->cap > src->cap ? 1 : -1);
}
/* 生成哈希值 FNV-1a */
PORT_API qty_t var_hash(struct var* var)
{
	COM_RETURN(var == 0 || var->data == 0, 0);
	if (var->token->ops.hash)
		return var->token->ops.hash(var);
	qty_t hash = COM_HASH_SEED;
	qty_t size = var->cap;
	unsigned char* data = (unsigned char*)var->data;
	while (size)
	{
		hash = (hash ^ *data) * COM_HASH_MUL;
		data++;
		size--;
	}
	return hash;
}
/* 字符串拼接辅助: 始终保留末尾 '\0' 的位置 */
PORT_HIDE static void var_put(char* buf, qty_t cap, qty_t* len, char c)
{
	if (*len + 1 < cap) buf[(*len)++] = c;
}
PORT_HIDE static void var_puts(char* buf, qty_t cap, qty_t* len, const char* s)
{
	while (s && *s) { var_put(buf, cap, len, *s); s++; }
}
/* 写入固定宽度小写十六进制 */
PORT_HIDE static void var_put_hex(char* buf, qty_t cap, qty_t* len, u64_t value, qty_t digits)
{
	static const char hex[] = "0123456789abcdef";
	while (digits)
	{
		digits--;
		var_put(buf, cap, len, hex[(value >> (digits * 4)) & 0xF]);
	}
}
/* 成员值: 按字节序(内存字节顺序)输出, 从低地址到高地址, 每字节两位小写十六进制
 * 注意: 没有数据(空变量)时不输出, 保持为空 */
PORT_HIDE static void var_put_value(char* buf, qty_t cap, qty_t* len, const void* data, qty_t size)
{
	qty_t i;
	if (data == 0 || size == 0) return;
	for (i = 0; i < size; i++)
		var_put_hex(buf, cap, len, ((const u8_t*)data)[i], 2);
}
/* 将变量转换为可视的字符串并返回
 * 自定义 to_string 优先; 默认输出为十六进制:
 *   token_name{
 *   [member:hex]
 *   [:hex]          // 成员名为 0 或空字符串时省略名称
 *   }
 */
PORT_API char* var_to_string(struct var* var, void* buff, qty_t size)
{
	char* string;
	qty_t len = 0;
	qty_t i;
	COM_RETURN(var == 0 || var->token == 0 || buff == 0 || size == 0, 0);
	if (var->token->ops.to_string)
		return var->token->ops.to_string(var, buff, size);
	string = (char*)buff;
	string[0] = '\0';
	if (var->token->name) var_puts(string, size, &len, var->token->name);
	var_put(string, size, &len, '{');
	var_put(string, size, &len, '\n');
	for (i = 0; i < var->token->count; i++)
	{
		const struct var_member* member = &var->token->member[i];
		var_put(string, size, &len, '[');
		if (member->name && member->name[0])
			var_puts(string, size, &len, member->name);
		var_put(string, size, &len, ':');
		var_put_value(string, size, &len,
			var->data ? (const char*)var->data + member->off : 0, member->size);
		var_put(string, size, &len, ']');
		var_put(string, size, &len, '\n');
	}
	var_put(string, size, &len, '}');
	string[len] = '\0';
	return string;
}

/* 检验token是否合法并返回序列化大小 */
PORT_API qty_t var_token_safe(struct var_token* token)
{
	COM_RETURN(token == 0, 0);
	if (token->name == 0 || token->size == 0 || token->member == 0 || token->count == 0)
		return 0;
	qty_t token_size = sizeof(struct var_token) + sizeof(struct var_member) * token->count;
	if (((token_size - sizeof(struct var_token)) / sizeof(struct var_member)) != token->count)
		return 0;
	qty_t token_string = (str_len(token->name, (qty_t)-1) + 1);
	if (token_string == 0) return 0;
	/* member计算 */
	{
		const struct var_member* member = token->member;
		qty_t member_size = 0;
		bool_t is_null_name = 0;
		qty_t i = 0;
		while (i < token->count)
		{
			/* name */
			if (member[i].name)
			{
				qty_t size = str_len(member[i].name, (qty_t)-1) + 1;
				if (size == 0 || token_string + size < token_string) return 0;
				token_string += size;
			}
			else
			{
				is_null_name++;
				if (is_null_name > 1) return 0;
			}
			/* size */
			{
				qty_t size = member[i].off + member[i].size;
				if (member[i].size)
				{
					if (size <= member[i].off)
						return 0;
				}
				else
					if (i + 1 != token->count) return 0;
				if (member_size < size)
					member_size = size;
			}
			/* flags */
			if (member[i].Ptr || member[i].Int || member[i].Float)
			{
				if (member[i].Ptr && (member[i].Int || member[i].Float))
					return 0;
				if (member[i].Int && (member[i].Ptr || member[i].Float))
					return 0;
				if (member[i].Float && (member[i].Ptr || member[i].Int))
					return 0;
			}
			i++;
		}
		/* 满足 可能的对齐|柔性数组预定义大小 需求 */
		if (member_size > token->size) return 0;
	}
	return token_size + token_string;
}
/*
 * 序列化token树为相对二进制
 * 部分无法相对偏移数据会被置空 例如 ops需要重新绑定
 * buff 外部提供的内存
 * size 如果buff不为NULL 表示容量上限
 * 失败返回0 成功返回序列化实际写入的字节数
 */
PORT_API qty_t var_token_save(struct var_token* token, void* buff, qty_t size)
{
	COM_RETURN(token == 0 || buff == 0 || size < sizeof(struct var_token) + sizeof(struct var_member), 0);
	char* s = (char*)buff;
	struct var_token* t = (struct var_token*)buff;
	struct var_member* m = (struct var_member*)((char*)buff + sizeof(struct var_token));
	qty_t off = (token->count * sizeof(struct var_member) + sizeof(struct var_token));
	qty_t tmp = 0;
	if (token->count == 0 || size < off)
		return 0;
	/* token member */
	*t = *token;
	struct var_ops ops = { 0 };
	t->ops = ops;
	mem_mov((void*)m, (void*)token->member, sizeof(struct var_member) * token->count);
	/* string.token */
	tmp = str_len(token->name, (qty_t)-1) + 1;
	if (off + tmp > size) return 0;
	str_cpy(s + off, token->name, tmp);
	t->member = (struct var_member*)sizeof(struct var_token);
	t->name = (char*)off;
	off = off + tmp;
	/* string.member */
	qty_t i = 0;
	while (i < token->count)
	{
		if (m[i].name == 0)
		{
			i++;
			continue;
		}
		tmp = str_len(m[i].name, size - off) + 1;
		if (off + tmp > size) return 0;
		str_cpy(s + off, m[i].name, tmp);
		m[i].name = (char*)off;
		off = off + tmp;
		i++;
	}
	return off;
}
/*
 * 从相对二进制数据中原地恢复token
 * 部分无法相对偏移的置空数据无法恢复
 * buff 包含二进制数据的可写内存块
 * size 数据块大小
 * 恢复出的token指针 失败返回0
 */
PORT_API struct var_token* var_token_load(void* buff, qty_t size)
{
	COM_RETURN(buff == 0 || size < sizeof(struct var_token) + sizeof(struct var_member), 0);
	char* s = (char*)buff;
	struct var_token* t = (struct var_token*)buff;
	struct var_member* m = (struct var_member*)((char*)buff + sizeof(struct var_token));
	qty_t off = (t->count * sizeof(struct var_member) + sizeof(struct var_token));
	if (t->count == 0 || size < off)
		return 0;
	if (((off - sizeof(struct var_token)) / sizeof(struct var_member)) != t->count)
		return 0;
	/* token */
	t->name = s + off;
	t->member = m;
	struct var_ops ops = { 0 };
	t->ops = ops;
	off = off + str_len(s + off, size - off) + 1;
	if (off > size) return 0;
	/* member */
	qty_t i = 0;
	while (i < t->count)
	{
		if (m[i].name == 0)
		{
			i++;
			continue;
		}
		m[i].name = s + off;
		off = off + str_len(s + off, size - off) + 1;
		if (off > size) return 0;
		i++;
	}
	/* ret */
	return t;
}


#else


PORT_API struct var* var_init(void* ptr, struct var_token* token, qty_t size) { ; }
PORT_API bool_t var_ext(struct var* var, void** old, void* ptr, qty_t size) { ; }
PORT_API bool_t var_retag(struct var* var, const struct var_token* token) { ; }
PORT_API void var_clear(struct var* var) { ; }
PORT_API qty_t var_find(struct var* var, const char* name) { ; }
PORT_API void* var_ptr(struct var* var, qty_t index) { ; }
PORT_API bool_t var_get(struct var* var, qty_t index, void* dst, qty_t size) { ; }
PORT_API bool_t var_set(struct var* var, qty_t index, const void* src, qty_t size) { ; }
PORT_API qty_t var_size(const struct var_token* token) { ; }
PORT_API qty_t var_count(struct var* var) { ; }
PORT_API const struct var_member* var_member(struct var* var, qty_t index) { ; }
PORT_API struct var* var_cpy(struct var* dst, struct var* src) { ; }
PORT_API cmp_t var_cmp(struct var* dst, struct var* src) { ; }
PORT_API qty_t var_hash(struct var* var) { ; }
PORT_API char* var_to_string(struct var* var, void* buff, qty_t size) { ; }
PORT_API qty_t var_token_safe(struct var_token* token) { ; }
PORT_API qty_t var_token_save(struct var_token* token, void* buff, qty_t size) { ; }
PORT_API struct var_token* var_token_load(void* buff, qty_t size) { ; }


#endif