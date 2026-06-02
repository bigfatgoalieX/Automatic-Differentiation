# Static Symbolic Automatic Differentiation

C++17 标量静态自动求导库：表达建构图、符号求导、可复用的 `Evaluator` 

## 支持的功能

- 四则运算、`square`、`abs`、整数幂 `pow(n)`
- 符号求导（含高阶，如对导数表达式再 `derivative`）
- 可扩展自定义算子（内置示例：`exp`）
- `Evaluator` 编译多表达式求值计划，支持多次 `evaluate`

## Build and Run

```bash
cmake -S . -B build
ln -sf build/compile_commands.json compile_commands.json   # IDE 跳转（仅需一次）
cmake --build build
./build/ad_example
ctest --test-dir build
```

### IDE 跳转（Command+点击）

工程已开启 `CMAKE_EXPORT_COMPILE_COMMANDS` 并包含 `.clangd` / `.vscode` 配置。若 `variable` 等无法跳转：

1. 执行上面的 `cmake` 与 `compile_commands.json` 软链接；
2. 在 Cursor 中 **Reload Window**（或重启 clangd：`clangd: Restart`）；
3. 确认使用 **clangd** 扩展时，不要与 Microsoft **C/C++** 的 IntelliSense 同时抢同一个文件（可在设置里对该项目禁用其一）。

## 示例

```cpp
#include "ad/ad.hpp"

const auto p = ad::Expr::variable("p");
const auto x = ad::Expr::variable("x");
const auto y = ad::Expr::variable("y");

const auto pred = ad::exp(x * p) + ad::Expr::constant(0.5) * x;
const auto loss = (pred - y).pow(2);

const auto derivatives = loss.derivative({p, pred});
const auto deriv_2nd_p = derivatives[0].derivative({p})[0];

ad::Evaluator evaluator({loss, derivatives[0], derivatives[1], deriv_2nd_p});
auto r = evaluator.evaluate({{"x", 1.0}, {"y", -0.05}, {"p", 0.5}});
```

详见 [DESIGN.md](DESIGN.md) 

## 扩展自定义算子

```cpp
ad::Expr my_op(const ad::Expr& a, const ad::Expr& b) {
  return ad::make_custom_op(
      "my_op", {a, b},
      [](const std::vector<double>& in) { return in[0] * in[1]; },
      [](const std::vector<ad::Expr>& inputs,
         const std::vector<ad::Expr>& grad_out) {
        return std::vector<ad::Expr>{grad_out[0] * inputs[1],
                                   grad_out[0] * inputs[0]};
      });
}
```
