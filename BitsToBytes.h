#pragma once
#include <vector>
#include <climits>
#include <ostream>
#include "KeyedShared.h"
#include "Definitions.h"

/**
 * Writes data to an output stream where the data is provided as
 * a sequence of bits. I.e. this class converts bits to bytes
 * and outputs the bytes to the given output stream.
 */
class BitsToBytes {
public:
  explicit BitsToBytes(std::ostream& out);

  /**
   * Send bits to the given output stream as bytes. Bits are
   * stored until a full byte can be made.
   */
  BitsToBytes& operator<<(const BitString& bits);
  /**
   * Write any remaining bits that have not made up a full byte to the stream.
   * Zeros will be appended to make the full byte.
   */
  void flush();
private:
  void commitByte();
  static constexpr std::size_t BitBegin = CHAR_BIT;
  static constexpr std::size_t BitEnd = 0;

  /**
   * Data required for outputting
   */
  struct PositionBlock {
    std::ostream* const out;
    std::size_t bitPos = BitBegin;
    char outByte = 0;
  };
  static KeyedSharedStorage<std::ostream*, PositionBlock> positionBlocks;

  KeyedSharedHdl<PositionBlock> m_pos;
};
