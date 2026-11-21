#include "mmap.h"
#if COM_USE(CONFIG_SYSTEM_MMAP)


#include "core/system/STATIC/base.h"
#include "core/system/thread.h"

#if PLATFORM_WINDOWS
static system_once_init init = { 0 };
static HMODULE hKernel = (void*)0;
typedef DWORD(WINAPI* DiscardVirtualMemory_t)(PVOID, SIZE_T);
typedef BOOL(WINAPI* PrefetchVirtualMemory_t)(HANDLE, ULONG_PTR, PWIN32_MEMORY_RANGE_ENTRY, ULONG);
static PrefetchVirtualMemory_t pPrefetch = NULL;
static DiscardVirtualMemory_t pDiscard = NULL;
#elif PLATFORM_POSIX
/**/
#else
/**/
#endif

#if PLATFORM_WINDOWS
PORT_HIDE static DWORD system_mmap_set_prot(struct system_mmap_prot_t prot)
{

    if (prot.none)
        return PAGE_NOACCESS;
    if (prot.exec)
    {
        if (prot.write) return PAGE_EXECUTE_READWRITE;
        if (prot.read)  return PAGE_EXECUTE_READ;
        return PAGE_EXECUTE;
    }
    else
    {
        if (prot.write) return PAGE_READWRITE;
        if (prot.read)  return PAGE_READONLY;
        return PAGE_NOACCESS;
    }
}
#elif PLATFORM_POSIX
PORT_HIDE static int system_mmap_set_prot(struct system_mmap_prot_t prot)
{
    if (prot.none) return PROT_NONE;
    int p = 0;
    if (prot.read)  p |= PROT_READ;
    if (prot.write) p |= PROT_WRITE;
    if (prot.exec)  p |= PROT_EXEC;
    return p;
}
#endif

/* 分配内存 prot权限 flags标志位 */
PORT_API void* system_mmap_new(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags)
{
    if (size == 0)return 0;
    if (flags.fixed == 0)ptr = (void*)0;
#if PLATFORM_WINDOWS
    DWORD flProtect = { 0 };
    flProtect = system_mmap_set_prot(prot);
    if (flags.shared)
    {
        /* 无huge_tlb支持 */
        if (flags.huge_tlb) return 0;
        /* 共享匿名映射 */
        HANDLE hMap = CreateFileMappingA(
            INVALID_HANDLE_VALUE, NULL, flProtect | SEC_COMMIT,
            (DWORD)(((ULONG_PTR)size >> 32) & 0xFFFFFFFF),
            (DWORD)((ULONG_PTR)size & 0xFFFFFFFF), NULL);
        if (hMap == 0) return 0;
        DWORD desiredAccess;
        if (flProtect == PAGE_READWRITE || flProtect == PAGE_EXECUTE_READWRITE)
            desiredAccess = FILE_MAP_ALL_ACCESS;
        else if (flProtect == PAGE_READONLY || flProtect == PAGE_EXECUTE_READ)
            desiredAccess = FILE_MAP_READ;
        else if (flProtect == PAGE_EXECUTE)
            desiredAccess = FILE_MAP_EXECUTE;
        else
            desiredAccess = FILE_MAP_READ;
        void* addr = MapViewOfFileEx(hMap, desiredAccess, 0, 0, (SIZE_T)size, ptr);
        CloseHandle(hMap);
        if (addr == 0) return 0;
        if (flags.populate == 1)VirtualLock(addr, (SIZE_T)size);
        return addr;
    }
    /* 私有匿名映射 */
    DWORD allocFlags = MEM_COMMIT | MEM_RESERVE;
    if (flags.huge_tlb) allocFlags |= MEM_LARGE_PAGES;
    void* addr = VirtualAlloc(ptr, (SIZE_T)size, allocFlags, flProtect);
    if (addr == 0) return 0;
    if (flags.populate == 1)VirtualLock(addr, (SIZE_T)size);
    return addr;
#elif PLATFORM_POSIX
    int sys_prot = 0;
    sys_prot = system_mmap_set_prot(prot);
    int sys_flags = MAP_ANONYMOUS;
    if (flags.shared)
        sys_flags |= MAP_SHARED;
    else
        sys_flags |= MAP_PRIVATE;
    if (flags.fixed == 1)
#ifdef MAP_FIXED_NOREPLACE
        sys_flags |= MAP_FIXED_NOREPLACE;
#else
        return 0;
#endif
    if (flags.huge_tlb == 1)
        sys_flags |= MAP_HUGETLB;
    if (flags.populate == 1)
        sys_flags |= MAP_POPULATE;
    void* addr = mmap(ptr, (size_t)size, sys_prot, sys_flags, -1, 0);
    if (addr == MAP_FAILED) return 0;
    return addr;
#else
    return 0;
#endif
}
/* 释放内存 */
PORT_API bool_t system_mmap_del(void* ptr, qty_t size)
{
    if (ptr == 0)return 0;
#if PLATFORM_WINDOWS
    (void)size;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)return 0;
    if (mbi.Type == MEM_MAPPED)return UnmapViewOfFile(ptr) ? 1 : 0;
    if (mbi.Type == MEM_PRIVATE)return VirtualFree(ptr, 0, MEM_RELEASE) ? 1 : 0;
    return 0;
