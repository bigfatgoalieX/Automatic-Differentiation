#pragma once

#include "ad/expr.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace ad {

enum class NodeKind {
  Variable,
  Constant,
  Add,
  Sub,
  Mul,
  Div,
  Neg,
  Square,
  Abs,
  Pow,
  Custom,
};

class Node {
public:
  virtual ~Node() = default;

  virtual NodeKind kind() const = 0;
  virtual std::vector<std::shared_ptr<const Node>> children() const = 0;
  virtual Expr differentiate(const Expr& self, const Expr& wrt) const = 0;
  virtual double forward(const std::vector<double>& child_values) const = 0;

  const std::string& var_name() const { return var_name_; }
  double constant_value() const { return constant_value_; }
  int pow_exponent() const { return pow_exponent_; }

protected:
  std::string var_name_;
  double constant_value_{0.0};
  int pow_exponent_{0};
};

std::shared_ptr<const Node> make_variable_node(const std::string& name);
std::shared_ptr<const Node> make_constant_node(double value);
std::shared_ptr<const Node> make_binary_node(NodeKind kind,
                                             std::shared_ptr<const Node> lhs,
                                             std::shared_ptr<const Node> rhs);
std::shared_ptr<const Node> make_unary_node(NodeKind kind,
                                            std::shared_ptr<const Node> child);
std::shared_ptr<const Node> make_pow_node(std::shared_ptr<const Node> base,
                                          int exponent);

bool is_variable_node(const std::shared_ptr<const Node>& node);
bool is_same_variable(const std::shared_ptr<const Node>& a,
                      const std::shared_ptr<const Node>& b);

}  // namespace ad
