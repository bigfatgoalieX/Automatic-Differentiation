#include "ad/expr.hpp"

#include "ad/detail/derivative.hpp"
#include "ad/detail/node.hpp"

namespace ad {

namespace detail {

Expr make_expr(std::shared_ptr<const Node> node) { return Expr(std::move(node)); }

}  // namespace detail

Expr::Expr() = default;

Expr::Expr(std::shared_ptr<const Node> node) : node_(std::move(node)) {}

Expr Expr::variable(const std::string& name) {
  return detail::make_expr(make_variable_node(name));
}

Expr Expr::constant(double value) {
  return detail::make_expr(make_constant_node(value));
}

Expr Expr::operator+(const Expr& rhs) const {
  return detail::make_expr(make_binary_node(NodeKind::Add, node_, rhs.node_));
}

Expr Expr::operator-(const Expr& rhs) const {
  return detail::make_expr(make_binary_node(NodeKind::Sub, node_, rhs.node_));
}

Expr Expr::operator*(const Expr& rhs) const {
  return detail::make_expr(make_binary_node(NodeKind::Mul, node_, rhs.node_));
}

Expr Expr::operator/(const Expr& rhs) const {
  return detail::make_expr(make_binary_node(NodeKind::Div, node_, rhs.node_));
}

Expr Expr::operator-() const {
  return detail::make_expr(make_unary_node(NodeKind::Neg, node_));
}

Expr& Expr::operator+=(const Expr& rhs) {
  *this = *this + rhs;
  return *this;
}

Expr& Expr::operator-=(const Expr& rhs) {
  *this = *this - rhs;
  return *this;
}

Expr& Expr::operator*=(const Expr& rhs) {
  *this = *this * rhs;
  return *this;
}

Expr& Expr::operator/=(const Expr& rhs) {
  *this = *this / rhs;
  return *this;
}

Expr Expr::square() const {
  return detail::make_expr(make_unary_node(NodeKind::Square, node_));
}

Expr Expr::abs() const {
  return detail::make_expr(make_unary_node(NodeKind::Abs, node_));
}

Expr Expr::pow(int exponent) const {
  return detail::make_expr(make_pow_node(node_, exponent));
}

std::vector<Expr> Expr::derivative(std::initializer_list<Expr> wrt) const {
  std::vector<Expr> out;
  out.reserve(wrt.size());
  for (const auto& v : wrt) {
    out.push_back(differentiate_expr(*this, v));
  }
  return out;
}

bool same_expr(const Expr& a, const Expr& b) { return a.node() == b.node(); }

}  // namespace ad
