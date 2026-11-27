#ifndef AS_INI_H
/* ini
 * 插入设置等行为无合法验证 禁止传输含有非标准ini的字符串
 * 无转义能力
 *
 * 内存与所有权:
 * - 不复制整段 text; 只把 section 名/key/value 逐条复制进 arena.
 * - text 必须在 ini_load 调用期间可写且有效; 解析会原地写入分隔符并恢复.
 * - 加载失败时 ini_load 会调用 arena_clear(arena);
 *   arena 里不能同时放其他需要保留的对象, 否则会一起被清空.
 */
#define AS_INI_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* entry */
struct ini_entry
{
    struct ini_entry* next;
    char* key;
    char* value;
};
/* sectn */
struct ini_sectn
{
    struct ini_sectn* next;
    struct ini_entry* entry;
    qty_t entry_count;
    char* name;
};
/* ini */
struct ini
{
    struct arena* arena;
    qty_t sectn_count;
    struct ini_sectn* sectn;
};

/* 创建空白ini结构 */
PORT_API struct ini* ini_init(struct arena* arena);
/* 清空ini创建结构 非o1行为 */
PORT_API void ini_clear(struct ini* ini);
/* 检查text是否为合法的ini结构 */
PORT_API err_t ini_safe(const char* text, qty_t len);
/* 保存ini为可读取的字符串 如果保存导致再加载行为错误时返回失败 */
PORT_API bool_t ini_save(struct ini* ini, char* text, qty_t len);
/* 加载ini为可操作的结构 */
PORT_API struct ini* ini_load(struct arena* arena, const char* text, qty_t len);

/* 查找sectn的索引 找不到时返回((qty_t)-1) */
PORT_API qty_t  ini_find_sectn(struct ini* ini, const char* name);
/* 查找entry的索引 找不到时返回((qty_t)-1) */
PORT_API qty_t  ini_find_entry(struct ini* ini, qty_t sectn, const char* key);
/* 插入一个sectn 不能重名 */
PORT_API bool_t ini_insert_sectn(struct ini* ini, const char* name);
/* 插入一个entry 在同一个sectn中不能重名 */
PORT_API bool_t ini_insert_entry(struct ini* ini, qty_t sectn, const char* key, const char* value);
/* 删除一个sectn */
PORT_API bool_t ini_remove_sectn(struct ini* ini, qty_t sectn);
/* 删除一个entry */
PORT_API bool_t ini_remove_entry(struct ini* ini, qty_t sectn, qty_t entry);

/* 设置当前sectn的名称 */
PORT_API bool_t ini_set_sectn_name(struct ini* ini, qty_t sectn, const char* name);
/* 设置当前sectn当前entry的key */
PORT_API bool_t ini_set_entry_key(struct ini* ini, qty_t sectn, qty_t entry, const char* key);
/* 设置当前sectn当前entry的value */
PORT_API bool_t ini_set_entry_value(struct ini* ini, qty_t sectn, qty_t entry, const char* value);
/* 获取当前sectn的名称 */
PORT_API const char* ini_get_sectn_name(struct ini* ini, qty_t sectn);
/* 获取当前sectn当前entry的key */
PORT_API const char* ini_get_entry_key(struct ini* ini, qty_t sectn, qty_t entry);
/* 获取当前sectn当前entry的value */
PORT_API const char* ini_get_entry_value(struct ini* ini, qty_t sectn, qty_t entry);

/* 获取sectn的数量 */
PORT_API qty_t ini_sectn_count(struct ini* ini);
/* 获取当前sectn的entry数量 */
PORT_API qty_t ini_entry_count(struct ini* ini, qty_t sectn);
/* 获取ini实例化可能的大小 */
PORT_API qty_t ini_size(const char* text, qty_t len);


EXTERN_C_END
#endif