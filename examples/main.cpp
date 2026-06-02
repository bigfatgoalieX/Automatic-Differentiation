#include "ad/ad.hpp"

#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>

int main() {
  const auto p = ad::Expr::variable("p");
  const auto x = ad::Expr::variable("x");
  const auto y = ad::Expr::variable("y");

  const auto pred = ad::exp(x * p) + ad::Expr::constant(0.5) * x;
  const auto loss = (pred - y).pow(2);

  const auto derivatives = loss.derivative({p, pred});
  const auto& deriv_p = derivatives[0];
  const auto& deriv_pred = derivatives[1];

  const auto deriv_2nd_p = deriv_p.derivative({p})[0];

  const ad::Evaluator evaluator({loss, deriv_p, deriv_pred, deriv_2nd_p});

  auto results = evaluator.evaluate({{"x", 1.0}, {"y", -0.05}, {"p", 0.5}});
  std::cout << "loss = " << results[0] << ", "
            << "derivative of p = " << results[1] << ", "
            << "derivative of pred = " << results[2] << ", "
            << "2nd derivative of p = " << results[3] << std::endl;

  std::vector<std::unordered_map<std::string, double>> data_source = {
      {{"x", 0.5}, {"y", 0.1}, {"p", 0.3}},
      {{"x", 2.0}, {"y", 1.0}, {"p", 1.0}},
  };

  for (const auto& data : data_source) {
    results = evaluator.evaluate(data);
    std::cout << "batch: loss = " << results[0] << ", d(loss)/dp = " << results[1]
              << std::endl;
  }

  return 0;
}
