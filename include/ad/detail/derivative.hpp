#pragma once

#include "ad/expr.hpp"

namespace ad {

Expr differentiate_expr(const Expr& expr, const Expr& wrt);

}  // namespace ad
