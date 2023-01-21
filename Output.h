#pragma once
#include <iosfwd>
#include "Statistics.h"

/**
 * string_view sv = "hello";
 * Example: printf("%.*s %s", PRINTF_STRING_VIEW(sv), "world");
 */
#define PRINTF_STRING_VIEW(SV) \
    (SV.size() <= INT_MAX ? static_cast<int>(SV.size()) : INT_MAX), SV.data()
/**
 * Handles outputting messages to the user in a consistent way 
 */
namespace Output {
  /** 
   * Sets the program name. Must be called before calling
   * any other Output functions. Argument is expected to be valid
   * throughout life of program, e.g. argv[0]
   */
  void setProgramName(const char*);

  /**
   * Print to stderr in the format
   *     programName: message1: message2
   * Where message1 is required. If message2 is ommitted then
   *     programName: message1
   */
  void error(const char* message1, const char* message2 = nullptr);

  /**
   * Print program usage to stream
   */
  void usage(std::ostream& stream);

  /**
   * Print statistical information about the compression or decompression
   * operation performed
   */
  void statistics(const Statistics& stats);
};
