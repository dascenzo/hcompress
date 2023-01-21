#pragma once
#include "Definitions.h"
#include <ostream>
#include <istream>

/**
 * Compress input stream in to output stream out. The input data will be
 * treated as a sequence of (in_byte_count / wordSize) words for the purposes
 * of Huffman encoding. Any data (in_byte_count % wordSize) is just stored
 * unaltered in the compressed output stream.
 */
void compress(std::istream& in, std::ostream& out, WordSize wordSize);

/**
 * Decompress the input stream in to output stream out.
 */
void decompress(std::istream& in, std::ostream& out);

