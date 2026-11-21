#ifndef SYSTEM_CMD_H
/* ----命令----
* 依赖可能有不一致行为
----命令---- */
#define SYSTEM_CMD_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
#include "core/config.h"
#include "core/system/STATIC/def.h"
#include "core/system/pipe.h"
#include "core/system/file.h"
EXTERN_C_BEGIN


/*
* 创建一次性命令执行
* 将标准输出句柄重定向到文件
* 不 创建窗口 分离 生命周期绑定
* arg 为单条字符串
*/
PORT_API bool_t system_cmd_exec(const char* path, const char* arg, system_file_t fp);
/*
* 创建命令行
* 将标准输入输出句柄重定向到管道
* 不 创建窗口 分离 生命周期绑定
* 标准错误默认继承 当传递时重定向
* fpR fpW 为返回管道句柄
*/
PORT_API bool_t system_cmd_spawn(const char* path, system_pipe_t* fpR, system_pipe_t* fpW, system_handle_h fpE);


EXTERN_C_END
#endif