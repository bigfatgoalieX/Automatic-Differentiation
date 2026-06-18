# CSE Explained: Pointer Sharing vs Structural Sharing

The original implementation already allowed some node sharing, but only when
the same `Expr` object was reused. The CSE change extends that to structurally
equivalent expressions that are written separately.

## Before CSE: sharing by pointer

Each `Expr` holds a `shared_ptr<const Node>`. In the original implementation,
operator construction always created a fresh node:

```cpp
Expr Expr::operator*(const Expr& rhs) const {
  return detail::make_expr(make_binary_node(NodeKind::Mul, node_, rhs.node_));
}
```

The old factory then allocated unconditionally:

```cpp
std::shared_ptr<const Node> make_binary_node(
    NodeKind kind,
    std::shared_ptr<const Node> lhs,
    std::shared_ptr<const Node> rhs) {
  return std::make_shared<BinaryNode>(kind, std::move(lhs), std::move(rhs));
}
```

So writing the same formula twice produced two different nodes:

```cpp
const auto a = x * p;
const auto b = x * p;

ad::same_expr(a, b);  // false before CSE
```

Conceptually:

```text
a.node() -> BinaryNode #1: Mul(x, p)
b.node() -> BinaryNode #2: Mul(x, p)
```

The formulas are mathematically identical, but their node pointers are
different.

However, sharing did happen when the exact same `Expr` object was reused:

```cpp
const auto a = x * p;
const auto loss = a + a;
```

Conceptually:

```text
a.node()    -> BinaryNode #1: Mul(x, p)

loss.node() -> Add(a.node(), a.node())
```

Both sides of the `Add` refer to the same existing `a.node()`. This is pointer
sharing: the graph shares nodes only when user code reuses the same expression
handle.

## After CSE: sharing by structure

With CSE, factory functions build a structural key before creating a node. For a
binary expression such as `x * p`, the key is based on:

- node kind: `Mul`
- child node identities: `x.node()` and `p.node()`
- any operation-specific metadata

Conceptually:

```cpp
NodeKey key;
key.kind = NodeKind::Mul;
key.children = {lhs.get(), rhs.get()};
```

The factory checks a cache:

```text
first  x * p: cache miss -> create BinaryNode #1
second x * p: cache hit  -> reuse  BinaryNode #1
```

So now:

```cpp
const auto a = x * p;
const auto b = x * p;

ad::same_expr(a, b);  // true after CSE
```

The implementation also canonicalizes commutative operators:

```cpp
const auto a = x * p;
const auto b = p * x;

ad::same_expr(a, b);  // true after CSE
```

For `Add` and `Mul`, operand order is normalized before the cache lookup. That
means both expressions use the same structural key.

## Example with `exp`

Before CSE:

```cpp
const auto loss = ad::exp(x * p) + ad::exp(x * p);
```

could build separate equivalent subgraphs:

```text
Mul(x, p)      Mul(x, p)
   |              |
 Exp            Exp
    \            /
        Add
```

After CSE, repeated construction reuses the same nodes:

```text
       Mul(x, p)
          |
         Exp
        /   \
       Add
```

This is equivalent to manually writing:

```cpp
const auto t = ad::exp(x * p);
const auto loss = t + t;
```

but users no longer have to introduce `t` manually for this kind of sharing.

## What does not change

Variable identity is still pointer-based:

```cpp
const auto x1 = ad::Expr::variable("x");
const auto x2 = ad::Expr::variable("x");

ad::same_expr(x1, x2);  // false
```

Two separate calls to `Expr::variable("x")` create two distinct variables, even
if their names match. This preserves the original variable identity semantics.

## Summary

- Original implementation: sharing by pointer reuse.
- New CSE implementation: sharing by structural equivalence during node
  construction.
- `a + a` already shared because both operands reused the same `Expr`.
- `x * p` written twice did not share before, but now it does.
- `x * p` and `p * x` now share because `Mul` is commutative and canonicalized.
