#include "ad/ops.hpp"

#include "ad/custom_op.hpp"

#include <cmath>
#include <vector>

namespace ad {

Expr exp(const Expr& x) {
  return make_custom_op(
      "exp", {x},
      [](const std::vector<double>& inputs) { return std::exp(inputs[0]); },
      [](const std::vector<Expr>& inputs, const std::vector<Expr>& grad_outputs) {
        return std::vector<Expr>{grad_outputs[0] * exp(inputs[0])};
      });
}

}  // namespace ad
