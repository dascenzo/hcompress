#include "BitStream.h"
#include "HuffmanTree.h"
#include "FrequencyData.h"
#include "CompressedFileHeader.h"
#include "Persist.h"
#include "BitsToBytes.h"
#include "Compress.h"
#include "Exception.h"

void compress(std::istream& in, std::ostream& out, WordSize wordSize) {
  ASSERT(wordSize > 0, "invalid word size");
  const auto frequencyData = FrequencyData(in, wordSize);
  const auto tree = HuffmanTree::fromFrequencyData(frequencyData);
  WriteCompressedFileHeader(
      out, tree, frequencyData.totalWords(), frequencyData.noWord());
  const auto codeTable = tree.getCodeTable();
  // if |word set| < 2, the word and the count alone are enough to decompress
  if (codeTable.size() > 1) {
    auto bitsToBytes = BitsToBytes(out);
    auto wordsToEncodeCount = frequencyData.totalWords();
    in.seekg(0);
    while (wordsToEncodeCount-- > 0) {
      const auto word = Word(Persist::readChars(in, wordSize));
      const auto word2CodeWord = codeTable.find(word);
      ASSERT(word2CodeWord != codeTable.end(), "missing code word for word in file");
      bitsToBytes << word2CodeWord->second;
    }
    bitsToBytes.flush();
  }
}
void decompress(std::istream& in, std::ostream& out) {
  const auto [tree, totalWords, noWord] = ReadCompressedFileHeader(in);
  auto bitStream = BitStream(in);
  auto readWordCount = WordCount(0);
  auto treeItr = tree.root();
  bool bit;
  while (readWordCount < totalWords) {
    if (const auto leaf = treeItr.getLeafIfAtLeaf()) {
      out << leaf->word();
      ++readWordCount;
      treeItr = tree.root();
    }
    else {
      bitStream >> bit;
      if (bit == 1) {
        treeItr.traverseDownRight();
      } else {
        treeItr.traverseDownLeft();
      }
    }
  }
  out << noWord;
}
