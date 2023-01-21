#pragma once
#include <cstdint>
#include <vector>
#include <limits>
#include "SizedString.h"

/** Total number of words in a file */
using WordCount = std::uint64_t;

/** Number of bytes that make up a word */
using WordSize = std::uint8_t;

using BitString = std::vector<bool>;

/**
 * An indivisible block of information that is represented in
 * compressed form by a code word. Each word in the data to
 * be compressed is a leaf in the Huffman tree.
 */
using Word = SizedString<WordSize>;

/**
 * Excess data, i.e.  data - (word_count * word_size), which is
 * stored unencoded
 */
using NoWord = SizedString<WordSize>;
