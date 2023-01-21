#pragma once
#include "KeyedShared.h"
#include <climits>
#include <istream>

/**
 * Converts the bytes in an input stream to a stream of bits. Different
 * BitStream instances referencing the same input stream are synchronized; in
 * other words reading a bit from one will cause the next read of another to
 * read the bit after.
 */
class BitStream {
public:
  explicit BitStream(std::istream& in);
  
  /**
   * Get next bit in stream
   */
  BitStream& operator >>(bool& bit);

private:
  static constexpr std::size_t BitBegin = CHAR_BIT;
  static constexpr std::size_t BitEnd = 0;
  /**
   * Stores information required for data extraction
   */
  struct PositionBlock {
    std::istream* const in;
    std::size_t bitPos = BitEnd;
    char currentInput;
  };
  static KeyedSharedStorage<std::istream*, PositionBlock> positionBlocks;

  KeyedSharedHdl<PositionBlock> m_pos;
};
