#include "ini.h"
#if COM_USE(CONFIG_AS_INI)


#include "core/errorcode.h"
#include "core/mem.h"
#include "core/struct/string.h"
#include "core/struct/arena.h"

/* 创建空白ini结构 */
PORT_API struct ini* ini_init(struct arena* arena)
{
	COM_RETURN(arena == 0, 0);
	struct ini* ini = arena_new(arena, sizeof(struct ini));
	if (ini == 0) return 0;
	ini->arena = arena;
	ini->sectn = 0;
	ini->sectn_count = 0;
	if (ini_insert_sectn(ini,"") == 0)
	{
		arena_clear(arena);
		return 0;
	}
	return ini;
}
/* 检查text是否为合法的ini结构 */
PORT_API err_t ini_safe(const char* text, qty_t len)
{
	COM_RETURN(text == 0 || len == 0, ERROR_PARAMTER);
	const char* p = text;
	const char* end = text + len;
	/* 跳过 UTF-8 BOM */
	if (len >= 3 && (unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF)
		p += 3;
	while (p < end)
	{
		const char* line = p;
		while (p < end && *p != '\n' && *p != '\r' && *p != '\0') p++;
		const char* line_end = p;
		if (p < end && *p == '\0')
		{
			if (p + 1 != end) return ERROR_SYNTAX;
			p = end;
		}
		else if (p < end)
		{
			if (*p == '\r' && p + 1 < end && p[1] == '\n') p += 2;
			else p++;
		}
		/* 行首/行尾 trim */
		while (line < line_end && (*line == ' ' || *line == '\t')) line++;
		while (line_end > line && (line_end[-1] == ' ' || line_end[-1] == '\t')) line_end--;
		if (line == line_end) continue;
		if (*line == ';' || *line == '#') continue;
		if (*line == '[')
		{
			const char* close = line + 1;
			while (close < line_end && *close != ']') close++;
			if (close >= line_end) return ERROR_SYNTAX;
			const char* t = close + 1;
			while (t < line_end && (*t == ' ' || *t == '\t')) t++;
			if (t != line_end) return ERROR_SYNTAX;
			/* [] 作为根 section, 与 ini_load 保持一致 */
		}
		else
		{
			const char* eq = line;
			while (eq < line_end && *eq != '=') eq++;
			if (eq >= line_end) return ERROR_SYNTAX;
			const char* key_end = eq;
			while (key_end > line && (key_end[-1] == ' ' || key_end[-1] == '\t')) key_end--;
			if (key_end == line) return ERROR_SYNTAX;
		}
	}
	return ERROR_NULL;
}
/* 保存ini为可读取的字符串 如果保存导致再加载行为错误时返回失败 */
PORT_API bool_t ini_save(struct ini* ini, char* text, qty_t len)
{
	COM_RETURN(ini == 0 || text == 0 || len == 0, 0);
	struct ini_sectn* sectn = ini->sectn;
	while (sectn)
	{
		if (str_chr(sectn->name, "\r\n", 0)) return 0;
		*text = '[';
		text++;
		len--;
		qty_t isize = str_len(sectn->name,(qty_t)-1);
		if (isize + 3 > len) return 0;
		str_cpy(text, sectn->name, isize + 1);
		text += isize;
		len -= isize;
		*text = ']';
		text++;
		len--;
		*text = '\n';
		text++;
		len--;
		struct ini_entry* entry = sectn->entry;
		if (entry == 0)
		{
			sectn = sectn->next;
			continue;
		}
		while (entry)
		{
			if (str_chr(entry->key, "=\r\n", 0)) return 0;
			if (str_chr(entry->value, "]\r\n", 0)) return 0;
			qty_t jksize = str_len(entry->key, (qty_t)-1);
			qty_t jvsize = str_len(entry->value, (qty_t)-1);
			if (jksize + 1 + jvsize + 1 > len) return 0;
			str_cpy(text, entry->key, jksize + 1);
			text += jksize;
			len -= jksize;
			*text = '=';
			text++;
			len--;
			str_cpy(text, entry->value, jvsize + 1);
			text += jvsize;
			len -= jvsize;
			*text = '\n';
			text++;
			len--;
			entry = entry->next;
		}
		sectn = sectn->next;
	}
	if (len)
	{
		*text = '\0';
		return 1;
	}
	return 0;
}
/* 加载ini为可操作的结构 */
PORT_API struct ini* ini_load(struct arena* arena, const char* text, qty_t len)
{
	COM_RETURN(arena == 0 || text == 0 || len == 0, 0);
	struct ini* ini = ini_init(arena);
	if (ini == 0) return 0;
	/* 复制一份到 arena，避免修改调用方的 const 文本 */
	char* copy = arena_new(arena, len + 1);
	if (copy == 0) return 0;
	mem_cpy(copy, text, len);
	copy[len] = '\0';
	char* texts = copy;
	qty_t index = ((qty_t)-1);

	char table[(((unsigned char)-1) + 1)];
	str_table(table,"\n", (((unsigned char)-1) + 1));

	while (texts != 0)
	{
		char* string = texts;
		const char* scan = texts;
		qty_t size = str_tok(&scan, table, 1);
		texts = (char*)scan;
		if (*string == ';' || *string == '#')
			continue;
		
		qty_t jump = str_span(string," \t", 0);
		if (jump)
		{
			string += jump;
			size -= jump;
		}

		char tmp = *(string + size);
		*(string + size) = '\0';
		if (*string == '[')
		{
			char* tmp2 = str_chr(string, "]", 1);
			*tmp2 = '\0';
			ini_insert_sectn(ini, (string + 1));
			index = ini_find_sectn(ini, (string+ 1));
			if (index == ((qty_t)-1))
			{
				arena_clear(ini->arena);
				return 0;
			}
			*tmp2 = ']';
		}
		else if (*string != '\0')
		{
			char* tmp2 = str_chr(string, "=", 0);
			*tmp2 = '\0';
			char* stringk = string;
			char* stringv = tmp2 + 1;
			ini_insert_entry(ini, index, stringk, stringv);
			*tmp2 = '=';
		}
		*(string + size) = tmp;
	}
	return ini;
}

/* 查找sectn的索引 找不到时返回((qty_t)-1) */
PORT_API qty_t  ini_find_sectn(struct ini* ini, const char* name)
{
	COM_RETURN(ini == 0 || ini->sectn == 0 || name == 0, ((qty_t)-1));
	if (*name == '\0') return 0;
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (str_cmp(tmp->name, name, str_len(tmp->name, (qty_t)-1) + 1) == 0)
			return i;
		tmp = tmp->next;
		i++;
	}
	return ((qty_t)-1);
}
/* 查找entry的索引 找不到时返回((qty_t)-1) */
PORT_API qty_t  ini_find_entry(struct ini* ini, qty_t sectn, const char* key)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || key == 0 || *key == 0, ((qty_t)-1));
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (str_cmp(tmp2->key, key, str_len(tmp2->key, (qty_t)-1) + 1) == 0)
					return i;
				tmp2 = tmp2->next;
				i++;
			}
			return ((qty_t)-1);
		}
		tmp = tmp->next;
		i++;
	}
	return ((qty_t)-1);
}
/* 插入一个sectn 不能重名 */
PORT_API bool_t ini_insert_sectn(struct ini* ini, const char* name)
{
	COM_RETURN(ini == 0 || ini_find_sectn(ini, name) != ((qty_t)-1), 0);
	struct ini_sectn* tmp = ini->sectn;
	while (tmp && tmp->next)
		tmp = tmp->next;
	
	struct ini_sectn* newsectn = arena_new(ini->arena, sizeof(struct ini_sectn));
	qty_t newssize = str_len(name, (qty_t)-1) + 1;
	char* newsname = arena_new(ini->arena, newssize);
	if (newsectn == 0 || newsname == 0)
	{
		if (newsectn == 0) arena_del(ini->arena, newsectn);
		if (newsname == 0) arena_del(ini->arena, newsname);
		return 0;
	}

	ini->sectn_count += 1;

	newsectn->entry = 0;
	newsectn->entry_count = 0;
	newsectn->name = newsname;
	newsectn->next = 0;
	str_cpy(newsname, name, newssize);
	if (tmp)
		tmp->next = newsectn;
	else
		ini->sectn = newsectn;
	return 1;
}
/* 插入一个entry 在同一个sectn中不能重名 */
PORT_API bool_t ini_insert_entry(struct ini* ini, qty_t sectn, const char* key, const char* value)
{
	COM_RETURN(ini == 0 || ini_find_entry(ini, sectn, key) != ((qty_t)-1), 0);
	struct ini_sectn* tmp = ini->sectn;
	struct ini_entry* tmp2 = 0;
	{
		while (tmp && sectn)
		{
			sectn--;
			tmp = tmp->next;
		}
		if (tmp == 0) return 0;
		tmp2 = tmp->entry;
		while (tmp2 && tmp2->next)
			tmp2 = tmp2->next;
	}

	struct ini_entry* newentry = arena_new(ini->arena, sizeof(struct ini_entry));
	qty_t newksize = str_len(key, (qty_t)-1) + 1;
	qty_t newvsize = str_len(value, (qty_t)-1) + 1;
	char* newkey = arena_new(ini->arena, newksize);
	char* newvalue = arena_new(ini->arena, newvsize);
	if (newentry == 0 || newkey == 0 || newvalue == 0)
	{
		if (newentry == 0) arena_del(ini->arena, newentry);
		if (newkey == 0) arena_del(ini->arena, newkey);
		if (newvalue == 0) arena_del(ini->arena, newvalue);
		return 0;
	}

	tmp->entry_count += 1;

	newentry->key = newkey;
	newentry->value = newvalue;
	newentry->next = 0;
	str_cpy(newkey, key, newksize);
	str_cpy(newvalue, value, newvsize);
	if (tmp2)
		tmp2->next = newentry;
	else
		tmp->entry = newentry;
	return 1;
}
/* 删除一个sectn */
PORT_API bool_t ini_remove_sectn(struct ini* ini, qty_t sectn)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (sectn == 0 || (i + 1) == sectn)
		{
			struct ini_sectn* del = tmp->next;
			if (sectn > 0)
			{
				if (tmp->next)
					tmp->next = tmp->next->next;
			}
			else
			{
				del = ini->sectn;
				if (ini->sectn)
					ini->sectn = ini->sectn->next;
			}

			ini->sectn_count -= 1;

			arena_del(ini->arena,del->name);
			struct ini_entry* del2 = del->entry;
			while (del2)
			{
				struct ini_entry* del2_ey = del2->next;
				arena_del(ini->arena,del2->key);
				arena_del(ini->arena, del2->value);
				arena_del(ini->arena, del2);
				del2 = del2_ey;
			}
			arena_del(ini->arena, del);
			return 1;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 删除一个entry */
PORT_API bool_t ini_remove_entry(struct ini* ini, qty_t sectn, qty_t entry)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			if (entry >= tmp->entry_count)
				return 0;
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (entry == 0 || (i + 1) == entry)
				{
					struct ini_entry* del = tmp2->next;
					if (entry > 0)
					{
						if (tmp2->next)
							tmp2->next = tmp2->next->next;
					}
					else
					{
						del = tmp->entry;
						if (tmp->entry)
							tmp->entry = tmp->entry->next;
					}

					tmp->entry_count -= 1;

					arena_del(ini->arena, del->key);
					arena_del(ini->arena, del->value);
					arena_del(ini->arena, del);
					return 1;
				}
				tmp2 = tmp2->next;
				i++;
			}
			return 0;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}

