#include "BitStream.h"
#include "Exception.h"

KeyedSharedStorage<std::istream*, BitStream::PositionBlock>
    BitStream::positionBlocks;

BitStream::BitStream(std::istream& in)
  : m_pos{ KeyedSharedInsert(&in, PositionBlock{ &in }, positionBlocks) }
{
}
BitStream& BitStream::operator >>(bool& bit) {
  if (m_pos->bitPos == BitEnd) {
    try {
      m_pos->in->get(m_pos->currentInput);
    }
    catch (const std::ios::failure& e) {
      throw StreamReadFailure();
    }
    m_pos->bitPos = BitBegin;
  }
  bit = (m_pos->currentInput & (1 << --m_pos->bitPos));
  return *this;
}
