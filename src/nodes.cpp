#include "ad/detail/node.hpp"

#include "ad/custom_op.hpp"
#include "ad/detail/derivative.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ad {

namespace {

class VariableNode : public Node {
public:
  explicit VariableNode(std::string name) { var_name_ = std::move(name); }

  NodeKind kind() const override { return NodeKind::Variable; }
  std::vector<std::shared_ptr<const Node>> children() const override { return {}; }

  Expr differentiate(const Expr& self, const Expr& wrt) const override {
    if (is_same_variable(self.node(), wrt.node())) {
      return Expr::constant(1.0);
    }
    return Expr::constant(0.0);
  }

  double forward(const std::vector<double>&) const override {
    throw std::logic_error("variable forward is handled by Evaluator");
  }
};

class ConstantNode : public Node {
public:
  explicit ConstantNode(double value) { constant_value_ = value; }

  NodeKind kind() const override { return NodeKind::Constant; }
  std::vector<std::shared_ptr<const Node>> children() const override { return {}; }

  Expr differentiate(const Expr&, const Expr&) const override {
    return Expr::constant(0.0);
  }

  double forward(const std::vector<double>&) const override { return constant_value_; }
};

class UnaryNode : public Node {
public:
  UnaryNode(NodeKind kind, std::shared_ptr<const Node> child)
      : kind_(kind), child_(std::move(child)) {}

  NodeKind kind() const override { return kind_; }
  std::vector<std::shared_ptr<const Node>> children() const override { return {child_}; }

  Expr differentiate(const Expr& self, const Expr& wrt) const override;
  double forward(const std::vector<double>& child_values) const override;

private:
  NodeKind kind_;
  std::shared_ptr<const Node> child_;
};

class BinaryNode : public Node {
public:
  BinaryNode(NodeKind kind, std::shared_ptr<const Node> lhs,
             std::shared_ptr<const Node> rhs)
      : kind_(kind), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  NodeKind kind() const override { return kind_; }
  std::vector<std::shared_ptr<const Node>> children() const override {
    return {lhs_, rhs_};
  }

  Expr differentiate(const Expr& self, const Expr& wrt) const override;
  double forward(const std::vector<double>& child_values) const override;

private:
  NodeKind kind_;
  std::shared_ptr<const Node> lhs_;
  std::shared_ptr<const Node> rhs_;
};

class PowNode : public Node {
public:
  PowNode(std::shared_ptr<const Node> base, int exponent)
      : base_(std::move(base)), exponent_(exponent) {
    pow_exponent_ = exponent;
  }

  NodeKind kind() const override { return NodeKind::Pow; }
  std::vector<std::shared_ptr<const Node>> children() const override { return {base_}; }

  Expr differentiate(const Expr& self, const Expr& wrt) const override;
  double forward(const std::vector<double>& child_values) const override;

private:
  std::shared_ptr<const Node> base_;
  int exponent_;
};

class CustomNode : public Node {
public:
  CustomNode(std::string name, std::vector<std::shared_ptr<const Node>> inputs,
             ForwardFn forward, SymbolicGradFn symbolic_grad)
      : name_(std::move(name)),
        inputs_(std::move(inputs)),
        forward_(std::move(forward)),
        symbolic_grad_(std::move(symbolic_grad)) {}

  NodeKind kind() const override { return NodeKind::Custom; }
  std::vector<std::shared_ptr<const Node>> children() const override { return inputs_; }

  Expr differentiate(const Expr& self, const Expr& wrt) const override;
  double forward(const std::vector<double>& child_values) const override;

  const std::string& op_name() const { return name_; }

private:
  std::string name_;
  std::vector<std::shared_ptr<const Node>> inputs_;
  ForwardFn forward_;
  SymbolicGradFn symbolic_grad_;
};

}  // namespace

std::shared_ptr<const Node> make_variable_node(const std::string& name) {
  return std::make_shared<VariableNode>(name);
}

std::shared_ptr<const Node> make_constant_node(double value) {
  return std::make_shared<ConstantNode>(value);
}

std::shared_ptr<const Node> make_binary_node(NodeKind kind,
                                             std::shared_ptr<const Node> lhs,
                                             std::shared_ptr<const Node> rhs) {
  return std::make_shared<BinaryNode>(kind, std::move(lhs), std::move(rhs));
}

std::shared_ptr<const Node> make_unary_node(NodeKind kind,
                                            std::shared_ptr<const Node> child) {
  return std::make_shared<UnaryNode>(kind, std::move(child));
}