#elif PLATFORM_POSIX
    return (munmap(ptr, (size_t)size) == 0) ? 1 : 0;
#else
    return 0;
#endif
}
/* 修改内存权限 */
PORT_API bool_t system_mmap_protect(void* ptr, qty_t size, struct system_mmap_prot_t prot)
{
#if PLATFORM_WINDOWS
    DWORD flNewProtect = system_mmap_set_prot(prot);
    DWORD oldProtect;
    if (VirtualProtect(ptr, (SIZE_T)size, flNewProtect, &oldProtect))
        return 1;
    return 0;
#elif PLATFORM_POSIX
    int sys_prot = system_mmap_set_prot(prot);
    if (mprotect(ptr, (size_t)size, sys_prot) == 0)
        return 1;
    return 0;
#else
    return 0;
#endif
}
/* 调整内存大小 flags == 1允许移动 2固定新地址 4保留旧映射 允许叠加flags */
PORT_API void* system_mmap_resize(void* ptr, qty_t old_size, qty_t new_size, qty_t flags)
{
    if (ptr == 0 || old_size == 0 || new_size == 0) return 0;
#if PLATFORM_WINDOWS
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) return 0;
    /* 只支持私有内存 文件映射返回失败 */
    if (mbi.Type != MEM_PRIVATE) return 0;
    DWORD oldProtect = mbi.AllocationProtect;
    SIZE_T copy_size = (old_size < new_size) ? (SIZE_T)old_size : (SIZE_T)new_size;
    /* 分配新内存 使用相同的保护属性 */
    void* new_ptr = VirtualAlloc(NULL, (SIZE_T)new_size, MEM_COMMIT | MEM_RESERVE, oldProtect);
    if (new_ptr == 0) return 0;
    /* 拷贝 */
    {
        void* dst = new_ptr;
        void* src = ptr;
        qty_t size = (qty_t)copy_size;
        qty_t cmp = ((qty_t)(uintptr_t)dst | (qty_t)(uintptr_t)src) & (sizeof(qty_t) - 1);
        cmp = cmp ? sizeof(qty_t) - cmp : 0;
        switch (1)
        {
        case 1:
        {
            char* d = dst;
            char* s = src;
            qty_t m = (size < COM_SMALL_SIZE) ? size : cmp;
            qty_t n = m;
            while (n >= sizeof(char))
            {

                *d = *s;
                d++;
                s++;
                n--;
            }
            size = size - m;
            if (size == 0)break;
            dst = d;
            src = s;
#if	(COMMON_USE_ALIGN)
            cmp = ((qty_t)(uintptr_t)dst | (qty_t)(uintptr_t)src) & (sizeof(qty_t) - 1);
#else
            cmp = 0;
#endif
        }
        if (cmp == 0)
        {
            qty_t* d = dst;
            qty_t* s = src;
            while (size >= sizeof(qty_t))
            {
                *d = *s;
                d++;
                s++;
                size = size - sizeof(qty_t);
            }
            dst = d;
            src = s;
        }
        if (size)
        {
            char* d = dst;
            char* s = src;
            while (size >= sizeof(char))
            {
                *d = *s;
                d++;
                s++;
                size = size - sizeof(char);
            }
        }
        }
    }
    /* 如果未设置保留旧映射 则释放旧内存 */
    if ((flags & 4) == 0) VirtualFree(ptr, 0, MEM_RELEASE);
    return new_ptr;
#elif PLATFORM_POSIX
    int sys_flags = 0;
    if (flags & 1) sys_flags |= MREMAP_MAYMOVE;
    if (flags & 2) sys_flags |= MREMAP_FIXED;
    if (flags & 4)
