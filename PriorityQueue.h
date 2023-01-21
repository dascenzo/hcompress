#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include <type_traits>

/**
 * Priority queue that allows elements to be std::move'd out of
 * it (unlike std::priority_queue). Reference: https://stackoverflow.com/a/22050490
 */
template<class T, class Compare = std::less<T>>
class PriorityQueue {
  /*  MoveAssignable and MoveConstructible are the requirements on the
      elements for std::*_heap operations. These are nothrow because
      an exception during *_heap would corrupt the structure.
      Also nothrow MoveConstructible so that in Pop,
      1) an exception when moving into the result doesn't corrupt the structure.
      2) an exception when moving while returning doesn't lose the element. */
  static_assert(std::is_nothrow_move_constructible_v<T>
                && std::is_nothrow_move_assignable_v<T>);
private:
  std::vector<T> m_elements;
  Compare m_compare;
public:
  using size_type = typename decltype(m_elements)::size_type;
  explicit PriorityQueue(const Compare& compare = Compare())
    : m_compare(compare) {
  }
  void Push(T element) {
    m_elements.push_back(std::move(element));
    std::push_heap(m_elements.begin(), m_elements.end(), m_compare);
  }
  T Pop() {
    std::pop_heap(m_elements.begin(), m_elements.end(), m_compare);
    T result = std::move(m_elements.back());
    m_elements.pop_back();
    return result;
  }
  size_type Size() const { return m_elements.size(); } 
  bool Empty() const { return m_elements.empty(); }
};
