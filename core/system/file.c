#include "file.h"
#if  COM_USE(CONFIG_SYSTEM_FILE)


#include "core/system/STATIC/base.h"
#include "core/system/STATIC/string.h"
#include "core/system/mmap.h"

/* File 文件 */

/* 打开文件,path路径,flags功能,mode权限 */
PORT_API system_file_t system_file_open(const char* path, struct system_file_flags_t flags, struct system_file_mode_t mode)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	(void)mode; /* Windows 无 mode 对应实现, 显式忽略 */
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	DWORD dwAccess = 0;
	if (flags.read) dwAccess |= GENERIC_READ;
	if (flags.write)
	{
		dwAccess |= GENERIC_WRITE;
		if (flags.add)
			dwAccess |= FILE_APPEND_DATA;
	}
	else if (flags.add)
		dwAccess |= FILE_APPEND_DATA;
	DWORD dwDisposition = OPEN_EXISTING;
	if (flags.cre)
	{
		if (flags.trunc)
			dwDisposition = CREATE_ALWAYS;
		else if (flags.exc)
			dwDisposition = CREATE_NEW;
		else
			dwDisposition = OPEN_ALWAYS;
	}
	else
	{
		if (flags.trunc)
			dwDisposition = TRUNCATE_EXISTING;
		else
			dwDisposition = OPEN_EXISTING;
	}
	DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
	if (flags.sync)   dwFlags |= FILE_FLAG_WRITE_THROUGH;
	if (flags.direct) dwFlags |= FILE_FLAG_NO_BUFFERING;
	HANDLE h = CreateFileW(wpath, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwDisposition, dwFlags, NULL);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	return (h == INVALID_HANDLE_VALUE) ? 0 : h;
#elif PLATFORM_POSIX
	int oflag = 0;
	if (flags.read && flags.write)
		oflag |= O_RDWR;
	else if (flags.read)
		oflag |= O_RDONLY;
	else if (flags.write)
		oflag |= O_WRONLY;
	else if (flags.add)
		oflag |= O_WRONLY;
	if (flags.cre) oflag |= O_CREAT;
	if (flags.exc) oflag |= O_EXCL;
	if (flags.trunc) oflag |= O_TRUNC;
	if (flags.add) oflag |= O_APPEND;
	if (flags.sync) oflag |= O_SYNC;
#ifdef O_DIRECT
	if (flags.direct) oflag |= O_DIRECT;
#endif
	mode_t m = 0;
	if (mode.read)  m |= S_IRUSR;
	if (mode.write) m |= S_IWUSR;
	if (mode.exec)  m |= S_IXUSR;
	switch (mode.other)
	{
	case 3:
		m |= S_IWGRP | S_IWOTH;
	case 2:
		m |= S_IXGRP | S_IXOTH;
	case 1:
		m |= S_IRGRP | S_IROTH;
	case 0:
	default:
		break;
	}
	int fd = open(path, oflag, m);
	if (fd < 0) return 0;
	return (system_file_t)(qty_t)(fd + 1);
#else
	return 0;