#ifdef MREMAP_DONTUNMAP
        sys_flags |= MREMAP_DONTUNMAP;
#else
        return 0;
#endif
    void* new_ptr = mremap(ptr, (size_t)old_size, (size_t)new_size, sys_flags, NULL);
    if (new_ptr == MAP_FAILED) return 0;
    return new_ptr;
#else
    return 0;
#endif
}
/* 内存文件映射写回 flags == 1无效化缓存 2同步写回 4异步写回 允许叠加1 */
PORT_API bool_t system_mmap_sync(void* ptr, qty_t size, qty_t flags)
{
    if (ptr == 0)return 0;
#if PLATFORM_WINDOWS
    if (flags & 2)
    {
        if (FlushViewOfFile(ptr, (SIZE_T)size) == 0)
            return 0;
    }
    /* 异步或无效化不做任何操作 直接返回成功 */
    return 1;
#elif PLATFORM_POSIX
    int sys_flags = 0;
    if (flags & 1) sys_flags |= MS_INVALIDATE;
    if (flags & 2) sys_flags |= MS_SYNC;
    if (flags & 4) sys_flags |= MS_ASYNC;
    if (msync(ptr, (size_t)size, sys_flags) == 0)
        return 1;
    return 0;
#else
    return 0;
#endif
}
/* 内存加锁 */
PORT_API bool_t system_mmap_lock(void* ptr, qty_t size)
{
#if PLATFORM_WINDOWS
    if (VirtualLock(ptr, (SIZE_T)size))
        return 1;
    return 0;
#elif PLATFORM_POSIX
    if (mlock(ptr, (size_t)size) == 0)
        return 1;
    return 0;
#else
    return 0;
#endif
}
/* 内存解锁 */
PORT_API bool_t system_mmap_unlock(void* ptr, qty_t size)
{
#if PLATFORM_WINDOWS
    if (VirtualUnlock(ptr, (SIZE_T)size))
        return 1;
    return 0;
#elif PLATFORM_POSIX
    if (munlock(ptr, (size_t)size) == 0)
        return 1;
    return 0;
#else
    return 0;
#endif
}
PORT_HIDE static void system_mmap_advise_aux_00(void)
{
    if (hKernel == 0)
        hKernel = GetModuleHandleA("kernel32.dll");
    if (hKernel == 0)
        return;
    if (pPrefetch == 0)
        pPrefetch = (PrefetchVirtualMemory_t)GetProcAddress(hKernel, "PrefetchVirtualMemory");
    if (pDiscard == 0)
        pDiscard = (DiscardVirtualMemory_t)GetProcAddress(hKernel, "DiscardVirtualMemory");
}
/* 设置内存的策略行为 advice建议类型 */
PORT_API bool_t system_mmap_advise(void* ptr, qty_t size, enum system_mmap_advise_value advice)
{
    if (ptr == 0) return 0;
#if PLATFORM_WINDOWS
#if COM_USE(CONFIG_SYSTEM_THREAD)
    if (init == 0) system_once_run(&init, &system_mmap_advise_aux_00);
#endif
    /* Windows 8+ 提供了 PrefetchVirtualMemory 和 DiscardVirtualMemory 来模拟部分 advice */
    if (hKernel == 0)
        hKernel = GetModuleHandleA("kernel32.dll");
    if (hKernel == 0)return 0;
    switch (advice)
    {
    case SYSTEM_MMAP_ADVISE_WILLNEED:
    {
        /* Win8+ 预读内存页到物理内存 */
        if (pPrefetch == 0)
            pPrefetch = (PrefetchVirtualMemory_t)GetProcAddress(hKernel, "PrefetchVirtualMemory");
        if (pPrefetch)
        {
            WIN32_MEMORY_RANGE_ENTRY range = { ptr, (SIZE_T)size };
            if (pPrefetch(GetCurrentProcess(), 1, &range, 0))
                return 1;
            else
                return 0;
        }
        /* 无论函数是否存在 都视为成功 */
        return 1;
    }
    case SYSTEM_MMAP_ADVISE_DONTNEED:
    case SYSTEM_MMAP_ADVISE_FREE:
    {
        /* Win8.1+ 丢弃物理内存页 类似 MADV_DONTNEED / MADV_FREE */
        if (pDiscard == 0)
            pDiscard = (DiscardVirtualMemory_t)GetProcAddress(hKernel, "DiscardVirtualMemory");
        if (pDiscard)
            if (pDiscard(ptr, (SIZE_T)size) == ERROR_SUCCESS)
                return 1;
            else
                return 0;
        return 1;
    }
    default:
        /* NORMAL RANDOM SEQUENTIAL 在 Windows 下无等效 直接返回成功 */
        return 1;
    }
#elif PLATFORM_POSIX
    if (madvise(ptr, (size_t)size, (int)advice) == 0)
        return 1;
    if (advice == SYSTEM_MMAP_ADVISE_FREE)
        return 1;
    return 0;
#else
    return 0;
#endif
}
/* 检查内存的策略行为 vec == (size + page_size - 1) / page_size */
PORT_API bool_t system_mmap_residency(void* ptr, qty_t size, unsigned char* vec)
{
#if PLATFORM_WINDOWS
    /* 手动定义 PSAPI_WORKING_SET_EX_INFORMATION 和函数原型, 避免包含 <psapi.h> */
    typedef struct _PSAPI_WORKING_SET_EX_INFORMATION {
        PVOID  VirtualAddress;
        struct {
            ULONG Valid : 1;
            ULONG Reserved0 : 14;
            ULONG Shared : 1;
            ULONG Reserved1 : 16;
        } VirtualAttributes;
    } PSAPI_WORKING_SET_EX_INFORMATION;
    typedef BOOL(WINAPI* QueryWorkingSetEx_t)(
        HANDLE hProcess,
        PSAPI_WORKING_SET_EX_INFORMATION* pv,
        DWORD cb
        );
    /* 动态加载 psapi.dll 避免静态依赖 */
    static QueryWorkingSetEx_t pQueryWorkingSetEx = NULL;
    static HMODULE hPsapi = NULL;
    if (hPsapi == 0)
        hPsapi = LoadLibraryW(L"psapi.dll");
    if (hPsapi && pQueryWorkingSetEx == 0)
        pQueryWorkingSetEx = (QueryWorkingSetEx_t)GetProcAddress(hPsapi, "QueryWorkingSetEx");
    if ((hPsapi && pQueryWorkingSetEx) == 0)
        return 0;
    /* 获取页面大小 */
    SYSTEM_INFO si = { 0 };
    GetSystemInfo(&si);
    size_t page_size = si.dwPageSize;
    size_t page_count = ((size_t)size + page_size - 1) / page_size;
    /* 分配查询数组 使用 HeapAlloc 避免栈溢出 不依赖 CRT */
    PSAPI_WORKING_SET_EX_INFORMATION* ws_info =
        (PSAPI_WORKING_SET_EX_INFORMATION*)HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY,
            page_count * sizeof(PSAPI_WORKING_SET_EX_INFORMATION));
    if (ws_info == 0)
        return 0;
    /* 填充每个页的虚拟地址 */
    for (size_t i = 0; i < page_count; ++i)
        ws_info[i].VirtualAddress = (BYTE*)ptr + i * page_size;
    /* 查询工作集信息 */
    BOOL success = pQueryWorkingSetEx(
        GetCurrentProcess(), ws_info,
        (DWORD)(page_count * sizeof(PSAPI_WORKING_SET_EX_INFORMATION)));
    if (success)
    {
        /* 提取 Valid 位 */
        for (size_t i = 0; i < page_count; ++i)
            vec[i] = ws_info[i].VirtualAttributes.Valid ? 1 : 0;
    }
    /* 释放资源 */
    HeapFree(GetProcessHeap(), 0, ws_info);
    return success ? 1 : 0;
