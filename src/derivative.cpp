#include "ad/detail/derivative.hpp"

#include "ad/detail/node.hpp"

#include <stdexcept>

namespace ad {

Expr differentiate_expr(const Expr& expr, const Expr& wrt) {
  if (!expr.node()) {
    return Expr::constant(0.0);
  }
  if (!wrt.node()) {
    throw std::invalid_argument("derivative w.r.t. expression is empty");
  }
  if (expr.node() == wrt.node()) {
    if (expr.node()->kind() == NodeKind::Constant) {
      return Expr::constant(0.0);
    }
    return Expr::constant(1.0);
  }
  return expr.node()->differentiate(expr, wrt);
}

}  // namespace ad
