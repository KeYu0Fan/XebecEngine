#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "core/struct/string.h"
#include "core/struct/array.h"
#include "core/struct/list.h"
#include "core/struct/hash.h"
#include "core/struct/buffer.h"
#include "core/struct/var.h"
#include "core/struct/life.h"
#include "core/struct/arena.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static cmp_t int_cmp(void* a, void* b)
{
	return *(int*)a - *(int*)b;
}

struct hash_entry
{
	struct hash_node node;
	int key;
	int val;
};
static qty_t hash_key(void* key) { return (qty_t)(*(int*)key); }
static void* hash_get(void* host) { return &((struct hash_entry*)host)->key; }
static cmp_t hash_cmp(void* a, void* b) { return *(int*)a - *(int*)b; }

struct list_item
{
	struct list_node node;
	int v;
};

struct life_host
{
	struct life l;
	int dead;
};
static void life_on_des(void* host) { ((struct life_host*)host)->dead = 1; }

static struct var_member g_members[2] = {
	{ "x", 0, 4, 0, 0, 0 },
	{ "y", 4, 4, 0, 0, 0 },
};
static struct var_token g_token = { "vec2", 8, g_members, 2, { 0 } };

static void test_string(void)
{
	char sbuf[64];
	const char* tok = "a,b,c";
	CHECK(str_len("abc", (qty_t)-1) == 3);
	CHECK(str_cmp("abc", "abd", 3) < 0);
	CHECK(str_cpy(sbuf, "hello", 6) == sbuf);
	CHECK(str_cmp(sbuf, "hello", 6) == 0);
	CHECK(str_cat(sbuf, " world", 7) != 0);
	CHECK(str_cmp(sbuf, "hello world", 12) == 0);
	CHECK(str_chr(sbuf, "w", 0) != 0);
	CHECK(str_str(sbuf, "world", 5) != 0);
	CHECK(str_span("abc123", "abc", 0) == 3);
	CHECK(str_tok(&tok, ",", 0) == 1);
	CHECK(str_rev(sbuf, 5) == sbuf);
	CHECK(str_cmp(sbuf, "olleh world", 12) == 0);
}

static void test_array(void)
{
	char store[sizeof(struct array) + sizeof(int) * 8];
	struct array* arr = array_init(store, sizeof(int), 8);
	int i;
	int v;
	CHECK(arr != 0);
	for (i = 0; i < 5; i++)
	{
		v = i * 10;
		CHECK(array_push(arr, &v));
	}
	CHECK(arr->count == 5);
	v = 0;
	CHECK(array_get(arr, &v, 2) != 0 && v == 20);
	v = 99;
	CHECK(array_set(arr, &v, 1));
	CHECK(array_get(arr, &v, 1) != 0 && v == 99);
	CHECK(array_find(arr, &v, int_cmp) == 1);
	CHECK(array_remove(arr, 0));
	CHECK(arr->count == 4);
	CHECK(array_sort(arr, int_cmp));
	{
		int prev = 0;
		CHECK(array_get(arr, &prev, 0) != 0);
		for (i = 1; i < (int)arr->count; i++)
		{
			int cur = 0;
			CHECK(array_get(arr, &cur, (qty_t)i) != 0);
			CHECK(prev <= cur);
			prev = cur;
		}
	}
	CHECK(array_pop(arr));
	CHECK(arr->count == 3);
	array_clear(arr);
	CHECK(arr->count == 0);
}

static void test_hash(void)
{
	struct hash h;
	struct hash_ops ops = { hash_key, hash_get, hash_cmp };
	void* buckets[8];
	struct hash_entry e[4];
	int i;
	int key;
	CHECK(hash_init(&h, offsetof(struct hash_entry, node), &ops) != 0);
	CHECK(hash_ext(&h, 0, buckets, 8));
	for (i = 0; i < 4; i++)
	{
		e[i].key = i;
		e[i].val = i * 2;
		CHECK(hash_insert(&h, &e[i].node));
	}
	CHECK(hash_count(&h) == 4);
	CHECK(!hash_empty(&h));
	key = 2;
	{
		struct hash_entry* found = (struct hash_entry*)hash_find(&h, &key);
		CHECK(found != 0 && found->val == 4);
	}
	CHECK(hash_remove(&h, &key));
	CHECK(hash_count(&h) == 3);
	CHECK(!hash_remove(&h, &key));
	hash_clear(&h);
	CHECK(hash_count(&h) == 0);
}

