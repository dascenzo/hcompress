#include "CMessage.h"
#include <cstdio>
#include <cstring>

CMessage::CMessage(const char* initialMsg)
  : CMessage() {
  *this << initialMsg;
}
CMessage& CMessage::operator<<(const char* msg) {
  const std::size_t len = std::strlen(m_buf);
  if (len + 1 != BufSize) {
    const std::size_t left = BufSize - (len + 1);
    const int n = std::snprintf(m_buf + len, left, "%s", msg);
    ASSERT(!(n < 0), "invalid formatting");
    if (n >= left) {
      std::strcpy(m_buf + (BufSize - 4), "...");
    }
  }
  return *this;
}
