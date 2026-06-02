#include "ad/evaluator.hpp"

#include "ad/detail/node.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace ad {

namespace {

void topo_visit(const std::shared_ptr<const Node>& node,
                std::unordered_set<const Node*>& visiting,
                std::unordered_set<const Node*>& visited,
                std::vector<std::shared_ptr<const Node>>& order) {
  if (!node) {
    return;
  }
  const Node* key = node.get();
  if (visited.count(key)) {
    return;
  }
  if (visiting.count(key)) {
    throw std::logic_error("cycle detected in expression graph");
  }
  visiting.insert(key);
  for (const auto& child : node->children()) {
    topo_visit(child, visiting, visited, order);
  }
  visiting.erase(key);
  visited.insert(key);
  order.push_back(node);
}

std::vector<std::shared_ptr<const Node>> build_topo_order(
    const std::initializer_list<Expr>& roots) {
  std::unordered_set<const Node*> visiting;
  std::unordered_set<const Node*> visited;
  std::vector<std::shared_ptr<const Node>> order;
  for (const auto& root : roots) {
    topo_visit(root.node(), visiting, visited, order);
  }
  return order;
}

}  // namespace

Evaluator::Evaluator(std::initializer_list<Expr> roots) {
  const auto order = build_topo_order(roots);

  std::unordered_map<const Node*, std::size_t> slot_of;
  plan_.reserve(order.size());

  for (const auto& node : order) {
    CompiledNode entry;
    entry.node = node;
    for (const auto& child : node->children()) {
      entry.child_slots.push_back(slot_of.at(child.get()));
    }
    entry.slot = plan_.size();
    slot_of[node.get()] = entry.slot;
    plan_.push_back(std::move(entry));

    if (node->kind() == NodeKind::Variable) {
      var_slots_[node->var_name()] = entry.slot;
    }
  }

  root_slots_.reserve(roots.size());
  for (const auto& root : roots) {
    if (!root.node()) {
      throw std::invalid_argument("evaluator root expression is empty");
    }
    root_slots_.push_back(slot_of.at(root.node().get()));
  }

  workspace_.resize(plan_.size(), 0.0);
}

std::vector<double> Evaluator::evaluate(
    const std::unordered_map<std::string, double>& bindings) const {
  for (const auto& [name, slot] : var_slots_) {
    const auto it = bindings.find(name);
    if (it == bindings.end()) {
      throw std::invalid_argument("missing binding for variable: " + name);
    }
    workspace_[slot] = it->second;
  }

  for (const auto& entry : plan_) {
    switch (entry.node->kind()) {
      case NodeKind::Variable:
        break;
      case NodeKind::Constant:
        workspace_[entry.slot] = entry.node->constant_value();
        break;
      default: {
        std::vector<double> child_values;
        child_values.reserve(entry.child_slots.size());
        for (const auto child_slot : entry.child_slots) {
          child_values.push_back(workspace_[child_slot]);
        }
        workspace_[entry.slot] = entry.node->forward(child_values);
        break;
      }
    }
  }

  std::vector<double> results;
  results.reserve(root_slots_.size());
  for (const auto root_slot : root_slots_) {
    results.push_back(workspace_[root_slot]);
  }
  return results;
}

}  // namespace ad
