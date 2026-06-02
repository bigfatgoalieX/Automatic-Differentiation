#pragma once

#include "ad/expr.hpp"

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace ad {

/// Compiles one or more expression roots into a linear evaluation plan for reuse.
class Evaluator {
public:
  explicit Evaluator(std::initializer_list<Expr> roots);

  std::vector<double> evaluate(
      const std::unordered_map<std::string, double>& bindings) const;

private:
  struct CompiledNode {
    std::shared_ptr<const Node> node;
    std::vector<std::size_t> child_slots;
    std::size_t slot{0};
  };

  std::vector<CompiledNode> plan_;
  std::vector<std::size_t> root_slots_;
  std::unordered_map<std::string, std::size_t> var_slots_;
  mutable std::vector<double> workspace_;
};

}  // namespace ad