std::shared_ptr<const Node> make_pow_node(std::shared_ptr<const Node> base,
                                          int exponent) {
  return std::make_shared<PowNode>(std::move(base), exponent);
}

bool is_variable_node(const std::shared_ptr<const Node>& node) {
  return node && node->kind() == NodeKind::Variable;
}

bool is_same_variable(const std::shared_ptr<const Node>& a,
                      const std::shared_ptr<const Node>& b) {
  return is_variable_node(a) && is_variable_node(b) && a == b;
}

Expr UnaryNode::differentiate(const Expr& /*self*/, const Expr& wrt) const {
  const Expr child = detail::make_expr(child_);
  const Expr d_child = differentiate_expr(child, wrt);

  switch (kind_) {
    case NodeKind::Neg:
      return -d_child;
    case NodeKind::Square:
      return Expr::constant(2.0) * child * d_child;
    case NodeKind::Abs:
      return child / child.abs() * d_child;
    default:
      throw std::logic_error("UnaryNode: invalid kind");
  }
}

double UnaryNode::forward(const std::vector<double>& child_values) const {
  const double x = child_values.at(0);
  switch (kind_) {
    case NodeKind::Neg:
      return -x;
    case NodeKind::Square:
      return x * x;
    case NodeKind::Abs:
      return std::abs(x);
    default:
      throw std::logic_error("UnaryNode: invalid kind");
  }
}

Expr BinaryNode::differentiate(const Expr& /*self*/, const Expr& wrt) const {
  const Expr lhs = detail::make_expr(lhs_);
  const Expr rhs = detail::make_expr(rhs_);
  const Expr d_lhs = differentiate_expr(lhs, wrt);
  const Expr d_rhs = differentiate_expr(rhs, wrt);

  switch (kind_) {
    case NodeKind::Add:
      return d_lhs + d_rhs;
    case NodeKind::Sub:
      return d_lhs - d_rhs;
    case NodeKind::Mul:
      return lhs * d_rhs + rhs * d_lhs;
    case NodeKind::Div:
      return (d_lhs * rhs - lhs * d_rhs) / (rhs * rhs);
    default:
      throw std::logic_error("BinaryNode: invalid kind");
  }
}

double BinaryNode::forward(const std::vector<double>& child_values) const {
  const double a = child_values.at(0);
  const double b = child_values.at(1);
  switch (kind_) {
    case NodeKind::Add:
      return a + b;
    case NodeKind::Sub:
      return a - b;
    case NodeKind::Mul:
      return a * b;
    case NodeKind::Div:
      return a / b;
    default:
      throw std::logic_error("BinaryNode: invalid kind");
  }
}

Expr PowNode::differentiate(const Expr& /*self*/, const Expr& wrt) const {
  const Expr base = detail::make_expr(base_);
  const Expr d_base = differentiate_expr(base, wrt);
  if (exponent_ == 0) {
    return Expr::constant(0.0);
  }
  if (exponent_ == 1) {
    return d_base;
  }
  return Expr::constant(static_cast<double>(exponent_)) * base.pow(exponent_ - 1) *
         d_base;
}

double PowNode::forward(const std::vector<double>& child_values) const {
  return std::pow(child_values.at(0), exponent_);
}

Expr CustomNode::differentiate(const Expr& /*self*/, const Expr& wrt) const {
  std::vector<Expr> input_exprs;
  input_exprs.reserve(inputs_.size());
  for (const auto& in : inputs_) {
    input_exprs.push_back(detail::make_expr(in));
  }

  const std::vector<Expr> grad_out = {Expr::constant(1.0)};
  const std::vector<Expr> partials = symbolic_grad_(input_exprs, grad_out);

  if (partials.size() != inputs_.size()) {
    throw std::logic_error("Custom op symbolic_grad returned wrong arity");
  }

  Expr sum = Expr::constant(0.0);
  for (std::size_t i = 0; i < inputs_.size(); ++i) {
    sum += partials[i] * differentiate_expr(input_exprs[i], wrt);
  }
  return sum;
}

double CustomNode::forward(const std::vector<double>& child_values) const {
  return forward_(child_values);
}

Expr make_custom_op(const std::string& name, const std::vector<Expr>& inputs,
                    ForwardFn forward, SymbolicGradFn symbolic_grad) {
  std::vector<std::shared_ptr<const Node>> nodes;
  nodes.reserve(inputs.size());
  for (const auto& e : inputs) {
    nodes.push_back(e.node());
  }
  return detail::make_expr(std::make_shared<CustomNode>(
      name, std::move(nodes), std::move(forward), std::move(symbolic_grad)));
}

}  // namespace ad
