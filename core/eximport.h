#ifndef CORE_EXIMPORT_H
#define CORE_EXIMPORT_H


/* 导出导入 */
#define PORT_HIDE
#define PORT_API
#if defined(PORT_OPEN)
    #if defined(__clang__) || defined(__GNUC__)
        #define PORT_API   __attribute__((visibility("default")))
        #define PORT_HIDE   __attribute__((visibility("hidden")))
    #elif defined(_MSC_VER)
    #if defined(PORT_EX)
        #define PORT_API __declspec(dllexport)
    #else
        #define PORT_API __declspec(dllimport)
    #endif
    #endif
#endif

/* CPP兼容 */
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#ifdef __cplusplus
	#define EXTERN_C_BEGIN extern "C" {
	#define EXTERN_C_END   }
#endif

/* 函数内联 */
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE 
#endif


#endif