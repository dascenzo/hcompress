#include "Node.h"
#include "Exception.h"
#include "Definitions.h"

//InternalNode
InternalNode::InternalNode(std::unique_ptr<Node> left,
                           std::unique_ptr<Node> right)
  : m_left(std::move(left)), m_right(std::move(right)) 
{
  ASSERT(m_left && m_right, "internal node missing children");
}
Node& InternalNode::left() const {
   return *m_left;
}
Node& InternalNode::right() const {
  return *m_right;
} 
// LeafNode
LeafNode::LeafNode(Word word)
  : m_word(std::move(word))
{
}
const Word& LeafNode::word() const {
  return m_word;
}
