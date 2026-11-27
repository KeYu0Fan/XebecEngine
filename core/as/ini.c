#include "ini.h"
#if COM_USE(CONFIG_AS_INI)


#include "core/errorcode.h"
#include "core/struct/string.h"
#include "core/struct/arena.h"

PORT_HIDE static struct ini_sectn* ini_get_sectn(struct ini* ini, qty_t index)
{
	if (index == 0) return ini->sectn;
	struct ini_sectn* tmp = ini->sectn;
	while (index && tmp)
	{
		tmp = tmp->next;
		index--;
	}
	if (index) return 0;
	return tmp;
}
PORT_HIDE static struct ini_entry* ini_get_entry(struct ini_sectn* sectn, qty_t index)
{
	if (index == 0) return sectn->entry;
	struct ini_entry* tmp = sectn->entry;
	while (index && tmp)
	{
		tmp = tmp->next;
		index--;
	}
	if (index) return 0;
	return tmp;
}

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
		arena_del(arena,  ini);
		return 0;
	}
	return ini;
}
/* 清空ini创建结构 非o1行为 */
PORT_API void ini_clear(struct ini* ini)
{
	COM_RETURN(ini == 0, );
	struct ini_sectn* sectn = ini->sectn;
	while (sectn)
	{
		struct ini_sectn* tmpsectn = sectn->next;
		struct ini_entry* entry = sectn->entry;
		while (entry)
		{
			struct ini_entry* tmpentry = entry->next;
			arena_del(ini->arena, entry->key);
			arena_del(ini->arena, entry->value);
			arena_del(ini->arena, entry);
			entry = tmpentry;
		}
		arena_del(ini->arena, sectn->name);
		arena_del(ini->arena, sectn);
		sectn = tmpsectn;
	}
	arena_del(ini->arena, ini);
}
/* 检查text是否为合法的ini结构 */
PORT_API err_t ini_safe(const char* text, qty_t len)
{
	COM_RETURN(text == 0 || len == 0, ERROR_PARAMTER);
	const char* string = text;
	const char* end = text + len;
	while (text)
	{
		str_tok(&text, "\n", 0);
		string += str_span(string, " \t", 0);
		if (text >= end) return ERROR_MEMORY_OVER;
		if (*string == '\0') return ERROR_NULL;
		if (*string == ';' || *string == '#')
		{
			string = text;
			continue;
		}
		if (*string == '[')
		{
			char* tmp = str_chr(string, "]\n", 0);
			if (tmp == 0 || *tmp == '\n' || tmp >= end) return ERROR_MEMORY_OVER;
			if (*(tmp + 1 + str_span(tmp + 1, " \t", 0)) != '\n')
				return ERROR_SYNTAX;
		}
		else
		{
			if (*str_chr(string, "=\n", 0) == '\n')
				return ERROR_SYNTAX;
		}
		string = text;
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
			if (isize + 3 > len) return 0;
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
	const char* texts = text;
	qty_t index = ((qty_t)-1);

	char table[(((unsigned char)-1) + 1)];
	str_table(table,"\n", (((unsigned char)-1) + 1));

	while (texts != 0)
	{
		char* string = (char*)texts;
		qty_t size = str_tok(&texts, table, 1);
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
				ini_clear(ini);
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
	struct ini_sectn* tmpsectn = ini->sectn;
	while (tmpsectn)
	{
		if (str_cmp(tmpsectn->name, name, str_len(tmpsectn->name, (qty_t)-1) + 1) == 0)
			return i;
		tmpsectn = tmpsectn->next;
		i++;
	}
	return ((qty_t)-1);
}
/* 查找entry的索引 找不到时返回((qty_t)-1) */
PORT_API qty_t  ini_find_entry(struct ini* ini, qty_t sectn, const char* key)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || key == 0 || *key == 0, ((qty_t)-1));
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0) return ((qty_t)-1);
	struct ini_entry* tmpentry = tmpsectn->entry;
	qty_t i = 0;
	while (tmpentry)
	{
		if (str_cmp(tmpentry->key, key, str_len(tmpentry->key, (qty_t)-1) + 1) == 0)
			return i;
		tmpentry = tmpentry->next;
		i++;
	}
	return ((qty_t)-1);
}
/* 插入一个sectn 不能重名 */
PORT_API bool_t ini_insert_sectn(struct ini* ini, const char* name)
{
	COM_RETURN(ini == 0 || ini_find_sectn(ini, name) != ((qty_t)-1), 0);
	struct ini_sectn* tmpsectn = ini->sectn;
	while (tmpsectn && tmpsectn->next) tmpsectn = tmpsectn->next;
	
	struct ini_sectn* newsectn = arena_new(ini->arena, sizeof(struct ini_sectn));
	qty_t newssize = str_len(name, (qty_t)-1) + 1;
	char* newsname = arena_new(ini->arena, newssize);
	if (newsectn == 0 || newsname == 0)
	{
		if (newsectn) arena_del(ini->arena, newsectn);
		if (newsname) arena_del(ini->arena, newsname);
		return 0;
	}

	ini->sectn_count += 1;
	newsectn->entry = 0;
	newsectn->entry_count = 0;
	newsectn->name = newsname;
	newsectn->next = 0;
	str_cpy(newsname, name, newssize);
	if (tmpsectn)
		tmpsectn->next = newsectn;
	else
		ini->sectn = newsectn;
	return 1;
}
/* 插入一个entry 在同一个sectn中不能重名 */
PORT_API bool_t ini_insert_entry(struct ini* ini, qty_t sectn, const char* key, const char* value)
{
	COM_RETURN(ini == 0 || ini_find_entry(ini, sectn, key) != ((qty_t)-1), 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0) return 0;
	struct ini_entry* tmpentry = tmpsectn->entry;
	while (tmpentry && tmpentry->next) tmpentry = tmpentry->next;

	struct ini_entry* newentry = arena_new(ini->arena, sizeof(struct ini_entry));
	qty_t newksize = str_len(key, (qty_t)-1) + 1;
	qty_t newvsize = str_len(value, (qty_t)-1) + 1;
	char* newkey = arena_new(ini->arena, newksize);
	char* newvalue = arena_new(ini->arena, newvsize);
	if (newentry == 0 || newkey == 0 || newvalue == 0)
	{
		if (newentry) arena_del(ini->arena, newentry);
		if (newkey) arena_del(ini->arena, newkey);
		if (newvalue) arena_del(ini->arena, newvalue);
		return 0;
	}

	tmpsectn->entry_count += 1;
	newentry->key = newkey;
	newentry->value = newvalue;
	newentry->next = 0;
	str_cpy(newkey, key, newksize);
	str_cpy(newvalue, value, newvsize);
	if (tmpentry)
		tmpentry->next = newentry;
	else
		tmpsectn->entry = newentry;
	return 1;
}
/* 删除一个sectn */
PORT_API bool_t ini_remove_sectn(struct ini* ini, qty_t sectn)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn ? sectn - 1 : 0);
	if (tmpsectn)
	{
		struct ini_sectn* delsectn = tmpsectn->next;
		if (sectn)
			tmpsectn->next = tmpsectn->next->next;
		else
		{
			delsectn = ini->sectn;
			ini->sectn = ini->sectn->next;
		}
		if (delsectn == 0) return 0;
		ini->sectn_count -= 1;
		arena_del(ini->arena, delsectn->name);
		struct ini_entry* delentry = delsectn->entry;
		while (delentry)
		{
			struct ini_entry* tmp = delentry->next;
			arena_del(ini->arena, delentry->key);
			arena_del(ini->arena, delentry->value);
			arena_del(ini->arena, delentry);
			delentry = tmp;
		}
		arena_del(ini->arena, delsectn);
		return 1;
	}
	return 0;
}
/* 删除一个entry */
PORT_API bool_t ini_remove_entry(struct ini* ini, qty_t sectn, qty_t entry)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0 || entry >= tmpsectn->entry_count)
		return 0;
	struct ini_entry* tmpentry = ini_get_entry(tmpsectn, entry ? entry - 1 : 0);
	if (tmpentry)
	{
		struct ini_entry* del = tmpentry->next;
		if (entry)
			tmpentry->next = tmpentry->next->next;
		else
		{
			del = tmpsectn->entry;
			tmpsectn->entry = tmpsectn->entry->next;
		}
		if (del == 0) return 0;
		tmpsectn->entry_count -= 1;
		arena_del(ini->arena, del->key);
		arena_del(ini->arena, del->value);
		arena_del(ini->arena, del);
		return 1;
	}
	return 0;
}

