#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "core/error.h"
#include "core/struct/string.h"
#include "core/struct/arena.h"
#include "core/as/ini.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)
char string[] = "[]\na=s\ns=a\n;666\n[?]\nname=fky\nage=21\n[5]\n";

struct arena* a_ini_init(void)
{
	qty_t arena_s = 4096;
	qty_t arena_l = 4;
	qty_t arena_c = arena_size(arena_s, arena_l);
	void* arena_m = malloc(arena_c);
	if (arena_m == 0) return 0;
	return arena_init(arena_m, arena_c, arena_s);
}
void a_ini_del(struct arena* arena)
{
	free(arena);
}

int main()
{
	struct arena* arena = a_ini_init();
	if (arena == 0)
	{
		printf("%s", error_to_string(ERROR_MEMORY_LACK));
		return 0;
	}

	/* 创建 */
	struct ini* ini = ini_load(arena,string,sizeof(string));
	if (ini == 0)
	{
		printf("%s", error_to_string(-1));
		return 0;
	}
	/* 获取 */
	CHECK(ini_sectn_count(ini) == 3);
	CHECK(ini_entry_count(ini, 0) == 2);
	{
		const char* name = ini_get_sectn_name(ini, 0);
		CHECK(str_cmp(name,"",1) == 0);
	}
	{
		const char* key = ini_get_entry_key(ini, 1, 0);
		CHECK(str_cmp(key,"name",5) == 0);
	}
	{
		const char* value = ini_get_entry_value(ini, 1, 0);
		CHECK(str_cmp(value, "fky", 4) == 0);
	}
	CHECK(ini_find_sectn(ini,"") == 0);
	CHECK(ini_find_sectn(ini, 0) == ((qty_t)-1));
	CHECK(ini_find_entry(ini, 0,"a") == 0);
	CHECK(ini_find_entry(ini, 1, "a") == ((qty_t)-1));
	/* 修改 */
	ini_set_sectn_name(ini, 0, "6666");
	CHECK(str_cmp(ini_get_sectn_name(ini,0), "6666", 5) == 0);
	ini_set_entry_key(ini, 1, 0, "you name");
	CHECK(str_cmp(ini_get_entry_key(ini, 1,0), "you name", sizeof("you name")) == 0);
	ini_set_entry_value(ini, 1, 0, "deepssek");
	CHECK(str_cmp(ini_get_entry_value(ini, 1, 0), "deepssek", sizeof("deepssek")) == 0);

	ini_remove_entry(ini, 1, 1);
	CHECK(ini_find_entry(ini, 1, "age") == (qty_t)-1);
	ini_remove_sectn(ini, 2);
	CHECK(ini_find_sectn(ini, "5") == (qty_t)-1);

	{
		ini_insert_sectn(ini, "i love is?");
		qty_t id = ini_find_sectn(ini, "i love is?");
		CHECK(ini_insert_entry(ini, id, "name", "deepseek") == 1);
		printf("i love %s.\n", ini_get_entry_value(ini, id, 0));
	}
	/* 保存 */
	{
		char* p = malloc(4096);
		CHECK(p != 0);
		if (p == 0) { a_ini_del(arena); return 1; }
		CHECK(ini_save(ini, p, 4096) == 1);
		printf("%s\n", p);
		CHECK(ini_safe(p, strlen(p) + 1) == ERROR_NULL);

		ini_set_entry_key(ini, 1, 0, "you \nname");
		CHECK(ini_save(ini, p, 4096) == 0);
		free(p);
	}

	if (failures)
	{
		printf("as tests: FAIL (%d)\n", failures);
		a_ini_del(arena);
		return 1;
	}
	printf("as tests: PASS\n");
	a_ini_del(arena);
	return 0;
}