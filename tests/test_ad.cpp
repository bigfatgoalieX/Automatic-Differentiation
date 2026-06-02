#include "ad/ad.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <unordered_map>

namespace {

double central_diff(const std::function<double(double)>& f, double x, double h) {
  return (f(x + h) - f(x - h)) / (2.0 * h);
}

double loss_value(double x, double y, double p) {
  const double pred = std::exp(x * p) + 0.5 * x;
  const double d = pred - y;
  return d * d;
}

bool approx_equal(double a, double b, double tol = 1e-6) {
  return std::abs(a - b) <= tol;
}

}  // namespace

int main() {
  const auto p = ad::Expr::variable("p");
  const auto x = ad::Expr::variable("x");
  const auto y = ad::Expr::variable("y");

  const auto pred = ad::exp(x * p) + ad::Expr::constant(0.5) * x;
  const auto loss = (pred - y).pow(2);

  const auto deriv_p = loss.derivative({p})[0];
  const auto deriv_2nd_p = deriv_p.derivative({p})[0];

  const ad::Evaluator evaluator({loss, deriv_p, deriv_2nd_p});

  const double xv = 1.0;
  const double yv = -0.05;
  const double pv = 0.5;
  const auto results =
      evaluator.evaluate({{"x", xv}, {"y", yv}, {"p", pv}});

  const double loss_numeric = loss_value(xv, yv, pv);
  if (!approx_equal(results[0], loss_numeric)) {
    std::cerr << "loss mismatch: " << results[0] << " vs " << loss_numeric << '\n';
    return 1;
  }

  const double h = 1e-6;
  const double d1 =
      central_diff([&](double pp) { return loss_value(xv, yv, pp); }, pv, h);
  if (!approx_equal(results[1], d1, 1e-4)) {
    std::cerr << "first derivative mismatch: " << results[1] << " vs " << d1 << '\n';
    return 1;
  }

  const double d2 = central_diff(
      [&](double pp) {
        return central_diff(
            [&](double ppp) { return loss_value(xv, yv, ppp); }, pp, h);
      },
      pv, h);
  if (!approx_equal(results[2], d2, 1e-3)) {
    std::cerr << "second derivative mismatch: " << results[2] << " vs " << d2 << '\n';
    return 1;
  }

  std::cout << "all tests passed\n";
  return 0;
}