/* 设置当前sectn的名称 */
PORT_API bool_t ini_set_sectn_name(struct ini* ini, qty_t sectn, const char* name)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || name == 0 || *name == 0, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			qty_t oldmsize = str_len(tmp->name, (qty_t)-1) + 1;
			qty_t newmsize = str_len(name, (qty_t)-1) + 1;
			if (oldmsize >= newmsize)
			{
				str_cpy(tmp->name, name, newmsize);
			}
			else
			{
				char* tmps = arena_new(ini->arena, newmsize);
				if (tmps == 0) return 0;
				str_cpy(tmps, name, newmsize);
				arena_del(ini->arena, tmp->name);
				tmp->name = tmps;
			}
			return 1;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 设置当前sectn当前entry的key */
PORT_API bool_t ini_set_entry_key(struct ini* ini, qty_t sectn, qty_t entry, const char* key)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || key == 0 || *key == 0, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			if (entry >= tmp->entry_count)
				return 0;
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (i == entry)
				{
					qty_t oldksize = str_len(tmp2->key, (qty_t)-1) + 1;
					qty_t newksize = str_len(key, (qty_t)-1) + 1;
					if (oldksize >= newksize)
					{
						str_cpy(tmp2->key, key, newksize);
					}
					else
					{
						char* tmpk = arena_new(ini->arena, newksize);
						if (tmpk == 0) return 0;
						str_cpy(tmpk, key, newksize);
						arena_del(ini->arena, tmp2->key);
						tmp2->key = tmpk;
					}
					return 1;
				}
				tmp2 = tmp2->next;
				i++;
			}
			return 0;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 设置当前sectn当前entry的value */
