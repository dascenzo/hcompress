#include "CompressedFileHeader.h"
#include "Persist.h"
#include "Exception.h"
#include "CMessage.h"

static CMessage getUnencodedTooLargeMsg(WordSize noWordSize, WordSize wordSize) {
  return CMessage("unencoded byte count not less than word size")
      << " - bytes: " << noWordSize << " - max: " << (wordSize - 1);
}
/**
 * Header: word_size,
 *         word_count,
 *         no_word_data_size,
 *         no_word_data,
 *         encoded_huffman_tree
 */

std::tuple<HuffmanTree, WordCount, NoWord>
ReadCompressedFileHeader(std::istream& in) {
  try {
    WordSize wordSize;
    Persist::readBigEndian(in, wordSize);
    if (wordSize == 0) {
      throw InvalidCompressionHeader("invalid word size: 0");
    }
    WordCount wordCount;
    Persist::readBigEndian(in, wordCount);

    WordSize noWordSize;
    Persist::readBigEndian(in, noWordSize);
    if (noWordSize >= wordSize) {
      throw InvalidCompressionHeader(
          getUnencodedTooLargeMsg(noWordSize, wordSize).c_str());
    }
    auto noWord = NoWord(Persist::readChars(in, noWordSize));
    auto tree = HuffmanTree::fromStream(in, wordSize);

    if (wordCount != 0 && tree.root() == tree.end()) {
      throw InvalidCompressionHeader("empty huffman tree");
    }
    return {std::move(tree), wordCount, std::move(noWord)}; 
  }
  catch (const StreamReadFailure& e) {
    throw InvalidCompressionHeader(e.what());
  }
  catch (const HuffmanTreeError& e) {
    throw InvalidCompressionHeader(e.what());
  }
}
void WriteCompressedFileHeader(std::ostream& out, const HuffmanTree& tree,
    const WordCount& totalWords, const NoWord& noWord) {

  ASSERT(noWord.size() < tree.wordSize(),
      getUnencodedTooLargeMsg(noWord.size(), tree.wordSize()));

  Persist::writeBigEndian(out, tree.wordSize());
  Persist::writeBigEndian(out, totalWords);
  Persist::writeBigEndian(out, noWord.size());
  out << noWord;
  out << tree;
}
