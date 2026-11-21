#include "dynlib.h"
#if COM_USE(CONFIG_SYSTEM_DYNLIB)


#include "core/system/STATIC/base.h"
#include "core/system/thread.h"
#include "core/system/mmap.h"

#if PLATFORM_WINDOWS
#include "core/system/STATIC/string.h"
#elif PLATFORM_POSIX
/**/
#else
/**/
#endif

#if COM_USE(CONFIG_SYSTEM_THREAD)
PORT_HIDE static system_tls_key system_dynlib_key = 0;
PORT_HIDE static bool_t system_dynlib_var = 0;
PORT_HIDE static void system_dynlib(void)
{
	system_dynlib_var = system_tls_key_init(&system_dynlib_key, 0);
}
#else
PORT_HIDE static void* system_dynlib_key = 0;
#endif

/* 加载动态库 */
PORT_API system_dynlib_t system_dynlib_load(const char* path)
{
	if (path == 0 || *path == '\0')return 0;
#if COM_USE(CONFIG_SYSTEM_THREAD)
	static system_once_init tmp_once = { 0 };
	if (system_dynlib_var == 0)system_once_run(&tmp_once, &system_dynlib);
#endif
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t prot = { 0 };
	prot.read = 1;
	prot.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* tmp_path = (wchar_t*)system_mmap_new(0, size, prot, flags);
	if (tmp_path == 0) return 0;
#else
	wchar_t tmp_path[SYSTEM_PATH_MAX] = { 0 };
#endif
	void* tmp_handle = 0;
	if (system_string_to_native(tmp_path, path, size))
	{
		tmp_handle = (void*)LoadLibraryW(tmp_path);
		if (tmp_handle == 0)
		{
#if COM_USE(CONFIG_SYSTEM_THREAD)
			system_tls_key_set(system_dynlib_key, (void*)(qty_t)(GetLastError()));
#else
			system_dynlib_key = (void*)(qty_t)GetLastError();
#endif
		}
	}
	else
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)(qty_t)ERROR_INVALID_PARAMETER);
#else
		system_dynlib_key = (void*)(qty_t)ERROR_INVALID_PARAMETER;
#endif
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(tmp_path, size);
#endif
	return (system_dynlib_t)tmp_handle;
#elif PLATFORM_POSIX
	void* tmp_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
	if (tmp_handle == 0)
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)dlerror());
#else
		system_dynlib_key = (void*)dlerror();
#endif
		return 0;
	}
	return (system_dynlib_t)tmp_handle;
#else
	return 0;
#endif

}
/* 查看是否已加载动态库 */
PORT_API system_dynlib_t system_dynlib_is_loaded(const char* path)
{
	if (path == 0 || *path == '\0')return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t prot = { 0 };
	prot.read = 1;
	prot.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* tmp_path = (wchar_t*)system_mmap_new(0, size, prot, flags);
	if (tmp_path == 0) return 0;
#else
	wchar_t tmp_path[SYSTEM_PATH_MAX] = { 0 };
#endif
	system_dynlib_t ret = 0;
	if (system_string_to_native(tmp_path, path, size))
		ret = (system_dynlib_t)GetModuleHandleW(tmp_path);
	else
		/* 转换失败 尝试直接用 ANSI */
		ret = (system_dynlib_t)GetModuleHandleA(path);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(tmp_path, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	return (system_dynlib_t)dlopen(path, RTLD_NOLOAD | RTLD_NOW);
#else
	return 0;
#endif
}
/* 获取符号对应地址 */
PORT_API void* system_dynlib_symbol(system_dynlib_t lib, const char* name)
{
	if (lib == 0 || name == 0 || *name == '\0')return 0;
#if PLATFORM_WINDOWS
	void* tmp = (void*)GetProcAddress((HMODULE)lib, name);
	if (tmp == 0)
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)(qty_t)(GetLastError()));
#else
		system_dynlib_key = (void*)(qty_t)GetLastError();
#endif
		return 0;
	}
	return tmp;
#elif PLATFORM_POSIX
	void* tmp = dlsym((void*)lib, name);
	if (tmp == 0)
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)dlerror());
#else
		system_dynlib_key = (void*)dlerror();
#endif
		return 0;
	}
	return tmp;
#else
	return 0;
#endif
}
/* 卸载动态库 */
PORT_API bool_t system_dynlib_unload(system_dynlib_t lib)
{
	if (lib == 0)return 0;
#if PLATFORM_WINDOWS
	bool_t tmp = (bool_t)FreeLibrary((HMODULE)lib);
	if (tmp == 0)
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)(qty_t)(GetLastError()));
#else
		system_dynlib_key = (void*)(qty_t)GetLastError();
#endif
		return 0;
	}
	return 1;
#elif PLATFORM_POSIX
	bool_t tmp = (bool_t)dlclose((void*)lib);
	if (tmp != 0)
	{
#if COM_USE(CONFIG_SYSTEM_THREAD)
		system_tls_key_set(system_dynlib_key, (void*)dlerror());
#else
		system_dynlib_key = (void*)dlerror();
#endif
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}
/* 获取动态库错误 */
PORT_API const char* system_dynlib_error(void)
{
#if PLATFORM_WINDOWS
#if COM_USE(CONFIG_SYSTEM_THREAD)
	qty_t tmp = (qty_t)system_tls_key_get(system_dynlib_key);
#else
	qty_t tmp = (qty_t)(uintptr_t)system_dynlib_key;
#endif
	if (tmp == 0)return 0;
	switch (tmp)
	{
	case ERROR_FILE_NOT_FOUND: return "File not found";
	case ERROR_PATH_NOT_FOUND: return "Path not found";
	case ERROR_ACCESS_DENIED: return "Access denied";
	case ERROR_INVALID_HANDLE: return "Invalid handle";
	case ERROR_NOT_ENOUGH_MEMORY: return "Out of memory";
	case ERROR_INVALID_PARAMETER: return "Invalid parameter";
	case ERROR_MOD_NOT_FOUND: return "Module not found";
	case ERROR_PROC_NOT_FOUND: return "Procedure not found";
	case ERROR_BAD_EXE_FORMAT: return "Invalid DLL format";
	case ERROR_INVALID_ADDRESS: return "Invalid address";
	case ERROR_NOACCESS: return "Invalid memory access";
	case ERROR_DLL_INIT_FAILED: return "DLL initialization failed";
	default: return "Undef or Mapping error";
	}
#elif PLATFORM_POSIX
#if COM_USE(CONFIG_SYSTEM_THREAD)
	return (const char*)system_tls_key_get(system_dynlib_key);
#else
	return (const char*)system_dynlib_key;
#endif
#else
	return "Undef system dynlib error";
#endif
}


#else


PORT_API system_dynlib_t system_dynlib_load(const char* path) { ; }
PORT_API system_dynlib_t system_dynlib_is_loaded(const char* path) { ; }
PORT_API void* system_dynlib_symbol(system_dynlib_t lib, const char* name) { ; }
PORT_API bool_t system_dynlib_unload(system_dynlib_t lib) { ; }
PORT_API const char* system_dynlib_error(void) { ; }


#endif