PORT_API bool_t ini_set_entry_value(struct ini* ini, qty_t sectn, qty_t entry, const char* value)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || value == 0 || *value == 0, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			if (entry >= tmp->entry_count)
				return 0;
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (i == entry)
				{
					qty_t oldvsize = str_len(tmp2->value, (qty_t)-1) + 1;
					qty_t newvsize = str_len(value, (qty_t)-1) + 1;
					if (oldvsize >= newvsize)
					{
						str_cpy(tmp2->value, value, oldvsize);
					}
					else
					{
						char* tmpv = arena_new(ini->arena, newvsize);
						if (tmpv == 0) return 0;
						str_cpy(tmpv, value, newvsize);
						arena_del(ini->arena, tmp2->value);
						tmp2->value = tmpv;
					}
					return 1;
				}
				tmp2 = tmp2->next;
				i++;
			}
			return 0;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 获取当前sectn的名称 */
PORT_API const char* ini_get_sectn_name(struct ini* ini, qty_t sectn)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
			return tmp->name;
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 获取当前sectn当前entry的key */
PORT_API const char* ini_get_entry_key(struct ini* ini, qty_t sectn, qty_t entry)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			if (entry >= tmp->entry_count)
				return 0;
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (i == entry)
					return tmp2->key;
				tmp2 = tmp2->next;
				i++;
			}
			return 0;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 获取当前sectn当前entry的value */
