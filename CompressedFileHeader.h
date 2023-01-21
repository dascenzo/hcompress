#pragma once
#include <ostream>
#include <istream>
#include "Definitions.h"
#include "HuffmanTree.h"

/**
 * Compressed files need to store information within themselves
 * for decompression. This info will be stored at the beginning
 * of the file and is referred to as the header. It consists of:
 * 1) the serialized huffman tree, 2) the total number of compressed
 * words, and 3) the part of the original file that was not stored
 * in the huffman tree because the file was not an exact multiple
 * of the specified word size.
 */

void WriteCompressedFileHeader(std::ostream& out, const HuffmanTree& tree,
    const WordCount& totalWords, const NoWord& noWord);

std::tuple<HuffmanTree, WordCount, NoWord>
ReadCompressedFileHeader(std::istream& in);

