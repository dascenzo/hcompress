#include "BitsToBytes.h"
#include "Persist.h"

KeyedSharedStorage<std::ostream*, BitsToBytes::PositionBlock>
    BitsToBytes::positionBlocks;

BitsToBytes::BitsToBytes(std::ostream& out)
  : m_pos{ KeyedSharedInsert(&out, PositionBlock{ &out }, positionBlocks) }
{
}
void BitsToBytes::flush() {
  if (m_pos->bitPos != BitBegin) {
    commitByte();
  }
}
BitsToBytes& BitsToBytes::operator<<(const BitString& bits) {
  if (m_pos->bitPos == BitEnd) { // there is an uncommitted byte
    commitByte();
  }
  for (const char bit : bits) {
    m_pos->outByte |= (bit << --m_pos->bitPos);
    if (m_pos->bitPos == BitEnd) {
      commitByte();
    }
  }
  return *this;
}
void BitsToBytes::commitByte() {
  m_pos->out->put(m_pos->outByte);
  m_pos->bitPos = BitBegin;
  m_pos->outByte = 0;
}
