#include "TreeIterator.h"
#include "Exception.h"

// TreeIterator
TreeIterator::TreeIterator(Node* node)
  : m_node(node)
{
}
void TreeIterator::traverseNonInternal() {
  if (auto n = dynamic_cast<const LeafNode*>(m_node)) {
    m_node = nullptr;
  }
  else {
    ASSERT(!m_node, "unknown node type");
    ASSERT(false, "incremented end iterator");
  }
}
void TreeIterator::traverseDownLeft() {
  if (auto n = dynamic_cast<const InternalNode*>(m_node)) {
    m_node = &n->left();
  }
  else {
    traverseNonInternal();
  }
}
void TreeIterator::traverseDownRight() {
  if (auto n = dynamic_cast<const InternalNode*>(m_node)) {
    m_node = &n->right();
  }
  else {
    traverseNonInternal();
  }
}
const LeafNode* TreeIterator::getLeafIfAtLeaf() const {
  return dynamic_cast<const LeafNode*>(m_node);
}
bool TreeIterator::operator ==(const TreeIterator& other) const {
  return m_node == other.m_node;
}
bool TreeIterator::operator !=(const TreeIterator& other) const {
  return !(*this == other);
}