#elif PLATFORM_POSIX
    if (mincore(ptr, (size_t)size, vec) == 0) return 1;
    return 0;
#else
    return 0;
#endif
}
/* 映射文件 fd句柄 offset偏移 */
PORT_API void* system_mmap_file(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags, \
    system_handle_h fd, system_off_t offset)
{
    if (size == 0) return 0;
    if (flags.fixed == 0) ptr = (void*)0;
#if PLATFORM_WINDOWS
    /* 转换保护 */
    DWORD flProtect;
    flProtect = system_mmap_set_prot(prot);
    /* 文件映射不支持大页 */
    if (flags.huge_tlb) return 0;
    /* 根据共享/私有调整映射对象保护 */
    DWORD flMappingProtect = flProtect;
    if (flags.shared == 0)
    {
        /* 私有映射 将可写保护转换为写时拷贝 */
        if (flProtect == PAGE_READWRITE)
            flMappingProtect = PAGE_WRITECOPY;
        else if (flProtect == PAGE_EXECUTE_READWRITE)
            flMappingProtect = PAGE_EXECUTE_WRITECOPY;
    }
    /* 如果最终保护是 NOACCESS 映射无法访问 直接失败 */
    if (flMappingProtect == PAGE_NOACCESS) return 0;
    /* 创建文件映射对象 */
    HANDLE hMap = CreateFileMappingW(
        (HANDLE)fd, NULL, flMappingProtect | SEC_COMMIT,
        (DWORD)(((ULONG_PTR)size >> 32) & 0xFFFFFFFF),
        (DWORD)((ULONG_PTR)size & 0xFFFFFFFF), NULL);
    if (hMap == 0) return 0;
    /* 映射视图 */
    DWORD desiredAccess;
    switch (flMappingProtect)
    {
        /* 共享映射 */
    case PAGE_READWRITE:
    case PAGE_EXECUTE_READWRITE:
        desiredAccess = FILE_MAP_ALL_ACCESS;
        break;
        /* 私有映射 写时拷贝 */
    case PAGE_WRITECOPY:
    case PAGE_EXECUTE_WRITECOPY:
        desiredAccess = FILE_MAP_COPY;
        break;
    case PAGE_READONLY:
    case PAGE_EXECUTE_READ:
        desiredAccess = FILE_MAP_READ;
        break;
    case PAGE_EXECUTE:
        desiredAccess = FILE_MAP_EXECUTE;
        break;
    default:
        CloseHandle(hMap);
        return 0;
    }
    DWORD offHigh = (DWORD)(((ULONG_PTR)offset >> 32) & 0xFFFFFFFF);
    DWORD offLow = (DWORD)((ULONG_PTR)offset & 0xFFFFFFFF);
    void* addr = MapViewOfFileEx(hMap, desiredAccess, offHigh, offLow, (SIZE_T)size, ptr);
    CloseHandle(hMap);
    if (addr == 0) return 0;
    /* populate */
    if (flags.populate == 1) VirtualLock(addr, (SIZE_T)size);
    /* populate == 2 忽略 Windows 没有非阻塞映射 */
    return addr;
#elif PLATFORM_POSIX
    int sys_prot = 0;
    sys_prot = system_mmap_set_prot(prot);
    int sys_flags = 0;
    if (flags.shared)
        sys_flags |= MAP_SHARED;
    else
        sys_flags |= MAP_PRIVATE;
    if (flags.fixed == 1)
#ifdef MAP_FIXED_NOREPLACE
        sys_flags |= MAP_FIXED_NOREPLACE;
#else
        return 0;
#endif
    if (flags.huge_tlb)
        sys_flags |= MAP_HUGETLB;
    if (flags.populate == 1)
        sys_flags |= MAP_POPULATE;
    else if (flags.populate == 2)
        sys_flags |= MAP_NONBLOCK;
    void* addr = mmap(ptr, (size_t)size, sys_prot, sys_flags, (int)(qty_t)fd, (off_t)offset);
    if (addr == MAP_FAILED) return 0;
    return addr;
#else
    return 0;
#endif  
}


#else


PORT_API void* system_mmap_new(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags) { ; }
PORT_API bool_t system_mmap_del(void* ptr, qty_t size) { ; }
PORT_API bool_t system_mmap_protect(void* ptr, qty_t size, struct system_mmap_prot_t prot) { ; }
PORT_API void* system_mmap_resize(void* ptr, qty_t old_size, qty_t new_size, qty_t flags) { ; }
PORT_API bool_t system_mmap_sync(void* ptr, qty_t size, qty_t flags) { ; }
PORT_API bool_t system_mmap_lock(void* ptr, qty_t size) { ; }
PORT_API bool_t system_mmap_unlock(void* ptr, qty_t size) { ; }
PORT_API bool_t system_mmap_advise(void* ptr, qty_t size, enum system_mmap_advise_value advice) { ; }
PORT_API bool_t system_mmap_residency(void* ptr, qty_t size, unsigned char* vec) { ; }
PORT_API void* system_mmap_file(void* ptr, qty_t size, struct system_mmap_prot_t prot, struct system_mmap_flags_t flags, \
    system_handle_h fd, system_off_t offset) { ; }


#endif