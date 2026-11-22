性能基准（待补）
================

用途:
  测吞吐/延迟/分配次数, 输出数字, 不做 pass/fail 断言.

计划:
  - struct: array push/get, hash insert/find, string 查找
  - math: vec/mat 运算
  - system: 线程创建/切换, 文件读写, mmap 分配

说明:
  需要固定环境、预热、多次取中位数; 结果记录到文档, 用于回归对比.
  不放进 doc/test/run.ps1 的快速回归.