static void test_list(void)
{
	struct list list;
	struct list_item it[3];
	int i;
	struct list_node* n;
	CHECK(list_init(&list, offsetof(struct list_item, node), 0) != 0);
	for (i = 0; i < 3; i++)
	{
		it[i].v = i;
		CHECK(list_push(&list, &it[i].node, 1));
	}
	CHECK(list_count(&list) == 3);
	n = list_pop(&list, 0);
	CHECK(n == &it[0].node);
	CHECK(list_count(&list) == 2);
	CHECK(list_remove(&list, &it[2].node));
	CHECK(list_count(&list) == 1);
	list_rev(&list);
	CHECK(list_count(&list) == 1);
	CHECK(list_parent(&list, list_end(&list, 0)) == &it[1]);
	list_clear(&list);
	CHECK(list_empty(&list));
}

static void test_buffer(void)
{
	struct buffer b;
	char data[8];
	char rd[4] = { 0 };
	unsigned char c = 0;
	CHECK(buffer_init(&b, 0) != 0);
	CHECK(buffer_ext(&b, 0, data, 8));
	CHECK(buffer_write(&b, "abc", 3));
	CHECK(buffer_rsize(&b) == 3);
	CHECK(buffer_read(&b, rd, 3) == 3);
	CHECK(memcmp(rd, "abc", 3) == 0);
	CHECK(buffer_push(&b, 'x'));
	CHECK(buffer_pop(&b, &c));
	CHECK(c == 'x');
	buffer_clear(&b);
	CHECK(buffer_rsize(&b) == 0);
}

static void test_life(void)
{
	struct life_host host;
	memset(&host, 0, sizeof(host));
	CHECK(life_init(&host, offsetof(struct life_host, l), life_on_des) != 0);
	CHECK(life_count(&host.l) == 1);
	CHECK(life_add(&host.l));
	CHECK(life_count(&host.l) == 2);
	CHECK(life_sub(&host.l));
	CHECK(!host.dead);
	CHECK(life_sub(&host.l));
	CHECK(host.dead);
	CHECK(life_count(&host.l) == 0);
}

static void test_arena(void)
{
	char buf[4096];
	struct arena* ar = arena_init(buf, sizeof(buf), 512);
	void* p;
	void* q;
	CHECK(ar != 0);
	p = arena_new(ar, 100);
	CHECK(p != 0);
	memset(p, 0xAB, 100);
	CHECK(arena_curt(ar) >= 100);
	q = arena_anew(ar, 64, 16);
	CHECK(q != 0);
	CHECK(((qty_t)(uintptr_t)q % 16) == 0);
	arena_del(ar, p);
	arena_del(ar, q);
	CHECK(arena_live(ar) == 0);
}

static void test_var(void)
{
	char store[sizeof(struct var) + 8];
	char store2[sizeof(struct var) + 8];
	char text[256];
	struct var* v = var_init(store, &g_token, 8);
	struct var* v2;
	int x = 0x11223344;
	int y = 0x55667788;
	int got = 0;
	CHECK(v != 0);
	CHECK(var_set(v, 0, &x, 4));
	CHECK(var_set(v, 1, &y, 4));
	CHECK(var_get(v, 1, &got, 4) && got == y);
	CHECK(var_find(v, "x") == 0);
	CHECK(var_find(v, "z") == 2);
	CHECK(var_count(&g_token) == 2);
	CHECK(var_size(&g_token) == 8);
	CHECK(var_to_string(v, text, sizeof(text)) != 0);
	CHECK(str_str(text, "vec2{", 5) != 0);
	CHECK(str_str(text, "[x:", 3) != 0);
	CHECK(var_hash(v) != 0);
	v2 = var_init(store2, &g_token, 8);
	CHECK(v2 != 0);
	CHECK(var_cpy(v2, v) != 0);
	CHECK(var_cmp(v, v2) == 0);
}

int main(void)
{
	test_string();
	test_array();
	test_hash();
	test_list();
	test_buffer();
	test_life();
	test_arena();
	test_var();
	if (failures)
	{
		printf("struct tests: FAIL (%d)\n", failures);
		return 1;
	}
	printf("struct tests: PASS\n");
	return 0;
}