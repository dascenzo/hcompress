#pragma once
#include <string>
#include "Definitions.h"
#include <variant>

/**
 * Represents parsed command line arguments. Parsing is done by
 * the constructor and the result is exposed through the variant member.
 */
class ProgramParameters {
public:
  ProgramParameters(int argc, char* argv[]);
  static const char* const usage;

  struct CompressionBase {
    std::string inputFile;
    bool keep = false, verbose = false; 
  };
  struct Compress : CompressionBase {
    WordSize wordSize = 1; // in bytes
  };
  struct Decompress : CompressionBase {};
  struct ShowHelp {};
  using VariantType = std::variant<Compress, Decompress, ShowHelp>;
  const VariantType& variant() const { return m_variant; }
private:
  /**
   * The particular variant of options the program was invoked with.
   * This specifies what the program is to do.
   */
   VariantType m_variant;
};
