#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/platform.h"
#if PLATFORM_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "core/system/thread.h"
#include "core/system/time.h"
#include "core/system/file.h"
#include "core/system/mmap.h"
#include "core/system/env.h"
#include "core/system/dynlib.h"
#include "core/system/pipe.h"
#include "core/system/net.h"
#include "core/system/info.h"
#include "core/system/process.h"
#include "core/system/console.h"
#include "core/system/cmd.h"
#include "core/system/priv.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static system_mutex_t g_lock;
static int g_count = 0;
static system_thread_main_ret PLATFORM_CALL worker(void* arg)
{
	system_mutex_lock(&g_lock);
	g_count += (int)(qty_t)arg;
	system_mutex_unlock(&g_lock);
	return 0;
}

static system_once_init g_once = { 0 };
static int g_once_count = 0;
static void once_fn(void) { g_once_count++; }

static void test_thread(void)
{
	system_thread_t th[4];
	int i;
	CHECK(system_mutex_init(&g_lock));
	g_count = 0;
	for (i = 0; i < 4; i++)
	{
		th[i] = system_thread_new(worker, (void*)(qty_t)1);
		CHECK(th[i] != 0);
	}
	for (i = 0; i < 4; i++)
		CHECK(system_thread_join(th[i], 0));
	CHECK(g_count == 4);
	CHECK(system_mutex_del(&g_lock));

	CHECK(system_once_run(&g_once, once_fn));
	CHECK(system_once_run(&g_once, once_fn));
	CHECK(g_once_count == 1);

	{
		system_tls_key key;
		CHECK(system_tls_key_init(&key, 0));
		CHECK(system_tls_key_set(key, (void*)(qty_t)7));
		CHECK(system_tls_key_get(key) == (void*)(qty_t)7);
		CHECK(system_tls_key_del(key));
	}
	system_thread_yield();
}

static void test_time(void)
{
	qty_t t0 = system_time_timing_ms();
	qty_t t1;
	CHECK(system_time_now_ms() > 0);
	CHECK(system_time_now_us() > 0);
	CHECK(system_sleep_ms(2));
	t1 = system_time_timing_ms();
	CHECK(t1 >= t0);
	CHECK(system_sleep_us(1000));
}

static void test_file(void)
{
	const char* tmp = getenv("TEMP");
	char path[512];
	char buf[16] = { 0 };
	struct system_file_flags_t flags = { 0 };
	struct system_file_mode_t mode = { 0 };
	struct system_file_info_t info;
	system_file_t fp;
	qty_t n = 0;
	if (tmp == 0) tmp = ".";
	snprintf(path, sizeof(path), "%s/xebec_system_test.tmp", tmp);
	flags.read = 1;
	flags.write = 1;
	flags.cre = 1;
	flags.trunc = 1;
	mode.read = 1;
	mode.write = 1;
	fp = system_file_open(path, flags, mode);
	CHECK(fp != 0);
	if (fp == 0) return;
	CHECK(system_file_write(fp, "hello", 5, &n));
	CHECK(n == 5);
	CHECK(system_file_lseek(fp, 0, 0) == 0);
	CHECK(system_file_read(fp, buf, 5, &n));
	CHECK(n == 5);
	CHECK(memcmp(buf, "hello", 5) == 0);
	CHECK(system_file_stat(fp, &info));
	CHECK(info.size >= 5);
	CHECK(system_file_close(fp));
	CHECK(system_file_del(path));
}

static void test_dir(void)
{
	const char* tmp = getenv("TEMP");
	char name[SYSTEM_STRING_SAFE_MAX];
	system_dir_t dir;
	if (tmp == 0) tmp = ".";
	dir = system_dir_open(tmp, name);
	CHECK(dir != 0);
	if (dir != 0) CHECK(system_dir_close(dir));
}

static void test_mmap(void)
{
	qty_t page = system_info_get_page_size();
	struct system_mmap_prot_t prot = { 1, 1, 0, 0 };
	struct system_mmap_flags_t flags = { 0 };
	void* mem;
	CHECK(page >= 4096);
	mem = system_mmap_new(0, page, prot, flags);
	CHECK(mem != 0);
	if (mem == 0) return;
	memset(mem, 0x5A, page);
	CHECK(((unsigned char*)mem)[123] == 0x5A);
	CHECK(system_mmap_del(mem, page));
}

