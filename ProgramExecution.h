#pragma once
#include "Statistics.h"
#include "ProgramParameters.h"
#include <memory>
#include <filesystem>
class ProgramExecImpl;

/**
 * Represents the main action the program performs
 */
class ProgramExecution {
public:
  ~ProgramExecution(); // = default; required for pimpl

  /**
   * Execute
   */
  void operator ()();
  /**
   * Returns a ProgramExecution object that performs compression
   */
  static ProgramExecution compress(const ProgramParameters::Compress& params);
  /**
   * Returns a ProgramExecution object that performs decompression
   */
  static ProgramExecution decompress(const ProgramParameters::Decompress& params);

private:
  ProgramExecution(std::unique_ptr<ProgramExecImpl>, std::filesystem::path,
                   bool showStats, bool keepOriginal);
  void printStatistics(const FileSizeChange&);
  /** Points to object that implements required operations for program execution */
  std::unique_ptr<ProgramExecImpl> m_impl;
  std::filesystem::path m_inFileName;
  bool m_showStats,
       m_keepOriginal;
};
