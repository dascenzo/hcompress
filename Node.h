#pragma once
#include <memory>
#include <string>
#include "Definitions.h"

class Node {
public:
  virtual ~Node() = default;
};
class InternalNode : public Node {
public:
  InternalNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
  InternalNode(InternalNode&&) = delete;
  InternalNode& operator =(InternalNode&&) = delete;
  Node& left() const;
  Node& right() const;
private:
  std::unique_ptr<Node> m_left, m_right;
};
class LeafNode : public Node {
public:
  LeafNode(Word word);
  LeafNode(const LeafNode&) = delete;
  LeafNode& operator =(const LeafNode&) = delete;
  const Word& word() const;
private:
  Word m_word;
};
