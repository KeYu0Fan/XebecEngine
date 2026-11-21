#ifndef SYSTEM_FILE_H
/* ----文件----
* direct直接读写磁盘 需要权限
*
* struct system_file_mode_t other windows 无效
*
* struct system_file_flags_t direct 需要对齐求严格的内存对齐和扇区大小对齐
*
* system_file_rename windows允许跨分区 linux不允许跨分区
*
* enum system_file_info_aux_type
* SYSTEM_FILE_TYPE_CHAR_DEV,SYSTEM_FILE_TYPE_BLOCK_DEV,SYSTEM_FILE_TYPE_SOCKET
* linux兼容 windows无对应
*
* system_file_read system_file_write
* 返回的成功可能未完全写入count个字节 有可能是 文件已到末尾 中断 等原因
*
* struct system_file_info_t cre_time 在linux下为状态变更时间 非创建时间
*
* struct system_file_info_t type
* linux 通过权限位判断 如果需要排除so 需要请自行判断
* windows 不为SYSTEM_FILE_TYPE_EXEC 需要请自行判断
*
* system_dir_open
* Windows 会将第一个条目的名称写入 buf 且 system_dir_read 从第二个条目开始 强制提供buf
* linux 不填充 buf system_dir_read 从第一个条目开始
*
* system_file_open linux 下返回的句柄是 fd+1 因为0被用作失败标志
*
* SYSTEM_PATH_MAX > SYSTEM_PATH_SAFE_MAX 开启长路径
*
* system_link_readlink 在 Windows 下返回的路径保留\\?\前缀 以支持长路径
*
* system_file_stat / system_dir_stat
* nlink 通常为 1 但有额外硬链接会大于1
----文件---- */
#define SYSTEM_FILE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
EXTERN_C_BEGIN


typedef system_handle_h system_file_t;
typedef void* system_dir_t;
struct system_file_flags_t
{
    /* 读取 */
    char read;
    /* 写入 */
    char write;
    /* 不存在创建 */
    char cre;
    /* 存在返回失败 */
    char exc;
    /* 清空文件 */
    char trunc;
    /* 追加模式 */
    char add;
    /* 磁盘同步 */
    char sync;
    /* 直接读写磁盘 */
    char direct;
};
struct system_file_mode_t
{
    /* 读取 */
    char read;
    /* 写入 */
    char write;
    /* 执行 */
    char exec;
    /* 非所有者用户 0禁止 1读 2读+执行 3读+执行+写 */
    char other;
};
enum system_file_info_type
 {
    /* 未知错误 */
    SYSTEM_FILE_TYPE_UNDEF = 0,
    /* 普通文件 .txt .so .dll 等 */
    SYSTEM_FILE_TYPE_REGULAR,
    /* 可执行文件 windows下为环境变量 linux下为文件的可执行位 */
    SYSTEM_FILE_TYPE_EXEC,
    /* 目录 */
    SYSTEM_FILE_TYPE_DIR,
    /* 符号链接 / 软链接 */
    SYSTEM_FILE_TYPE_LINK,
    /* 字符设备 键盘 串口等 */
    SYSTEM_FILE_TYPE_CHAR_DEV,
    /* 块设备 硬盘 U盘等 */
    SYSTEM_FILE_TYPE_BLOCK_DEV,
    /* 套接字 网络通信 */
    SYSTEM_FILE_TYPE_SOCKET,
    /* 命名管道 */
    SYSTEM_FILE_TYPE_NAMEPIPE,
};
struct system_file_info_t
{
    /* 文件大小 字节 */
    qty_t size;
    /* 创建时间 毫秒时间戳 */
    qty_t cre_time;
    /* 修改时间 毫秒时间戳 */
    qty_t mod_time;
    /* 访问时间 毫秒时间戳 */
    qty_t ass_time;
    /* 权限 */
    struct system_file_mode_t mode;
    /* 文件类型 */
    enum system_file_info_type type;
    /* 是否无硬链接 */
    qty_t nlink;
};

/* File 文件 */

/* 打开文件,path路径,flags功能,mode权限 */
PORT_API system_file_t system_file_open(const char* path, struct system_file_flags_t flags, struct system_file_mode_t mode);
/* 关闭文件, fd句柄 */
PORT_API bool_t system_file_close(system_file_t fp);
/* 读取文件,buf输出输入流,count字节数量 buf_size读取的字节数 */
PORT_API bool_t system_file_read(system_file_t fp, void* buf, qty_t count, qty_t* buf_size);
/* 写入文件 */
PORT_API bool_t system_file_write(system_file_t fp, const void* buf, qty_t count, qty_t* buf_size);
/* 文件指针移动,offset偏移量,whence偏移参考坐标(0头|1中|2尾) 失败返回 -1 */
PORT_API system_off_t system_file_lseek(system_file_t fp, system_off_t offset, qty_t whence);
/* 文件重命名 */
PORT_API bool_t system_file_rename(const char* oldpath, const char* newpath);
/* 删除文件 */
PORT_API bool_t system_file_del(const char* path);
/* 获取文件信息填充结构体 */
PORT_API bool_t system_file_stat(system_file_t fp, struct system_file_info_t* info);
/* 文件同步 将缓冲区数据和元数据强制写入磁盘 */
PORT_API bool_t system_file_sync(system_file_t fp);
/* 修改文件权限 Linux 下设置精确权限, Windows 下仅影响只读属性 */
PORT_API bool_t system_file_chmod(system_file_t fp, struct system_file_mode_t mode);
/* 设置文件访问时间和修改时间 毫秒时间戳 */
PORT_API bool_t system_file_set_time(const char* path, qty_t atime_ms, qty_t mtime_ms);
/* 获取指定路径所在文件系统的容量和空闲空间 字节 */
PORT_API bool_t system_file_get_space(const char* path, qty_t* total, qty_t* free);

/* Dir 目录 */

/* 创建目录 创建一个新的空目录 */
PORT_API bool_t system_dir_new(const char* path);
/* 删除目录 删除一个空的目录 */
PORT_API bool_t system_dir_del(const char* path);
/* 打开目录 返回文件名 buf[SYSTEM_STRING_SAFE_MAX] 失败返回0 */ 
PORT_API system_dir_t system_dir_open(const char* path, char buf[SYSTEM_STRING_SAFE_MAX]);
/* 关闭目录 */
PORT_API bool_t system_dir_close(system_dir_t dir);
/* 查看当前目录的一项 无法后退 */
PORT_API const char* system_dir_read(system_dir_t dir, char* buf, qty_t buf_size);
/* 获取目录信息 */
PORT_API bool_t system_dir_stat(const char* path, struct system_file_info_t* info);

/* Link 链接 */

/* 创建硬链接 target目标 linkpath链接源 */
PORT_API bool_t system_link_hardlink(const char* target, const char* linkpath);
/* 创建软连接/符号链接 target目标 linkpath链接源 */
PORT_API bool_t system_link_symlink(const char* target, const char* linkpath);
/* 读取软链接的目标路径 */
PORT_API bool_t system_link_readlink(const char* linkpath, char* buf, qty_t size);


EXTERN_C_END
#endif