PORT_API const char* ini_get_entry_value(struct ini* ini, qty_t sectn, qty_t entry)
{ 
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
		{
			if (entry >= tmp->entry_count)
				return 0;
			struct ini_entry* tmp2 = tmp->entry;
			i = 0;
			while (tmp2)
			{
				if (i == entry)
					return tmp2->value;
				tmp2 = tmp2->next;
				i++;
			}
			return 0;
		}
		tmp = tmp->next;
		i++;
	}
	return 0;
}

/* 获取sectn的数量 */
PORT_API qty_t ini_sectn_count(struct ini* ini)
{
	COM_RETURN(ini == 0, 0);
	return ini->sectn_count;
}
/* 获取当前sectn的entry数量 */
PORT_API qty_t ini_entry_count(struct ini* ini, qty_t sectn)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	qty_t i = 0;
	struct ini_sectn* tmp = ini->sectn;
	while (tmp)
	{
		if (i == sectn)
			return tmp->entry_count;
		tmp = tmp->next;
		i++;
	}
	return 0;
}
/* 获取ini实例化可能的大小 */
PORT_API qty_t ini_size(const char* text, qty_t len)
{
	COM_RETURN(text == 0 || *(text + len - 1) != '\0', 0);
	const char* string = text;
	qty_t size = len + sizeof(struct ini);
	while (text)
	{
		str_tok(&text, "\n", 0);
		string += str_span(string, " \t", 0);
		if (*string != ';' || *string != '#') size -= (text - string);
		if (*string == '[') len += sizeof(struct ini_sectn) + sizeof(qty_t);
		else len += sizeof(struct ini_entry) + sizeof(qty_t) * 2;
		string = text;
	}
	return size;
}


#else


PORT_API err_t ini_safe(const char* text, qty_t len) { ; }
PORT_API bool_t ini_save(struct ini* ini, char* text, qty_t len) { ; }
PORT_API struct ini* ini_load(struct arena* arena, const char* text, qty_t len) { ; }
PORT_API qty_t  ini_find_sectn(struct ini* ini, const char* name) { ; }
PORT_API qty_t  ini_find_entry(struct ini* ini, qty_t sectn, const char* key) { ; }
PORT_API bool_t ini_insert_sectn(struct ini* ini, const char* name) { ; }
PORT_API bool_t ini_insert_entry(struct ini* ini, qty_t sectn, const char* key, const char* value) { ; }
PORT_API bool_t ini_remove_sectn(struct ini* ini, qty_t sectn) { ; }
PORT_API bool_t ini_remove_entry(struct ini* ini, qty_t sectn, qty_t entry) { ; }
PORT_API bool_t ini_set_sectn_name(struct ini* ini, qty_t sectn, const char* name) { ; }
PORT_API bool_t ini_set_entry_key(struct ini* ini, qty_t sectn, qty_t entry, const char* key) { ; }
PORT_API bool_t ini_set_entry_value(struct ini* ini, qty_t sectn, qty_t entry, const char* value) { ; }
PORT_API const char* ini_get_sectn_name(struct ini* ini, qty_t sectn) { ; }
PORT_API const char* ini_get_entry_key(struct ini* ini, qty_t sectn, qty_t entry) { ; }
PORT_API const char* ini_get_entry_value(struct ini* ini, qty_t sectn, qty_t entry) { ; }
PORT_API qty_t ini_sectn_count(struct ini* ini) { ; }
PORT_API qty_t ini_entry_count(struct ini* ini, qty_t sectn) { ; }
PORT_API qty_t ini_size(const char* text, qty_t len) { ; }


#endif