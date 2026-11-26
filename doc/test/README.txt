单元测试
========

doc/test 按测试类型分层:

  unit/      功能/行为测试（快速回归）
    math/    数学
    format/  整数/浮点/十六进制格式转换
    as/      ini 测试
    struct/  数组/字符串/链表/哈希/缓冲区/var/life/arena
    system/  线程/时间/文件/目录/mmap/env/dynlib/pipe/console/cmd/priv/net/info/进程
  stress/    压力/并发测试（待补）
  bench/     性能基准（待补）

运行全部 unit:
  powershell -NoProfile -ExecutionPolicy Bypass -File doc\test\run.ps1   (Windows / MSVC)
  sh doc/test/run.sh                                                    (Linux / gcc)

Linux 依赖:
  build-essential, libcap-dev (priv.c 使用 sys/capability.h)
  链接使用 -pthread -ldl -lm -lcap

脚本会:
  1. 调用 unit/math/run.ps1;
  2. 构建并运行 unit/format、unit/as、unit/struct 与 unit/system;
  3. 任一失败返回非零.

说明:
  as/ini 测试在 unit/as/test.c, 已接入 run.ps1/run.sh.
  struct 的 realm 尚未实现, 不参与编译与测试.
  system 测试覆盖可稳定运行的接口; console/cmd/priv 只验证可调用与基本映射,
  net 用 127.0.0.1 回环验证映射, process 启动 cmd.exe/sh 并检查退出码.
  stress/bench 不参与快速回归, 后续单独补脚本.