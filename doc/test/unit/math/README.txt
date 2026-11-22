Xebec math 测试
================

用途
----
core/math 的独立验证程序。每个 .c 都是带 main() 的完整程序，直接 include
"core/math/math.h"，不依赖引擎其他模块，也不属于正式构建产物。

文件
----
test_math_core.c        标量/向量/矩阵/四元数基础与修复项（欧拉角、from_to、slerp 等）
test_math_geometry.c    几何图元、射线/平面/球/盒求交、视锥剔除、工具模块（颜色/半精度/定点/随机/噪声/曲线）
test_math_edge.c        边界与进阶项（万向锁、反向 slerp、正交化、半精度往返、TRS 逆、NDC 转换）
test_math_layout.c      类型宽度、plane/frustum 布局与步长、SIMD 非对齐加载
test_math_api.c         后期补充的接口（角度插值、SQUAD、swing-twist、OBB/平面交线、十六进制颜色、缓动等）
test_math_primitives.c  线段/三角形/胶囊体/OBB 及其两两求交

运行
----
在仓库根目录执行：

    powershell -ExecutionPolicy Bypass -File doc\test\unit\math\run.ps1

脚本会自动通过 vswhere 查找 MSVC 的 vcvars64.bat；也可以先打开
"x64 Native Tools Command Prompt"，再执行：

    powershell -ExecutionPolicy Bypass -File doc\test\unit\math\run.ps1

可选参数 -RepoRoot 用于指定仓库根（默认按脚本位置向上三级推断）。

所有程序输出 "ALL PASS" 或 "PASS" 且退出码为 0 视为通过；脚本最后打印汇总表，
任一程序失败则整体退出码为 1。

说明
----
- 编译参数固定为 /W4 /utf-8，任何警告都会在构建输出中显示；
- 测试程序只读 core/math 头文件，不修改工程；
- 新增测试时，把 .c 放进本目录并在 run.ps1 的 $tests 列表追加文件名即可。