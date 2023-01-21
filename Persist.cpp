#include "Persist.h"

std::string Persist::readChars(std::istream& in, std::size_t count) {
  std::string data;
  data.reserve(count);
  char c;
  while (count-- > 0) {
    if (in.peek() == EOF) {
      throw StreamReadFailure();
    }
    in.get(c);
    data.push_back(c);
  }
  return data;
}