static void test_env(void)
{
	char buf[64] = { 0 };
	CHECK(system_env_set("XEBEC_TEST_ENV", "42"));
	CHECK(system_env_get(buf, "XEBEC_TEST_ENV", sizeof(buf)));
	CHECK(strcmp(buf, "42") == 0);
	CHECK(system_env_unset("XEBEC_TEST_ENV"));
}

static void test_dynlib(void)
{
#if PLATFORM_WINDOWS
	system_dynlib_t lib = system_dynlib_load("kernel32.dll");
	CHECK(lib != 0);
	if (lib != 0)
	{
		CHECK(system_dynlib_symbol(lib, "GetTickCount") != 0);
		CHECK(system_dynlib_unload(lib));
	}
#elif PLATFORM_POSIX
	system_dynlib_t lib = system_dynlib_load("libc.so.6");
	CHECK(lib != 0);
	if (lib != 0)
	{
		CHECK(system_dynlib_symbol(lib, "getpid") != 0);
		CHECK(system_dynlib_unload(lib));
	}
#endif
}

static void test_pipe(void)
{
	system_pipe_t r = 0;
	system_pipe_t w = 0;
	char buf[8] = { 0 };
	qty_t n = 0;
	CHECK(system_pipe_open(&r, &w));
	CHECK(system_pipe_write(w, "abc", 3, &n));
	CHECK(n == 3);
	CHECK(system_pipe_read(r, buf, 3, &n));
	CHECK(n == 3);
	CHECK(memcmp(buf, "abc", 3) == 0);
	CHECK(system_pipe_close(r));
	CHECK(system_pipe_close(w));
}

static void test_net(void)
{
	system_net_socket_t server;
	system_net_socket_t client;
	system_net_socket_t accepted;
	struct sockaddr_in addr;
	qty_t len = (qty_t)sizeof(addr);
	char buf[8] = { 0 };
	CHECK(system_net_init());
	server = system_net_socket(AF_INET, SOCK_STREAM, 0);
	CHECK((qty_t)server != (qty_t)-1);
	if ((qty_t)server == (qty_t)-1) { system_net_clean(); return; }
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;
	CHECK(system_net_bind(server, (system_sock_addr_t)&addr, sizeof(addr)));
	CHECK(system_net_listen(server, 1));
	len = (qty_t)sizeof(addr);
	CHECK(system_net_get_local(server, (system_sock_addr_t)&addr, &len));
	CHECK(addr.sin_port != 0);

	client = system_net_socket(AF_INET, SOCK_STREAM, 0);
	CHECK((qty_t)client != (qty_t)-1);
	if ((qty_t)client == (qty_t)-1)
	{
		CHECK(system_net_close(server));
		system_net_clean();
		return;
	}
	CHECK(system_net_connect(client, (system_sock_addr_t)&addr, sizeof(addr)));
	len = (qty_t)sizeof(addr);
	accepted = system_net_accept(server, (system_sock_addr_t)&addr, &len);
	CHECK((qty_t)accepted != (qty_t)-1);
	if ((qty_t)accepted != (qty_t)-1)
	{
		CHECK(system_net_send(client, "ping", 4, 0) == 4);
		CHECK(system_net_recv(accepted, buf, 4, 0) == 4);
		CHECK(memcmp(buf, "ping", 4) == 0);
		CHECK(system_net_close(accepted));
	}
	CHECK(system_net_close(client));
	CHECK(system_net_close(server));
	CHECK(system_net_clean());
}

static void test_console(void)
{
	system_handle_h in;
	system_handle_h out;
	system_handle_h err;
	CHECK(system_console_init());
	in = system_console_get(0);
	out = system_console_get(1);
	err = system_console_get(2);
	CHECK(in != 0);
	CHECK(out != 0);
	CHECK(err != 0);
	CHECK(system_console_set(1, out));
	CHECK(system_console_reset(1));
}

