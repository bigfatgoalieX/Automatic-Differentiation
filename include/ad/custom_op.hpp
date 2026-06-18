#pragma once

#include "ad/expr.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ad {

class Node;

/// Symbolic gradient hook: given ∂L/∂output, return ∂L/∂inputs[i] (one per input).
using SymbolicGradFn = std::function<std::vector<Expr>(
    const std::vector<Expr>& inputs, const std::vector<Expr>& grad_outputs)>;

/// Numeric forward hook: evaluate output from input values.
using ForwardFn = std::function<double(const std::vector<double>& inputs)>;

/// Register a user-defined unary or n-ary operator.
///
/// The operator name participates in common-subexpression elimination, so use a
/// distinct name for each distinct set of forward/gradient semantics.
Expr make_custom_op(const std::string& name, const std::vector<Expr>& inputs,
                    ForwardFn forward, SymbolicGradFn symbolic_grad);

}  // namespace ad
