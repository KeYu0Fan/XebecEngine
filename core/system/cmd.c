#include "cmd.h"
#if COM_USE(CONFIG_SYSTEM_CMD)


#include "core/system/STATIC/base.h"
#include "core/system/process.h"

/*
* 创建一次性命令执行
* 将标准输出句柄重定向到文件
* 不 创建窗口 分离 生命周期绑定
* arg 为单条字符串
*/
PORT_API bool_t system_cmd_exec(const char* path, const char* arg, system_file_t fp)
{
	COM_RETURN(path == 0 || arg == 0 || fp == 0, 0);
	struct system_process_attr_t attr = { 0 };
	struct system_process_info_t info = { 0 };
	system_process_t process = 0;
	const char* argv[3] = { path,arg,((void*)0) };
	attr.argv = argv;
	attr.envp = 0;
	attr.workdir = 0;
	attr.stdin_h = 0;
	attr.stdout_h = fp;
	attr.stderr_h = fp;
	attr.h_arr = 0;
	attr.h_len = 0;
	attr.detach = 0;
	attr.window = -1;
	attr.exit_extends = 0;
	process = system_process_spawn(path, &attr);
	if (process)
	{
		bool_t wait = system_process_wait(process, &info, ((qty_t)-1));
		bool_t ret = (wait == 1 && info.exited == 1);
		if (ret == 0) system_process_kill(process);
		system_process_close(process);
		return ret;
	}
	return 0;
}
/*
* 创建命令行
* 将标准输入输出句柄重定向到管道
* 不 创建窗口 分离 生命周期绑定
* 标准错误默认继承 当传递时重定向
* fpR fpW 为返回管道句柄
*/
PORT_API bool_t system_cmd_spawn(const char* path, system_pipe_t* fpR, system_pipe_t* fpW, system_handle_h fpE)
{
	COM_RETURN(path == 0 || fpR == 0 || fpW == 0, 0);
	struct system_process_attr_t attr = { 0 };
	system_pipe_t fxr = 0, fxw = 0, fyr = 0, fyw = 0;
	if (system_pipe_open(&fxr, &fxw))
	{
		if (system_pipe_open(&fyr, &fyw) == 0)
		{
			system_pipe_close(fxr);
			system_pipe_close(fxw);
			return 0;
		}
	}
	attr.argv = 0;
	attr.envp = 0;
	attr.workdir = 0;
	attr.stdin_h = fxw;
	attr.stdout_h = fyr;
	attr.stderr_h = 0;
	if (fpE)
		attr.stderr_h = fpE;
	attr.h_arr = 0;
	attr.h_len = 0;
	attr.detach = 0;
	attr.window = -1;
	attr.exit_extends = 0;
	if (system_process_spawn(path, &attr))
	{
		*fpR = fxr;
		*fpW = fyw;
		return 1;
	}
	else
	{
		system_pipe_close(fxr);
		system_pipe_close(fxw);
		system_pipe_close(fyr);
		system_pipe_close(fyw);
		return 0;
	}
}


#else


PORT_API bool_t system_cmd_exec(const char* path, const char* arg, system_file_t fp) { ; }
PORT_API bool_t system_cmd_spawn(const char* path, system_pipe_t* fpR, system_pipe_t* fpW, system_handle_h fpE) { ; }


#endif