static void test_cmd(void)
{
#if PLATFORM_WINDOWS
	const char* tmp = getenv("TEMP");
	const char* cmd = getenv("ComSpec");
	char path[512];
	char buf[64] = { 0 };
	struct system_file_flags_t flags = { 0 };
	struct system_file_mode_t mode = { 0 };
	system_file_t fp;
	qty_t n = 0;
	if (tmp == 0) tmp = ".";
	if (cmd == 0) cmd = "C:\\Windows\\System32\\cmd.exe";
	snprintf(path, sizeof(path), "%s/xebec_cmd_test.txt", tmp);
	flags.read = 1;
	flags.write = 1;
	flags.cre = 1;
	flags.trunc = 1;
	mode.read = 1;
	mode.write = 1;
	fp = system_file_open(path, flags, mode);
	CHECK(fp != 0);
	if (fp == 0) return;
	CHECK(system_cmd_exec(cmd, "/c echo hello", fp));
	CHECK(system_file_close(fp));
	memset(&flags, 0, sizeof(flags));
	flags.read = 1;
	memset(&mode, 0, sizeof(mode));
	mode.read = 1;
	fp = system_file_open(path, flags, mode);
	CHECK(fp != 0);
	if (fp != 0)
	{
		CHECK(system_file_read(fp, buf, sizeof(buf) - 1, &n));
		CHECK(strstr(buf, "hello") != 0);
		CHECK(system_file_close(fp));
	}
	CHECK(system_file_del(path));
#endif
}

static void test_priv(void)
{
	bool_t has = system_priv_has(SYSTEM_PRIV_SYMLINK);
	CHECK(has == 0 || has == 1);
	(void)system_priv_open(SYSTEM_PRIV_SYMLINK);
	(void)system_priv_close(SYSTEM_PRIV_SYMLINK);
}

static void test_process(void)
{
	const char* exe;
	const char* argv[5];
#if PLATFORM_WINDOWS
	exe = getenv("ComSpec");
	if (exe == 0) exe = "C:\\Windows\\System32\\cmd.exe";
	argv[0] = exe;
	argv[1] = "/c";
	argv[2] = "exit";
	argv[3] = "7";
	argv[4] = 0;
#else
	exe = "/bin/sh";
	argv[0] = exe;
	argv[1] = "-c";
	argv[2] = "exit 7";
	argv[3] = 0;
	argv[4] = 0;
#endif
	struct system_process_attr_t attr = { 0 };
	struct system_process_info_t info = { 0 };
	system_process_t proc;
	system_pipe_t pr = 0;
	system_pipe_t pw = 0;
	attr.argv = argv;
	attr.window = -1;
	CHECK(system_pipe_open(&pr, &pw));
	attr.stdout_h = pw;
	proc = system_process_spawn(exe, &attr);
	CHECK(proc != 0);
	if (proc == 0)
	{
		system_pipe_close(pr);
		system_pipe_close(pw);
		return;
	}
	CHECK(system_process_wait(proc, &info, (qty_t)-1) == 1);
	CHECK(info.exited == 1);
	CHECK(info.exit_code == 7);
	CHECK(system_process_close(proc));
	CHECK(system_pipe_close(pr));
	CHECK(system_pipe_close(pw));
}

static void test_info(void)
{
	char host[256] = { 0 };
	CHECK(system_info_get_cpu_cores() > 0);
	CHECK(system_info_get_total_ram() > 0);
	CHECK(system_info_get_page_size() >= 4096);
	CHECK(system_info_get_os_name() != 0);
	CHECK(system_info_get_hostname(host, sizeof(host)));
	CHECK(host[0] != 0);
}

int main(void)
{
	CHECK(system_process_getpid() > 0);
	test_info();
	test_thread();
	test_time();
	test_file();
	test_dir();
	test_mmap();
	test_env();
	test_dynlib();
	test_pipe();
	test_console();
	test_cmd();
	test_priv();
	test_net();
	test_process();
	if (failures)
	{
		printf("system tests: FAIL (%d)\n", failures);
		return 1;
	}
	printf("system tests: PASS\n");
	return 0;
}