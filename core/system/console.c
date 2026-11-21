#include "console.h"
#if COM_USE(CONFIG_SYSTEM_CONSOLE)


#include "core/system/STATIC/base.h"

#define SYSTEM_CONSOLE_MAX 3

PORT_HIDE static bool_t console_y = 0;
PORT_HIDE static system_handle_h console_h[SYSTEM_CONSOLE_MAX] = { 0 };

/* 初始化控制台 */
PORT_API bool_t system_console_init(void)
{
	if (console_y) return 1;
	console_y = 1;
#if PLATFORM_WINDOWS
	console_h[0] = (system_handle_h)GetStdHandle(STD_INPUT_HANDLE);
	console_h[1] = (system_handle_h)GetStdHandle(STD_OUTPUT_HANDLE);
	console_h[2] = (system_handle_h)GetStdHandle(STD_ERROR_HANDLE);
	return 1;
#elif PLATFORM_POSIX
	console_h[0] = (system_handle_h)dup(0);
	console_h[1] = (system_handle_h)dup(1);
	console_h[2] = (system_handle_h)dup(2);
	if ((int)console_h[0] < 0 || (int)console_h[1] < 0 || (int)console_h[2] < 0)
		return ERR_FAIL;
	return 1;
#else
	return 0;
#endif
}
/*
* 重置控制台句柄
* 0read 1write 2error
*/
PORT_API bool_t system_console_reset(qty_t which)
{
	if (which >= SYSTEM_CONSOLE_MAX) return 0;
	if (console_y == 0) system_console_init();
	return system_console_set(which, console_h[which]);
}
/* 设置控制台句柄 */
PORT_API bool_t system_console_set(qty_t which, system_handle_h handle)
{
	if (which >= SYSTEM_CONSOLE_MAX) return 0;
	if (console_y == 0) system_console_init();
#if PLATFORM_WINDOWS
	DWORD id = which == 0 ? STD_INPUT_HANDLE
		: which == 1 ? STD_OUTPUT_HANDLE
		: STD_ERROR_HANDLE;
	return SetStdHandle(id, (HANDLE)handle) ? 1 : 0;
#elif PLATFORM_POSIX
	return dup2((int)handle, (int)which) >= 0 ? 1 : 0;
#else
	return 0;
#endif
}
/* 获取控制台句柄 */
PORT_API system_handle_h system_console_get(qty_t which)
{
	if (which >= SYSTEM_CONSOLE_MAX) return 0;
	if (console_y == 0) system_console_init();
#if PLATFORM_WINDOWS
	DWORD id = which == 0 ? STD_INPUT_HANDLE
		: which == 1 ? STD_OUTPUT_HANDLE
		: STD_ERROR_HANDLE;
	return (system_handle_h)GetStdHandle(id);
#elif PLATFORM_POSIX
	return (system_handle_h)which;
#else
	return 0;
#endif
}
/* 读取句柄 buf输出输入流,count字节数量 buf_size读取的字节数 */
PORT_API bool_t system_console_read(system_handle_h fp, void* buf, qty_t count, qty_t* buf_size)
{
	if (fp == 0 || buf == 0 || buf_size == 0) return 0;
	*buf_size = 0;
	if (count == 0) return 1;
	unsigned char* buf_tmp = buf;
#if PLATFORM_WINDOWS
	while (count > 0)
	{
		DWORD bytesRead = 0;
		DWORD bytesToRead = (count > ((DWORD)-1)) ? ((DWORD)-1) : (DWORD)count;
		bool_t ret = (bool_t)ReadFile(fp, buf_tmp, bytesToRead, &bytesRead, NULL);
		if (ret == 0)
		{
			if (*buf_size > 0)return 1;
			return 0;
		}
		buf_tmp += bytesRead;
		count -= bytesRead;
		*buf_size += (qty_t)bytesRead;
		if (bytesRead == 0) break;
	}
	return 1;
#elif PLATFORM_POSIX
	while (count > 0)
	{
		ssize_t ret = read((int)(qty_t)fp - 1, buf_tmp, (size_t)count);
		if (ret < 0)
		{
			if (*buf_size) return 1;
			return 0;
		}
		buf_tmp += ret;
		*buf_size += (qty_t)ret;
		count -= (qty_t)ret;
		if (ret == 0) break;
	}
	return 1;
#else
	return 0;
#endif
}
/* 写入句柄 */
PORT_API bool_t system_console_write(system_handle_h fp, const void* buf, qty_t count, qty_t* buf_size)
{
	if (fp == 0 || buf == 0 || buf_size == 0) return 0;
	*buf_size = 0;
	if (count == 0) return 1;
	const unsigned char* buf_tmp = buf;
#if PLATFORM_WINDOWS
	while (count > 0)
	{
		DWORD bytesWritten = 0;
		DWORD bytesToWrite = (count > ((DWORD)-1)) ? ((DWORD)-1) : (DWORD)count;
		bool_t ret = (bool_t)WriteFile(fp, buf_tmp, bytesToWrite, &bytesWritten, NULL);
		if (ret == 0)
		{
			if (*buf_size > 0)return 1;
			return 0;
		}
		buf_tmp += bytesWritten;
		count -= bytesWritten;
		*buf_size += (qty_t)bytesWritten;
		if (bytesWritten == 0) break;
	}
	return 1;
#elif PLATFORM_POSIX
	while (count > 0)
	{
		ssize_t ret = write((int)(qty_t)fp - 1, buf_tmp, (size_t)count);
		if (ret < 0)
		{
			if (*buf_size > 0) return 1;
			return 0;
		}
		*buf_size += (qty_t)ret;
		buf_tmp += ret;
		count -= (qty_t)ret;
		if (ret == 0) break;
	}
	return 1;
#else
	return 0;
#endif
}


#else


PORT_API bool_t system_console_init(void) { ; }
PORT_API bool_t system_console_reset(qty_t which) { ; }
PORT_API bool_t system_console_set(qty_t which, system_handle_h handle) { ; }
PORT_API system_handle_h system_console_get(qty_t which) { ; }
PORT_API bool_t system_console_read(system_handle_h fp, void* buf, qty_t count, qty_t* buf_size) { ; }
PORT_API bool_t system_console_write(system_handle_h fp, const void* buf, qty_t count, qty_t* buf_size) { ; }


#endif