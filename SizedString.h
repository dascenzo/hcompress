#pragma once
#include "Exception.h"
#include <ostream>
#include <string>
#include <functional>

/**
 * Immutable std::string that uses smaller type for its size
 */
template <class SizeType>
class SizedString {
public:
  static_assert(std::numeric_limits<SizeType>::max()
                  <= std::numeric_limits<std::size_t>::max());
  SizedString() = default;

  explicit SizedString(std::string data) {
    ASSERT(data.size() <= std::numeric_limits<SizeType>::max(),
           errorMessage(data.size(), std::numeric_limits<SizeType>::max())); 
    m_value = std::move(data);
  }
  SizeType size() const {
    return m_value.size();
  }
  bool operator==(const SizedString& other) const {
    return m_value == other.m_value;
  } 
  bool operator!=(const SizedString& other) const {
    return m_value != other.m_value;
  }
  static_assert(std::numeric_limits<SizeType>::max()
                  <= std::numeric_limits<std::streamsize>::max());

  friend std::ostream& operator<<(std::ostream& o, const SizedString& string) {
    o.write(string.m_value.c_str(), string.m_value.size());
    return o;
  }
  struct Hash {
    std::size_t operator()(const SizedString& string) const noexcept {
      return std::hash<std::string>()(string.m_value);
    }
  };
private:
  const char* errorMessage(std::size_t value, std::size_t max) {
    static char buf[512];
    std::sprintf(buf, "failed data size requirement: %zu <= %zu", value, max);
    return buf;
  }
  std::string m_value;
};
