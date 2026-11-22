#ifndef STRUCT_STRING_H
/*
* 字符串操作
* 字符串需要保证以'\0'为结尾
* 变量需求{0 1}时 传值视为真假
* n为长度单位 未额外说明及视为全部
*/
#define STRUCT_STRING_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 返回字符串的长度 不包括结尾的'\0' 最多检查n个字节 */
PORT_API qty_t str_len(const char* dst, qty_t n);
/* 只比较前n个字符或直到出现'\0' 返回目标与源之差 */
PORT_API cmp_t str_cmp(const char* dst, const char* src, qty_t n);

/*
* 复制src到dst 并保证在最终字符串末尾以'\0'结尾
* n为复制长度 n包括'\0' 剩余保持原样
*/
PORT_API char* str_cpy(char* dst, const char* src, qty_t n);
/*
* src追加到dst末尾 并保证在最终字符串末尾添加'\0'
* n为追加长度 n包括'\0'
*/
PORT_API char* str_cat(char* dst, const char* src, qty_t n);

/*
* 在字符串dst中查找src中任意一个字符第一次出现的位置 返回指向该位置的指针 若未找到则返回0
* dir为查询方向 0为正向 1为反向
*/
PORT_API char* str_chr(const char* dst, const char* src, qty_t dir);
/*
* 在字符串dst中查找子串src第一次出现的位置 若src为空字符串则返回dst 若未找到则返回0
* n为子串src的查询长度
*/
PORT_API char* str_str(const char* dst, const char* src, qty_t n);
/*
* 计算dst中是否包含src中任意字符的初始段长度 若未找到则返回0
* is为包含关系 0为包含 1为不包含
*/
PORT_API qty_t str_span(const char* dst, const char* src, qty_t is);

/*
* 传入dst目标字符串和tok分隔符字符串 不修改 返回token的长度
* 每个分隔符都是独立边界 连续分隔符会产出长度为0的token
* dst是指向目标字符串指针的指针 会将其更新为返回token之后的起始地址
* table为tok是否为表用于跳过内部创建 0为不是 1为是
* 若没有分隔符 则返回剩余字符串长度 并将dst置为0
*/
PORT_API qty_t str_tok(const char** dst, const char* tok, qty_t table);
/*
* 反转并修改字符串头尾顺序 返回字符串地址
* n为反转长度 反转长度不能超过'\0'
*/
PORT_API char* str_rev(char* dst,qty_t n);
/*
* 传入dst和src用于建造跳过内建表 dst会先清空
* dst大于单字符表示上限时 大于部分不构建
* size表明dst表的大小 当src有字符大于等于表size时 返回0
* 成功时返回表
*/
PORT_API char* str_table(char* dst, const char* src,qty_t size);


EXTERN_C_END
#endif