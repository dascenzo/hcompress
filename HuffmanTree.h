#pragma once
#include <memory>
#include <ostream>
#include <istream>
#include <unordered_map>
#include "Definitions.h"
#include "PriorityQueue.h"
#include "FrequencyData.h"
#include "Node.h"
#include "TreeIterator.h"

using CodeTable = std::unordered_map<Word, BitString/*code word*/, Word::Hash>;

class HuffmanTree {
public:
  static HuffmanTree fromFrequencyData(const FrequencyData&);
  static HuffmanTree fromStream(std::istream&, WordSize wordSize);

  TreeIterator root() const;
  TreeIterator end() const;
  WordSize wordSize() const { return m_wordSize; }
  CodeTable getCodeTable() const;

  friend std::ostream& operator<<(std::ostream&, const HuffmanTree&);
private:
  HuffmanTree() = default;

  void Serialize(Node* n, std::ostream& o) const;
  std::unique_ptr<Node> Deserialize(std::istream& in) const;

  WordSize m_wordSize = 1;
  std::unique_ptr<Node> m_tree;
};