#endif
}
/* 关闭文件 fd句柄 */
PORT_API bool_t system_file_close(system_file_t fp)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	return CloseHandle(fp) ? 1 : 0;
#elif PLATFORM_POSIX
	return (close((int)(qty_t)fp - 1) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 读取文件,buf输出输入流,count字节数量 buf_size读取的字节数 */
PORT_API bool_t system_file_read(system_file_t fp, void* buf, qty_t count, qty_t* buf_size)
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
/* 写入文件 */
PORT_API bool_t system_file_write(system_file_t fp, const void* buf, qty_t count, qty_t* buf_size)
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
/* 文件指针移动,offset偏移量,whence偏移参考坐标(0头|1中|2尾) 失败返回 -1 */
PORT_API system_off_t system_file_lseek(system_file_t fp, system_off_t offset, qty_t whence)
{
	if (fp == 0)return -1;
#if PLATFORM_WINDOWS
	DWORD dwMoveMethod;
	switch (whence)
	{
	case 0: dwMoveMethod = FILE_BEGIN; break;
	case 1: dwMoveMethod = FILE_CURRENT; break;
	case 2: dwMoveMethod = FILE_END; break;
	default: return -1;
	}
	LARGE_INTEGER liDistance = { 0 };
	liDistance.QuadPart = offset;
	LARGE_INTEGER liNewPos;

	if (SetFilePointerEx(fp, liDistance, &liNewPos, dwMoveMethod) == 0) return -1;
	return (system_off_t)liNewPos.QuadPart;
#elif PLATFORM_POSIX
	int posix_whence = 0;
	switch (whence)
	{
	case 0: posix_whence = SEEK_SET; break;
	case 1: posix_whence = SEEK_CUR; break;
	case 2: posix_whence = SEEK_END; break;
	default: return -1;
	}
	off_t new_pos = lseek((int)(qty_t)fp - 1, (off_t)offset, posix_whence);
	if (new_pos == (off_t)-1) return -1;
	return (system_off_t)new_pos;
#else
	return -1;
#endif
}
/* 文件重命名 */
PORT_API bool_t system_file_rename(const char* oldpath, const char* newpath)
{
	if (oldpath == 0 || newpath == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t) * 2;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* oldW = system_mmap_new(0, size, port, flags);
	if (oldW == 0) return 0;
	wchar_t* newW = (wchar_t*)((char*)oldW + size / 2);
#else
	wchar_t oldW[SYSTEM_PATH_MAX] = { 0 };
	wchar_t newW[SYSTEM_PATH_MAX] = { 0 };
#endif
	bool_t ret = 0;
	if (system_string_to_native(oldW, oldpath, size / 2))
		if (system_string_to_native(newW, newpath, size / 2))
			ret = MoveFileExW(oldW, (wchar_t*)newW, MOVEFILE_REPLACE_EXISTING) ? 1 : 0;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	system_mmap_del(oldW, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	return (rename(oldpath, newpath) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 删除文件 */
PORT_API bool_t system_file_del(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	return DeleteFileW(wpath) ? 1 : 0;
#elif PLATFORM_POSIX
	return (unlink(path) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 获取文件信息填充结构体 */
PORT_API bool_t system_file_stat(system_file_t fp, struct system_file_info_t* info)
{
	if (fp == 0 || info == 0)return 0;
#if PLATFORM_WINDOWS
	BY_HANDLE_FILE_INFORMATION fi;
	if (GetFileInformationByHandle(fp, &fi) == 0) return 0;
	info->size = ((qty_t)fi.nFileSizeHigh << 32) | fi.nFileSizeLow;
	qty_t time_off = sizeof(DWORD) * 8;
	info->cre_time = (((qty_t)fi.ftCreationTime.dwHighDateTime << time_off) + (qty_t)fi.ftCreationTime.dwLowDateTime) / 10000 - SYSTEM_TIME_OFFSET_MS;
	info->mod_time = (((qty_t)fi.ftLastWriteTime.dwHighDateTime << time_off) + (qty_t)fi.ftLastWriteTime.dwLowDateTime) / 10000 - SYSTEM_TIME_OFFSET_MS;
	info->ass_time = (((qty_t)fi.ftLastAccessTime.dwHighDateTime << time_off) + (qty_t)fi.ftLastAccessTime.dwLowDateTime) / 10000 - SYSTEM_TIME_OFFSET_MS;
	info->nlink = fi.nNumberOfLinks;
	info->mode.read = 1;
	info->mode.write = (fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 0 : 1;
	info->mode.exec = 0;
	info->mode.other = 0;
	info->type = 0;
	if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		info->type = SYSTEM_FILE_TYPE_DIR;
	else if (fi.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		info->type = SYSTEM_FILE_TYPE_LINK;
	else
	{
		DWORD fileType = GetFileType(fp);
		info->type = (fileType == FILE_TYPE_PIPE) ? SYSTEM_FILE_TYPE_NAMEPIPE : SYSTEM_FILE_TYPE_REGULAR;
	}
	return 1;
#elif PLATFORM_POSIX
	struct stat st = { 0 };
	if (fstat((int)(qty_t)fp - 1, &st) != 0) return 0;
	info->size = (qty_t)st.st_size;
	info->cre_time = ((qty_t)st.st_ctim.tv_sec * 1000 + (qty_t)st.st_ctim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->mod_time = ((qty_t)st.st_mtim.tv_sec * 1000 + (qty_t)st.st_mtim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->ass_time = ((qty_t)st.st_atim.tv_sec * 1000 + (qty_t)st.st_atim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->nlink = (qty_t)st.st_nlink;
	info->mode.read = (st.st_mode & S_IRUSR) ? 1 : 0;
	info->mode.write = (st.st_mode & S_IWUSR) ? 1 : 0;
	info->mode.exec = (st.st_mode & S_IXUSR) ? 1 : 0;
	info->mode.other = 0;
	if (st.st_mode & S_IROTH) info->mode.other = 1;
	if (st.st_mode & S_IXOTH) info->mode.other = 2;
	switch (st.st_mode & S_IFMT)
	{
	case S_IFREG:  info->type = SYSTEM_FILE_TYPE_REGULAR; break;
	case S_IFDIR:  info->type = SYSTEM_FILE_TYPE_DIR; break;
	case S_IFLNK:  info->type = SYSTEM_FILE_TYPE_LINK; break;
	case S_IFCHR:  info->type = SYSTEM_FILE_TYPE_CHAR_DEV; break;
	case S_IFBLK:  info->type = SYSTEM_FILE_TYPE_BLOCK_DEV; break;
	case S_IFSOCK: info->type = SYSTEM_FILE_TYPE_SOCKET; break;
	case S_IFIFO:  info->type = SYSTEM_FILE_TYPE_NAMEPIPE; break;
	default:       info->type = SYSTEM_FILE_TYPE_UNDEF; break;
	}
	return 1;
#else
	return 0;
#endif
}
/* 文件同步 将缓冲区数据和元数据强制写入磁盘 */
PORT_API bool_t system_file_sync(system_file_t fp)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	return FlushFileBuffers(fp) ? 1 : 0;
#elif PLATFORM_POSIX
	return (fsync((int)(qty_t)fp - 1) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 修改文件权限 Linux 下设置精确权限, Windows 下仅影响只读属性 */
PORT_API bool_t system_file_chmod(system_file_t fp, struct system_file_mode_t mode)
{
	if (fp == 0) return 0;
#if PLATFORM_WINDOWS
	FILE_BASIC_INFO info = { 0 };
	if (GetFileInformationByHandleEx(fp, FileBasicInfo, &info, sizeof(info)) == 0)
		return 0;
	if (mode.write == 1)
		info.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
	else
		info.FileAttributes |= FILE_ATTRIBUTE_READONLY;
	return SetFileInformationByHandle(fp, FileBasicInfo, &info, sizeof(info)) ? 1 : 0;
#elif PLATFORM_POSIX
	mode_t m = 0;
	if (mode.read)  m |= S_IRUSR;
	if (mode.write) m |= S_IWUSR;
	if (mode.exec)  m |= S_IXUSR;
	switch (mode.other)
	{
	case 3:
		m |= S_IWGRP | S_IWOTH;
	case 2:
		m |= S_IXGRP | S_IXOTH;
	case 1:
		m |= S_IRGRP | S_IROTH;
	case 0:
	default:
		break;
	}
	return (fchmod((int)(qty_t)fp - 1, m) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 设置文件访问时间和修改时间 毫秒时间戳 */
PORT_API bool_t system_file_set_time(const char* path, qty_t atime, qty_t mtime)
{
	if (path == 0 || *path == '\0') return 0;
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
	bool_t ret = 0;
	HANDLE h;
	ULARGE_INTEGER ua = { 0 }, um = { 0 };
	FILETIME ft_a = { 0 }, ft_m = { 0 };

	if (system_string_to_native(tmp_path, path, size))
	{
		h = CreateFileW(tmp_path, FILE_WRITE_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			/* 毫秒 -> 100ns, 加 1601->1970 偏移 */
			ua.QuadPart = (unsigned long long)atime * 10000ULL
				+ SYSTEM_TIME_OFFSET_MS * 10000ULL;
			um.QuadPart = (unsigned long long)mtime * 10000ULL
				+ SYSTEM_TIME_OFFSET_MS * 10000ULL;
			ft_a.dwLowDateTime = ua.LowPart;
			ft_a.dwHighDateTime = ua.HighPart;
			ft_m.dwLowDateTime = um.LowPart;
			ft_m.dwHighDateTime = um.HighPart;
			ret = SetFileTime(h, 0, &ft_a, &ft_m) ? 1 : 0;
			CloseHandle(h);
		}
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(tmp_path, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	struct timespec ts[2];
	unsigned long long a_us = (unsigned long long)atime * 1000ULL + SYSTEM_TIME_OFFSET_US;
	unsigned long long m_us = (unsigned long long)mtime * 1000ULL + SYSTEM_TIME_OFFSET_US;
	/* times[0] 是 atime times[1] 是 mtime */
	ts[0].tv_sec = (time_t)(a_us / 1000000ULL);
	ts[0].tv_nsec = (long)((a_us % 1000000ULL) * 1000ULL);
	ts[1].tv_sec = (time_t)(m_us / 1000000ULL);
	ts[1].tv_nsec = (long)((m_us % 1000000ULL) * 1000ULL);
	return utimensat(AT_FDCWD, path, ts, 0) == 0 ? 1 : 0;
#else
	return 0;
#endif
}
/* 获取指定路径所在文件系统的容量和空闲空间 字节 */
PORT_API bool_t system_file_get_space(const char* path, qty_t* total, qty_t* free)
{
	if (path == 0 || *path == '\0') return 0;
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
	bool_t ret = 0;
	ULARGE_INTEGER ufree, utotal, uavail;
	if (system_string_to_native(tmp_path, path, size))
	{
		if (GetDiskFreeSpaceExW(tmp_path, &uavail, &utotal, &ufree))
		{
			if (total) *total = (qty_t)utotal.QuadPart;
			if (free)  *free = (qty_t)uavail.QuadPart;
			ret = 1;
		}
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(tmp_path, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	struct statvfs st;
	if (statvfs(path, &st) != 0)
		return 0;
	if (total) *total = (qty_t)((unsigned long long)st.f_blocks * (unsigned long long)st.f_frsize);
	if (free)  *free = (qty_t)((unsigned long long)st.f_bavail * (unsigned long long)st.f_frsize);
	return 1;
#else
	return 0;
#endif
}

/* Dir 目录 */

/* 创建目录 创建一个新的空目录 */
PORT_API bool_t system_dir_new(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	return CreateDirectoryW(wpath, NULL) ? 1 : 0;
#elif PLATFORM_POSIX
	return (mkdir(path, 0777) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 删除目录 删除一个空的目录 */
PORT_API bool_t system_dir_del(const char* path)
{
	if (path == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	return RemoveDirectoryW(wpath) ? 1 : 0;
#elif PLATFORM_POSIX
	return (rmdir(path) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 打开目录 返回文件名 遍历结束或失败返回 0 */
PORT_API system_dir_t system_dir_open(const char* path, char buf[SYSTEM_STRING_SAFE_MAX])
{
	if (path == 0) return 0;
	if (buf != 0) buf[0] = '\0';
#if PLATFORM_WINDOWS
	if (buf == 0) return 0;
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	if (wcslen(wpath) + 2 < size / sizeof(wchar_t))
		wcscat_s(wpath, size / sizeof(wchar_t), L"\\*");
	else
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	WIN32_FIND_DATAW findData = { 0 };
	HANDLE hFind = FindFirstFileW(wpath, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	if (system_string_from_native(buf, findData.cFileName, SYSTEM_STRING_SAFE_MAX * sizeof(char)) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		FindClose(hFind);
		return 0;
	}
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	return (system_dir_t)hFind;
#elif PLATFORM_POSIX
	return (system_dir_t)opendir(path);
#else
	return 0;
#endif
}
/* 关闭目录 */
PORT_API bool_t system_dir_close(system_dir_t dir)
{
	if (dir == 0) return 0;
#if PLATFORM_WINDOWS
	return FindClose(dir) ? 1 : 0;
#elif PLATFORM_POSIX
	return (closedir((DIR*)dir) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 查看当前目录的一项 无法后退 */
PORT_API const char* system_dir_read(system_dir_t dir, char* buf, qty_t buf_size)
{
	if (dir == 0 || buf == 0 || buf_size == 0) return 0;
#if PLATFORM_WINDOWS
	WIN32_FIND_DATAW findData = { 0 };
	if (FindNextFileW(dir, &findData) == 0)
		return 0;
	if (system_string_from_native(buf, findData.cFileName, buf_size) == 0)
		return 0;
	return buf;
#elif PLATFORM_POSIX
	struct dirent* entry = readdir((DIR*)dir);
	if (entry == 0) return 0;
	if (system_string_cpy(buf, entry->d_name, (size_t)buf_size) == 0) return 0;
	return buf;
#else
	return 0;
#endif
}
/* 获取目录信息 */
PORT_API bool_t system_dir_stat(const char* path, struct system_file_info_t* info)
{
	if (path == 0 || info == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wpath = system_mmap_new(0, size, port, flags);
	if (wpath == 0) return 0;
#else
	wchar_t wpath[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wpath, path, size) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	HANDLE hFile = CreateFileW(
		wpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
		system_mmap_del(wpath, size);
#endif
		return 0;
	}
	bool_t ret = system_file_stat((system_file_t)hFile, info);
#if COM_USE(CONFIG_SYSTEM_MMAP) && (SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX)
	system_mmap_del(wpath, size);
#endif
	CloseHandle(hFile);
	return ret;
#elif PLATFORM_POSIX
	struct stat st = { 0 };
	if (stat(path, &st) != 0) return 0;
	info->size = (qty_t)st.st_size;
	info->cre_time = ((qty_t)st.st_ctim.tv_sec * 1000 + (qty_t)st.st_ctim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->mod_time = ((qty_t)st.st_mtim.tv_sec * 1000 + (qty_t)st.st_mtim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->ass_time = ((qty_t)st.st_atim.tv_sec * 1000 + (qty_t)st.st_atim.tv_nsec / 1000000) - SYSTEM_TIME_OFFSET_MS;
	info->nlink = (qty_t)st.st_nlink;
	info->mode.read = (st.st_mode & S_IRUSR) ? 1 : 0;
	info->mode.write = (st.st_mode & S_IWUSR) ? 1 : 0;
	info->mode.exec = (st.st_mode & S_IXUSR) ? 1 : 0;
	info->mode.other = 0;
	if (st.st_mode & S_IROTH) info->mode.other = 1;
	if (st.st_mode & S_IXOTH) info->mode.other = 2;
	switch (st.st_mode & S_IFMT)
	{
	case S_IFREG:  info->type = SYSTEM_FILE_TYPE_REGULAR; break;
	case S_IFDIR:  info->type = SYSTEM_FILE_TYPE_DIR; break;
	case S_IFLNK:  info->type = SYSTEM_FILE_TYPE_LINK; break;
	case S_IFCHR:  info->type = SYSTEM_FILE_TYPE_CHAR_DEV; break;
	case S_IFBLK:  info->type = SYSTEM_FILE_TYPE_BLOCK_DEV; break;
	case S_IFSOCK: info->type = SYSTEM_FILE_TYPE_SOCKET; break;
	case S_IFIFO:  info->type = SYSTEM_FILE_TYPE_NAMEPIPE; break;
	default:       info->type = SYSTEM_FILE_TYPE_UNDEF; break;
	}
	return 1;
#else
	return 0;
#endif
}

/* Link 链接 */

/* 创建硬链接 target目标 linkpath链接源 */
PORT_API bool_t system_link_hardlink(const char* target, const char* linkpath)
{
	if (target == 0 || linkpath == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t) * 2;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wtarget = system_mmap_new(0, size, port, flags);
	if (wtarget == 0) return 0;
	wchar_t* wlink = (wchar_t*)((char*)wtarget + size / 2);
#else
	wchar_t wtarget[SYSTEM_PATH_MAX] = { 0 };
	wchar_t wlink[SYSTEM_PATH_MAX] = { 0 };
#endif
	bool_t ret = 0;
	if (system_string_to_native(wtarget, target, size / 2))
		if (system_string_to_native(wlink, linkpath, size / 2))
			ret = CreateHardLinkW((wchar_t*)wlink, wtarget, NULL) ? 1 : 0;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	system_mmap_del(wtarget, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	return (link(target, linkpath) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 创建软连接/符号链接 target目标 linkpath链接源 */
PORT_API bool_t system_link_symlink(const char* target, const char* linkpath)
{
	if (target == 0 || linkpath == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t size = SYSTEM_PATH_MAX * sizeof(wchar_t) * 2;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wtarget = system_mmap_new(0, size, port, flags);
	if (wtarget == 0) return 0;
	wchar_t* wlink = (wchar_t*)((char*)wtarget + size / 2);
#else
	wchar_t wtarget[SYSTEM_PATH_MAX] = { 0 };
	wchar_t wlink[SYSTEM_PATH_MAX] = { 0 };
#endif
	bool_t ret = 0;
	if (system_string_to_native(wtarget, target, size / 2))
		if (system_string_to_native(wlink, linkpath, size / 2))
		{
			DWORD attrs = GetFileAttributesW(wtarget);
			DWORD flagss = 0;
			if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
				flagss = SYMBOLIC_LINK_FLAG_DIRECTORY;
			ret = CreateSymbolicLinkW((wchar_t*)wlink, wtarget, flagss) ? 1 : 0;
		}
#if COM_USE(CONFIG_SYSTEM_MMAP)
	system_mmap_del(wtarget, size);
#endif
	return ret;
#elif PLATFORM_POSIX
	return (symlink(target, linkpath) == 0) ? 1 : 0;
#else
	return 0;
#endif
}
/* 读取软链接的目标路径 */
PORT_API bool_t system_link_readlink(const char* linkpath, char* buf, qty_t size)
{
	if (linkpath == 0 || buf == 0 || size == 0) return 0;
#if PLATFORM_WINDOWS
	qty_t sizeM = SYSTEM_PATH_MAX * sizeof(wchar_t) * 2;
#if COM_USE(CONFIG_SYSTEM_MMAP)
	struct system_mmap_prot_t port = { 0 };
	port.read = 1;
	port.write = 1;
	struct system_mmap_flags_t flags = { 0 };
	wchar_t* wtarget = system_mmap_new(0, sizeM, port, flags);
	if (wtarget == 0) return 0;
	wchar_t* wlink = (wchar_t*)((char*)wtarget + sizeM / 2);
#else
	wchar_t wtarget[SYSTEM_PATH_MAX] = { 0 };
	wchar_t wlink[SYSTEM_PATH_MAX] = { 0 };
#endif
	if (system_string_to_native(wlink, linkpath, sizeM / 2) == 0)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP)
		system_mmap_del(wtarget, sizeM);
#endif
		return 0;
	}
	HANDLE h = CreateFileW(
		wlink, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP)
		system_mmap_del(wtarget, sizeM);
#endif
		return 0;
	}
	DWORD retLen = GetFinalPathNameByHandleW(h, wtarget, SYSTEM_PATH_MAX, 0);
	CloseHandle(h);
	if (retLen == 0 || retLen >= SYSTEM_PATH_MAX)
	{
#if COM_USE(CONFIG_SYSTEM_MMAP)
		system_mmap_del(wtarget, sizeM);
#endif
		return 0;
	}
	bool_t ret = system_string_from_native(buf, wtarget, size);
#if COM_USE(CONFIG_SYSTEM_MMAP)
	system_mmap_del(wtarget, sizeM);
#endif
	return ret;
#elif PLATFORM_POSIX
	ssize_t ret = readlink(linkpath, buf, (size_t)(size - 1));
	if (ret < 0) return 0;
	buf[ret] = '\0';
	return 1;
#else
	return 0;
#endif
}


#else


PORT_API system_file_t system_file_open(const char* path, struct system_file_flags_t flags, struct system_file_mode_t mode) { ; }
PORT_API bool_t system_file_close(system_file_t fp) { ; }
PORT_API bool_t system_file_read(system_file_t fp, void* buf, qty_t count, qty_t* buf_size) { ; }
PORT_API bool_t system_file_write(system_file_t fp, const void* buf, qty_t count, qty_t* buf_size) { ; }
PORT_API system_off_t system_file_lseek(system_file_t fp, system_off_t offset, qty_t whence) { ; }
PORT_API bool_t system_file_rename(const char* oldpath, const char* newpath) { ; }
PORT_API bool_t system_file_del(const char* path) { ; }
PORT_API bool_t system_file_stat(system_file_t fp, struct system_file_info_t* info) { ; }
PORT_API bool_t system_file_sync(system_file_t fp) { ; }
PORT_API bool_t system_file_chmod(system_file_t fp, struct system_file_mode_t mode) { ; }
PORT_API bool_t system_dir_new(const char* path) { ; }
PORT_API bool_t system_dir_del(const char* path) { ; }
PORT_API system_dir_t system_dir_open(const char* path, char buf[SYSTEM_STRING_SAFE_MAX]) { ; }
PORT_API bool_t system_dir_close(system_dir_t dir) { ; }
PORT_API const char* system_dir_read(system_dir_t dir, char* buf, qty_t buf_size) { ; }
PORT_API bool_t system_dir_stat(const char* path, struct system_file_info_t* info) { ; }
PORT_API bool_t system_link_hardlink(const char* target, const char* linkpath) { ; }
PORT_API bool_t system_link_symlink(const char* target, const char* linkpath) { ; }
PORT_API bool_t system_link_readlink(const char* linkpath, char* buf, qty_t size) { ; }


#endif