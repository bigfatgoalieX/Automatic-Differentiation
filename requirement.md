20260602凯读投资量化开发实习笔试：

请用C++实现一个标量版的静态求导框架，有以下要求：

- 能实现四则运算、平方、绝对值的前向计算和反向传播

- 能由用户拓展自定义算子，自定义算子也能够前向计算和反向传播

- 作为例子，请拓展出exp算子

- 可读性，拓展性，性能尽量好

- 包含一个md文件来介绍设计和优化思路

框架用法大致如下：

```cpp
// 定义计算图
const auto p = variable("p");
const auto x = variable("x");
const auto y = variable("y");
const auto pred = exp(x*p) + 0.5*x;
const auto loss = (pred - y).pow(2);

// 衍生出loss关于p和pred的导数
const auto derivatives = loss.derivative({p,pred});
const auto &deriv_p = derivatives[0];
const auto &deriv_pred = derivatives[1];

// 衍生出loss关于p的二阶导数
const auto deriv_2nd_p = grad_p.derivative({p})[0];

// 生成求值器
const auto evaluator = Evaluator({loss,grad_p,grad_pred,grad_2nd_p});

// 计算loss值和p1，p2的导数
auto results = evaluator.evaluate({{"x",1.0},{"y", -0.05},{"p",0.5},});
std :: cout << "loss = " << result[0] << ","
  					<< "derivative of p = " << result[1] << ","
  					<< "derivative of pred = " << result[2] << ","
  					<< "2nd derivative of pred = " << result[3] << std :: endl;

// 多次数据复用同一个Evaluator
for(auto &&data : data_source){
  auto results = evaluator.evaluate(data);
}
```

