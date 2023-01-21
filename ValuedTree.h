#pragma once
#include "Node.h"
#include <memory>
#include <type_traits>
#include "Exception.h"

/**
 * A tree and an associated numeric value
 */
template<typename T>
class ValuedTree {
  static_assert(std::is_arithmetic<T>::value && !std::is_const<T>::value);
public:
  ValuedTree(T value, std::unique_ptr<Node> tree)
    : m_value(value), m_tree(std::move(tree))
  {
    ASSERT(m_tree, "null tree");
  }
  ~ValuedTree() = default;
  ValuedTree(ValuedTree&& other) noexcept {
    m_value = other.m_value;
    m_tree = other.release();
  }
  ValuedTree& operator =(ValuedTree&& other) noexcept {
    if (this != &other) {
      m_value = other.m_value;
      m_tree = other.release();
    }
    return *this;
  }
  T value() const {
    return m_value;
  }
  std::unique_ptr<Node> release() noexcept {
    m_value = T();
    return std::unique_ptr<Node>(m_tree.release());
  } 
  bool operator >(const ValuedTree& other) const {
    return m_value > other.m_value;
  }
private:
  T m_value;
  std::unique_ptr<Node> m_tree;
};
