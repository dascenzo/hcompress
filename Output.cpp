#include "Output.h"
#include "ProgramParameters.h"
#include <string_view>
#include <iomanip>
#include <iostream>

static const char* ProgramName;

/**
 * Get POSIX basename() of _path, without modifying the pointed to data,
 * and without allocating memory.
 *
 * Written to enable taking basename() of argv[0] while preserving the argv[0] value.
 * 
 * Returns std::string_view to the basename()
 */
static std::string_view basename_nonModifying(const char* _path) noexcept {
  static char dot[] = ".";

  if (_path == nullptr) {
    return dot;
  }
  std::string_view path(_path);
  if (path.size() == 0) {
    return dot;
  }
  const auto end_pos = path.find_last_not_of("/");
  if (end_pos == 0 || end_pos == std::string_view::npos) {
    return path.substr(0,1);
  }
  auto start_pos = path.find_last_of("/", end_pos - 1);
  start_pos = (start_pos == std::string_view::npos) ? 0 : start_pos + 1;
  return path.substr(start_pos, end_pos - start_pos + 1);
}

void Output::setProgramName(const char* programName) {
  ProgramName = programName;
}
void Output::error(const char* message1, const char* message2) {
  if (message2 != nullptr) {
    std::cerr << ProgramName << ": " << message1
                             << ": " << message2 << std::endl;
  }
  else {
    std::cerr << ProgramName << ": " << message1 << std::endl;
  }
}
void Output::usage(std::ostream& stream) {
  const auto basename = basename_nonModifying(ProgramName);
  stream << "usage: " << basename << " " << ProgramParameters::usage << std::endl;
}
void Output::statistics(const Statistics& stats) {
  std::cout << "in:\t" << stats.sizeChange.originalSize << '\n';
  std::cout << "out:\t" << stats.sizeChange.finalSize << '\n';
  std::cout << "saving:\t";
  switch (stats.spaceSaving.type) {
  case SpaceSaving::EMPTY:
    std::cout << '-' << stats.spaceSaving.emptyFileCompressedSize;
    break;
  case SpaceSaving::PERCENT:
    const auto precision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(1) << stats.spaceSaving.percentSaving << '%'
              // undo stream modifiers
              << std::defaultfloat << std::setprecision(precision);
    break;
  }
  std::cout << std::endl;
}
