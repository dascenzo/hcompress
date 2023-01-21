#pragma once
#include "Exception.h"
#include <cstring>
#include <arpa/inet.h>
#include <ostream>
#include <istream>

namespace Persist
{
  template<class IntType>
  struct HostToNet;
  template<>
  struct HostToNet<std::uint64_t>
    { auto operator()(std::uint64_t val) const { return htonll(val); } };
  template<>
  struct HostToNet<std::uint32_t>
    { auto operator()(std::uint32_t val) const { return htonl(val); } };
  template<>
  struct HostToNet<std::uint16_t>
    { auto operator()(std::uint16_t val) const { return htons(val); } };
  template<>
  struct HostToNet<std::uint8_t>
    { auto operator()(std::uint8_t val) const { return val; } };

  template <class IntType>
  void writeBigEndian(std::ostream& out, IntType value) {
    value = HostToNet<IntType>()(value);
    char bytes[sizeof(IntType)];
    std::memcpy(bytes, &value, sizeof(IntType));
    for (std::size_t i = 0; i < sizeof(IntType); ++i) {
      out.put(bytes[i]);
    }
  }
  template<class IntType>
  struct NetToHost;
  template<>
  struct NetToHost<std::uint64_t>
    { auto operator()(std::uint64_t val) const { return ntohll(val); } };
  template<>
  struct NetToHost<std::uint32_t>
    { auto operator()(std::uint32_t val) const { return ntohl(val); } };
  template<>
  struct NetToHost<std::uint16_t>
    { auto operator()(std::uint16_t val) const { return ntohs(val); } };
  template<>
  struct NetToHost<std::uint8_t>
    { auto operator()(std::uint8_t val) const { return val; } };

  template<class IntType>
  void readBigEndian(std::istream& in, IntType& value) {
    char bytes[sizeof(IntType)];
    std::size_t count = 0;
    while (count < sizeof(IntType)) {
      if (in.peek() == EOF) {
        throw StreamReadFailure();
      }
      in.get(bytes[count++]);
    }
    std::memcpy(&value, bytes, sizeof(IntType));
    value = NetToHost<IntType>()(value);
  }

  std::string readChars(std::istream& in, std::size_t count);
}
