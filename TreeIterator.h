#pragma once
#include "Node.h"

class TreeIterator {
public:
  explicit TreeIterator(Node* node);
  void traverseDownLeft();
  void traverseDownRight();
  const LeafNode* getLeafIfAtLeaf() const;
  bool operator ==(const TreeIterator&) const;
  bool operator !=(const TreeIterator&) const;
private:
  void traverseNonInternal();
  const Node* m_node;
};