/* 设置当前sectn的名称 */
PORT_API bool_t ini_set_sectn_name(struct ini* ini, qty_t sectn, const char* name)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || name == 0 || *name == 0, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn)
	{
		qty_t oldmsize = str_len(tmpsectn->name, (qty_t)-1) + 1;
		qty_t newmsize = str_len(name, (qty_t)-1) + 1;
		if (oldmsize >= newmsize)
		{
			str_cpy(tmpsectn->name, name, newmsize);
		}
		else
		{
			char* tmps = arena_new(ini->arena, newmsize);
			if (tmps == 0) return 0;
			str_cpy(tmps, name, newmsize);
			arena_del(ini->arena, tmpsectn->name);
			tmpsectn->name = tmps;
		}
		return 1;
	}
	return 0;
}
/* 设置当前sectn当前entry的key */
PORT_API bool_t ini_set_entry_key(struct ini* ini, qty_t sectn, qty_t entry, const char* key)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || key == 0 || *key == 0, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0 || entry >= tmpsectn->entry_count)
		return 0;
	struct ini_entry* tmpentry = ini_get_entry(tmpsectn, entry);
	if (tmpentry)
	{
		qty_t oldksize = str_len(tmpentry->key, (qty_t)-1) + 1;
		qty_t newksize = str_len(key, (qty_t)-1) + 1;
		if (oldksize >= newksize)
		{
			str_cpy(tmpentry->key, key, newksize);
		}
		else
		{
			char* tmpk = arena_new(ini->arena, newksize);
			if (tmpk == 0) return 0;
			str_cpy(tmpk, key, newksize);
			arena_del(ini->arena, tmpentry->key);
			tmpentry->key = tmpk;
		}
		return 1;
	}
	return 0;
}
/* 设置当前sectn当前entry的value */
PORT_API bool_t ini_set_entry_value(struct ini* ini, qty_t sectn, qty_t entry, const char* value)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count || value == 0 || *value == 0, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0 || entry >= tmpsectn->entry_count)
		return 0;
	struct ini_entry* tmpentry = ini_get_entry(tmpsectn, entry);
	if (tmpentry)
	{
		qty_t oldvsize = str_len(tmpentry->value, (qty_t)-1) + 1;
		qty_t newvsize = str_len(value, (qty_t)-1) + 1;
		if (oldvsize >= newvsize)
		{
			str_cpy(tmpentry->value, value, oldvsize);
		}
		else
		{
			char* tmpv = arena_new(ini->arena, newvsize);
			if (tmpv == 0) return 0;
			str_cpy(tmpv, value, newvsize);
			arena_del(ini->arena, tmpentry->value);
			tmpentry->value = tmpv;
		}
		return 1;
	}
	return 0;
}
/* 获取当前sectn的名称 */
PORT_API const char* ini_get_sectn_name(struct ini* ini, qty_t sectn)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn)
		return tmpsectn->name;
	return 0;
}
/* 获取当前sectn当前entry的key */
PORT_API const char* ini_get_entry_key(struct ini* ini, qty_t sectn, qty_t entry)
{
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0 || entry >= tmpsectn->entry_count)
		return 0;
	struct ini_entry* tmpentry = ini_get_entry(tmpsectn, entry);
	if (tmpentry)
		return tmpentry->key;
	return 0;
}
/* 获取当前sectn当前entry的value */
PORT_API const char* ini_get_entry_value(struct ini* ini, qty_t sectn, qty_t entry)
{ 
	COM_RETURN(ini == 0 || sectn >= ini->sectn_count, 0);
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn == 0 || entry >= tmpsectn->entry_count)
		return 0;
	struct ini_entry* tmpentry = ini_get_entry(tmpsectn, entry);
	if (tmpentry)
		return tmpentry->value;
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
	struct ini_sectn* tmpsectn = ini_get_sectn(ini, sectn);
	if (tmpsectn) return tmpsectn->entry_count;
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


PORT_API struct ini* ini_init(struct arena* arena) { ; }
PORT_API void ini_clear(struct ini* ini) { ; }
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