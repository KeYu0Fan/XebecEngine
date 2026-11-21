#ifndef CORE_PLATFORM_H
#define CORE_PLATFORM_H


/* 系统平台 */
#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__)
    #define PLATFORM_POSIX  1
    #define PLATFORM_WINDOWS 0
#elif defined(_WIN32)
    #define PLATFORM_POSIX 0
    #define PLATFORM_WINDOWS 1
#else
    #define PLATFORM_POSIX 0
    #define PLATFORM_WINDOWS 0
#endif

/* 编译器类型 */
#if defined(__clang__)
    #define PLATFORM_CLANG 1
#elif defined(__GNUC__)
    #define PLATFORM_GCC 1
#elif defined(_MSC_VER)
    #define PLATFORM_MSVC 1
#endif

/* 调用约定 */
#if (defined _WIN64 || defined _WIN32)
	#define PLATFORM_CALL __stdcall
#elif ((defined(__GNUC__) || defined(__clang__)) && __GNUC__ >= 4)
	#define PLATFORM_CALL
#endif

/* 指令优化 */
#if defined(_MSC_VER)
#define PLATFORM_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define PLATFORM_RESTRICT __restrict__
#else
#define PLATFORM_RESTRICT
#endif

/* 非对齐支持 */
#if defined(__arm__) || defined(__thumb__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARMT)
#define PLATFORM_ALIGN 1
#else
#define PLATFORM_ALIGN 0
#endif


#endif