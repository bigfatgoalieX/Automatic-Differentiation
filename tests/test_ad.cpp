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

bool run_cse_tests() {
  const auto x = ad::Expr::variable("x");
  const auto p = ad::Expr::variable("p");

  if (ad::same_expr(x, ad::Expr::variable("x"))) {
    std::cerr << "same-name variables should preserve distinct identity\n";
    return false;
  }

  const auto c1 = ad::Expr::constant(2.0);
  const auto c2 = ad::Expr::constant(2.0);
  if (!ad::same_expr(c1, c2)) {
    std::cerr << "equal constants were not interned\n";
    return false;
  }

  const auto constant_derivatives = c1.derivative({c2});
  const ad::Evaluator constant_derivative({constant_derivatives[0]});
  const auto dc = constant_derivative.evaluate({});
  if (!approx_equal(dc[0], 0.0)) {
    std::cerr << "constant derivative mismatch: " << dc[0] << " vs 0\n";
    return false;
  }

  const auto xp = x * p;
  if (!ad::same_expr(xp, x * p)) {
    std::cerr << "repeated multiplication was not interned\n";
    return false;
  }
  if (!ad::same_expr(xp, p * x)) {
    std::cerr << "commutative multiplication was not canonicalized\n";
    return false;
  }

  const auto e1 = ad::exp(x * p);
  const auto e2 = ad::exp(p * x);
  if (!ad::same_expr(e1, e2)) {
    std::cerr << "repeated exp subexpression was not interned\n";
    return false;
  }

  const auto repeated = ad::exp(x * p) + ad::exp(p * x);
  const auto d_repeated_p = repeated.derivative({p})[0];
  const ad::Evaluator evaluator({repeated, d_repeated_p});

  const double xv = 3.0;
  const double pv = 0.25;
  const auto results = evaluator.evaluate({{"x", xv}, {"p", pv}});
  const double expected = 2.0 * std::exp(xv * pv);
  const double expected_derivative = 2.0 * xv * std::exp(xv * pv);
  if (!approx_equal(results[0], expected)) {
    std::cerr << "CSE expression value mismatch: " << results[0] << " vs "
              << expected << '\n';
    return false;
  }
  if (!approx_equal(results[1], expected_derivative)) {
    std::cerr << "CSE derivative mismatch: " << results[1] << " vs "
              << expected_derivative << '\n';
    return false;
  }

  return true;
}

}  // namespace

int main() {
  if (!run_cse_tests()) {
    return 1;
  }

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
