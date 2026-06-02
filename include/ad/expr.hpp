#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

namespace ad {

class Node;
class Expr;

namespace detail {
Expr make_expr(std::shared_ptr<const Node> node);
}

/// Scalar expression handle (shared DAG node).
class Expr {
public:
  Expr();

  static Expr variable(const std::string& name);
  static Expr constant(double value);

  Expr operator+(const Expr& rhs) const;
  Expr operator-(const Expr& rhs) const;
  Expr operator*(const Expr& rhs) const;
  Expr operator/(const Expr& rhs) const;
  Expr operator-() const;

  Expr& operator+=(const Expr& rhs);
  Expr& operator-=(const Expr& rhs);
  Expr& operator*=(const Expr& rhs);
  Expr& operator/=(const Expr& rhs);

  Expr square() const;
  Expr abs() const;
  Expr pow(int exponent) const;

  /// Symbolic partial derivatives w.r.t. each expression in @p wrt (typically variables).
  std::vector<Expr> derivative(std::initializer_list<Expr> wrt) const;

  const std::shared_ptr<const Node>& node() const { return node_; }

private:
  friend class Evaluator;
  friend Expr detail::make_expr(std::shared_ptr<const Node> node);
  explicit Expr(std::shared_ptr<const Node> node);

  std::shared_ptr<const Node> node_;
};

bool same_expr(const Expr& a, const Expr& b);

}  // namespace ad
