#ifndef STRUCT_VAR_H
/*
* 泛类型变量
* token是var的基础 不存在时失败
* 当存在自定义策略时 使用自定义替换默认
*/
#define STRUCT_VAR_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN

/* 类型成员 */
struct var_member
{
	/* 成员名称 允许存在一个无名成员(无字符串) */
	char* name;
	/* 成员偏移量 */
	qty_t off;
	/* 成员大小 */
	qty_t size;
	/* 以下为标志 互斥 */
	/* 是否为指针 */
	char Ptr;
	/* 是否为整数 */
	char Int;
	/* 是否为浮点数 */
	char Float;
};
/* 泛类型变量(前置声明, 供策略函数表引用) */
struct var;
/* 自定义泛类型变量策略函数表 存在时替换默认行为 */
struct var_ops
{
	/* 拷贝 深拷贝由用户实现 默认浅拷贝行为 */
	void* (*cpy)(struct var* dst, struct var* src);
	/* 销毁 如果var内部有动态资源 纯值类型填空 */
	void  (*des)(struct var* var);
	/* 比较判断逻辑是否相等 返回0表示相等 */
	cmp_t(*cmp)(struct var* var1, struct var* var2);
	/* 生成哈希 */
	qty_t(*hash)(struct var* var);
	/* 生成可视字符串 用于调试/日志 */
	char* (*to_string)(struct var* var, void* buff, qty_t size);
};
/* 泛类型变量生成令牌 */
struct var_token
{
	/* 类型名称 */
	char* name;
	/* 类型大小 */
	qty_t size;
	/* 类型结构 允许末尾存在一个柔性数组成员 */
	struct var_member* member;
	/* 成员数量 */
	qty_t count;
	/* 类型方法 */
	struct var_ops ops;
};
/* 泛类型变量 */
struct var
{
	/* 令牌 */
	struct var_token* token;
	/* 大小 */
	qty_t cap;
	/* 数据 */
	void* data;
};

/*
* 初始化ptr为var泛类型变量
* token为类型令牌 cap为data的大小
* 当使用柔性数组特性时 ptr应有var+data大小
* data拥有大小时应当满足token 否则失败
*/
PORT_API struct var* var_init(void* ptr, struct var_token* token, qty_t cap);
/*
* 扩展var 当采用柔性数组特性时不应使用当前函数
* 当传递old时获取旧指针
* 需要传递新data的指针和大小
*/
PORT_API bool_t var_ext(struct var* var, void** old, void* ptr, qty_t cap);
/* 仅重新设置变量的类型 当无法承载新类型时失败 数据亲和性不检查 */
PORT_API bool_t var_retag(struct var* var, const struct var_token* token);
/* 清空变量内容 */
PORT_API void var_clear(struct var* var);

/* 获取变量成员的下标 失败返回成员数量及以上 */
PORT_API qty_t var_find(struct var* var, const char* name);
/* 获取变量成员的指针 失败返回空指针 */
PORT_API void* var_ptr(struct var* var, qty_t index);
/* 获取变量成员的值 要求dst容量不小于成员大小 否则失败 */
PORT_API bool_t var_get(struct var* var, qty_t index, void* dst, qty_t size);
/* 设置变量成员的值 仅写入src的前size字节 要求size不大于成员大小 否则失败 */
PORT_API bool_t var_set(struct var* var, qty_t index, const void* src, qty_t size);

/* 获取泛变量的大小 */
PORT_API qty_t var_size(const struct var_token* token);
/* 获取成员数量 成功返回正整数 */
PORT_API qty_t var_count(const struct var_token* token);
/* 获取成员元信息 修改会导致token损坏 */
PORT_API const struct var_member* var_member(const struct var_token* token, qty_t index);

/* 复制变量 返回目标 无实现浅拷贝 */
PORT_API struct var* var_cpy(struct var* dst, struct var* src);
/* 比较两个变量 返回差值 */
PORT_API cmp_t var_cmp(struct var* dst, struct var* src);
/* 生成哈希值 FNV-1a */
PORT_API qty_t var_hash(struct var* var);
/* 将变量转换为可视的字符串并返回
 * 自定义 to_string 优先; 默认格式为十六进制:
 *   token_name{
 *   [member:hex]
 *   [:hex]          // 成员名为 0 或空字符串时省略名称
 *   }
 * hex 按字节序(内存字节顺序)转换: 从低地址到高地址, 每字节两位小写十六进制
 * 因此小端序下 0x16afde35 会输出为 35deaf16; 无数据时为空
 */
PORT_API char* var_to_string(struct var* var, void* buff, qty_t size);

/* 检验token是否合法并返回序列化大小 */
PORT_API qty_t var_token_safe(struct var_token* token);
/*
 * 序列化token树为相对二进制
 * 部分无法相对偏移数据会被置空 例如 ops需要重新绑定
 * buff 外部提供的内存
 * size 如果buff不为NULL 表示容量上限
 * 失败返回0 成功返回序列化实际写入的字节数
 */
PORT_API qty_t var_token_save(struct var_token* token, void* buff, qty_t size);
/*
 * 从相对二进制数据中原地恢复token
 * 部分无法相对偏移的置空数据无法恢复
 * buff 包含二进制数据的可写内存块
 * size 数据块大小
 * 恢复出的token指针 失败返回0
 */
PORT_API struct var_token* var_token_load(void* buff, qty_t size);


EXTERN_C_END
#endif