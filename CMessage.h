#pragma once
#include <type_traits>
#include <limits>
#include <charconv>
#include <system_error>
#include "Exception.h"

/**
 * Message backed by stack buffer (i.e. memory is not allocated via 'new') into
 * which strings and data can be written. The message will be ellipsized if
 * needed, and is limited in size to BufSize.
 */
class CMessage {
public:
  static constexpr int BufSize = 512;

  CMessage() = default;
  CMessage(const char* initialMsg);

  /** Appends string msg */
  CMessage& operator<<(const char* msg);

  /** Appends integral-value */
  template<class T,
           class = std::enable_if_t<std::is_integral_v<T>>
          >
  CMessage& operator<<(T value) {
    constexpr auto bufsize = std::numeric_limits<T>::digits10
                             + 1 // e.g. 8 bit => digits10 == 2 => 3 (for > 99)
                             + std::is_signed_v<T> // negative sign
                             + 1; // null terminator
    char buf[bufsize];
    const std::to_chars_result result = std::to_chars(buf, buf + bufsize-1, value);
    ASSERT(result.ec == std::errc(), "expect to_chars to always succeed");
    *result.ptr = '\0';
    return (*this << buf);
  }
  const char* c_str() const { return m_buf; }
  operator std::string_view() const { return c_str(); }
private:
  char m_buf[BufSize] = {};
};
