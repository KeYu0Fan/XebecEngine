#ifndef CORE_COMDEF_H
#define CORE_COMDEF_H


/* char最高位 */
#define COM_BYTE_HIGH 0x80
/* char最低位 */
#define COM_BYTE_LOW 0x01
/* 单字节位数 */
#define COM_BYTE_BIT 8
/* 重复构造单字节 */
#define COM_BYTE_REPEAT 0x0101010101010101ULL

/* 内存行缓冲大小 这是一个优化判断标准 */
#define COM_SMALL_SIZE 32
/* 哈希初始值 */
#define COM_HASH_SEED (0xcbf29ce484222325ULL)
/* 哈希混淆值 */
#define COM_HASH_MUL  (0x100000001b3ULL)

/* 标准行为 */
#define COM_STD 1
/* 优化行为 */
#define COM_PREF 1
/* 立即清理 */
#define COM_CLEAR 1
/* 调式状态 */
#define COM_DEBUG 1
/* 无别名提示|restrict */
#define COM_RESTRICT 0
/* 启用对齐支持 */
#define COM_ALIGN 1

/* 参数检查返回行为 */
#define COM_RETURN(cmp, ret) do{ if( cmp ) return ret; }while(0)
/* 手动内联代码 */
#define COM_DO(code) do{ code }while(0)
/* 重复任意位模式 */
#define COM_REPEAT_PATTERN(multiplier, type, pattern) ((type)multiplier * (type)pattern)
/* 拼接 */
#define COM_CAT_(prefix, data)  prefix ## data
#define COM_CAT(prefix, data)  COM_CAT_(prefix, data)
/* 计算未知类型单元大小 按void*计算 会向上取整 */
#define COM_UNITS(size) (((size) + sizeof(void*) - 1) / sizeof(void*))
/* 编译条件静态检查 */
#define COM_STATIC_ASSERT(cmp, name) typedef char COM_CAT(name, __LINE__)[ (cmp) ? 1 : -1]
/* 编译条件动态检查 为假触发 */
#define COM_ASSERT(cmp, call) COM_DO(if(!(cmp)) { call(); })
/* 模块启用 */
#define COM_USE(def)  1


#if (COM_RESTRICT == 0)
#if defined(PLATFORM_RESTRICT)
#undef PLATFORM_RESTRICT
#endif
#define PLATFORM_RESTRICT
#endif
#if (COM_ALIGN == 1)
#if defined(PLATFORM_ALIGN)
#undef PLATFORM_ALIGN
#endif
#define PLATFORM_ALIGN 1
#endif


#endif