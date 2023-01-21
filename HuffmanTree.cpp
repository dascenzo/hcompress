#include "HuffmanTree.h"
#include <queue>
#include "Persist.h"
#include "SafeMath.h"
#include "Node.h"
#include "ValuedTree.h"

HuffmanTree HuffmanTree::fromStream(std::istream& in, WordSize wordSize) {
  ASSERT(wordSize > 0, "invalid word size: 0");
  HuffmanTree tree;
  tree.m_wordSize = wordSize;
  tree.m_tree = tree.Deserialize(in);
  return tree;
}
HuffmanTree HuffmanTree::fromFrequencyData(const FrequencyData& frequencyData) {
  using SubTree = ValuedTree<WordCount>;
  using MinFrequencyQueue = PriorityQueue<SubTree, std::greater<SubTree>>;

  HuffmanTree tree;
  tree.m_wordSize = frequencyData.wordSize();

  if (frequencyData.uniqueWords() > 0) {
    auto q = MinFrequencyQueue(std::greater<SubTree>());
    for (const auto& [word, frequency] : frequencyData) {
      q.Push(SubTree(frequency, std::make_unique<LeafNode>(word)));
    }
    while (q.Size() > 1) {
      auto t1 = q.Pop();
      auto t2 = q.Pop();
      auto value = t1.value();
      if (!safeAddAssign(value, t2.value())) {
	throw HuffmanTreeError("word count exceeded maximum"); 
      }
      auto merged = std::make_unique<InternalNode>(t1.release(), t2.release());
      q.Push(SubTree(value, std::move(merged)));
    }
    auto root = q.Pop();
    tree.m_tree = root.release();
  }
  return tree;
}
TreeIterator HuffmanTree::root() const {
  return TreeIterator(m_tree.get());
}
TreeIterator HuffmanTree::end() const {
  return TreeIterator(nullptr);
}
CodeTable HuffmanTree::getCodeTable() const {
  auto codeTable = CodeTable();
  if (m_tree) {
    auto q = std::queue< std::pair<Node*, BitString> >();
    if (auto n = dynamic_cast<InternalNode*>(m_tree.get()); n) {
      q.emplace( &n->left(), BitString{false} );
      q.emplace( &n->right(), BitString{true} );
    }
    while (!q.empty()) { 
      const auto pr = q.front();
      q.pop();
      if (auto leaf = dynamic_cast<LeafNode*>(pr.first); leaf) {
        codeTable[leaf->word()] = pr.second;
      }
      else {
        auto n = dynamic_cast<InternalNode*>(pr.first);
        ASSERT(n, "not an expected Node subclass");
	auto v = pr.second;
	v.push_back(false);
	q.emplace(&n->left(), v);

	v.back() = true;
	q.emplace(&n->right(), v);
      }
    }
  }
  return codeTable;
}
enum : char {
  HUFFSYM_INTERNAL_NODE = 'X',
  HUFFSYM_DATA_NODE = 'D',
  HUFFSYM_NULL_NODE = '/'
};
void HuffmanTree::Serialize(Node* n, std::ostream& o) const {
  if (n) {
    if (auto leaf = dynamic_cast<LeafNode*>(n); leaf) {
      o.put(HUFFSYM_DATA_NODE);
      o << leaf->word();
      Serialize(nullptr, o);
      Serialize(nullptr, o);
    }
    else {
      o.put(HUFFSYM_INTERNAL_NODE);
      auto internal = dynamic_cast<InternalNode*>(n);
      ASSERT(internal, "not an expected Node subclass");
      Serialize(&internal->left(), o);
      Serialize(&internal->right(), o);
    }
  } else {
    o.put(HUFFSYM_NULL_NODE);
  }
}
std::unique_ptr<Node> HuffmanTree::Deserialize(std::istream& in) const {
  if (in.peek() == EOF) {
    throw HuffmanTreeError("not enough data");
  }
  char c;
  in.get(c);
  switch (c) {
  case HUFFSYM_INTERNAL_NODE: {
      auto left = Deserialize(in);
      auto right = Deserialize(in);
      return std::make_unique<InternalNode>(std::move(left), std::move(right));
    }
  case HUFFSYM_DATA_NODE: {
      std::string wordChars = [&](){
	try {
	  return Persist::readChars(in, m_wordSize);
	}
	catch (const StreamReadFailure& e) {
	  throw HuffmanTreeError("couldn't read data word");
	}
      }();
      Word w(std::move(wordChars));
      auto left = Deserialize(in);
      auto right = Deserialize(in);
      if (left) {
        throw HuffmanTreeError("data node shouldn't have left child");
      }
      if (right) {
        throw HuffmanTreeError("data node shouldn't have right child");
      }
      return std::make_unique<LeafNode>(w);
    }
  case HUFFSYM_NULL_NODE: {
      return nullptr;
    }
  default: {
      char errorMessage[256];
      std::sprintf(errorMessage, "unknown element found in serialized tree: %02hhX",
                   static_cast<unsigned char>(c));
      throw HuffmanTreeError(errorMessage);
    }
  }
}
std::ostream& operator<<(std::ostream& o, const HuffmanTree& tree) {
  tree.Serialize(tree.m_tree.get(), o);
  return o;
}
