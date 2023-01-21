#pragma once
#include <limits>

/** Do x+= y only if it wont wrap */
template<typename T1, typename T2>
bool safeAddAssign(T1& x, T2 y) {
  static_assert(std::is_unsigned<T1>::value, "x must be unsigned");
  static_assert(std::is_unsigned<T2>::value, "y must be unsigned");

  if (std::numeric_limits<T1>::max() - y < x) {
    return false;
  }
  x += y;
  return true;
}
