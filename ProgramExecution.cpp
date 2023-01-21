#include "ProgramExecution.h"
#include "Exception.h"
#include "Statistics.h"
#include "ExistingFile.h"
#include "GuardedNewFile.h"
#include "Compress.h"
#include "Output.h"
#include <utility>
#include <ostream>
#include <istream>

static const std::filesystem::path compressedFileExtension = ".hz";

/**
 * Functions to be implemented to perform the primary
 * action of the program
 */
class ProgramExecImpl {
public:
  /** Return the name of the output file */
  virtual std::filesystem::path makeOutFileName(const std::filesystem::path& inFileName) = 0;
  /** Do the file operation (i.e. compress/decompress) */
  virtual void doOperation(std::istream& inFile, const char* inFileName,
                           std::ostream& outFile, const char* outFileName) = 0;
  /** Convert change in file size to the space saving achieved */
  virtual SpaceSaving spaceSaving(const FileSizeChange&) = 0;

  virtual ~ProgramExecImpl() = default;
};
ProgramExecution::~ProgramExecution() {
}
void ProgramExecution::operator ()() {
  ASSERT(m_impl, "no implementation");
  auto sizeChange = FileSizeChange{};
  auto outFileName = m_impl->makeOutFileName(m_inFileName);
  auto inFile = ExistingFile(m_inFileName);
  if (m_showStats) {
    sizeChange.originalSize = inFile.size();
  }
  auto outFile = GuardedNewFile(std::move(outFileName));

  auto in = std::istream{&inFile};
  in.exceptions(std::ios::failbit | std::ios::badbit);
  auto out = std::ostream{&outFile};
  out.exceptions(std::ios::failbit | std::ios::badbit);

  m_impl->doOperation(in, inFile.path().c_str(), out, outFile.path().c_str());

  outFile.doneWriting();
  if (!m_keepOriginal) {
    outFile.fsync();
    inFile.remove();
  }
  if (m_showStats) {
    sizeChange.finalSize = outFile.size();
    printStatistics(sizeChange);
  }
}
void ProgramExecution::printStatistics(const FileSizeChange& sizeChange) {
  const Statistics stats{sizeChange,
                         m_impl->spaceSaving(sizeChange)};
  Output::statistics(stats);
}

// === COMPRESS ===
class CompressImpl : public ProgramExecImpl {
public:
  CompressImpl(WordSize wordSize)
    : m_wordSize(wordSize) {
  }
  std::filesystem::path makeOutFileName(const std::filesystem::path& inFileName) override {
    if (inFileName.extension() == compressedFileExtension) {
      throw IOError("file already has %s extension: %s",
		    compressedFileExtension.c_str(), inFileName.c_str());
    }
    return std::filesystem::path(inFileName) += compressedFileExtension;
  }

  void doOperation(std::istream& inFile, const char* inFileName,
                   std::ostream& outFile, const char* /*outFileName*/) override {
    try {
      compress(inFile, outFile, m_wordSize);
    }
    catch (const StreamReadFailure& e) {
      throw FileReadError(inFileName, e.what());
    }
  }
  SpaceSaving spaceSaving(const FileSizeChange& sizeChange) override {
    return {sizeChange.finalSize, sizeChange.originalSize};
  }
private:
  WordSize m_wordSize;
};

// === DECOMPRESS ===
class DecompressImpl : public ProgramExecImpl {
public:
  std::filesystem::path makeOutFileName(const std::filesystem::path& inFileName) override {
    if (inFileName.extension() != compressedFileExtension) {
      throw IOError("unknown extension for file: %s", inFileName.c_str());
    }
    return std::filesystem::path(inFileName).replace_extension();
  }
  void doOperation(std::istream& inFile, const char* inFileName,
                   std::ostream& outFile, const char* /*outFileName*/) override {
    try {
      decompress(inFile, outFile);
    }
    catch (const StreamReadFailure& e) {
      throw FileReadError(inFileName, e.what());
    }
    catch(const InvalidCompressionHeader& e) {
      throw FileReadError(inFileName, e.what());
    }
  }
  SpaceSaving spaceSaving(const FileSizeChange& sizeChange) override {
    return {sizeChange.originalSize, sizeChange.finalSize};
  }
};
ProgramExecution::ProgramExecution(std::unique_ptr<ProgramExecImpl> impl, std::filesystem::path inFileName,
 bool showStats, bool keepOriginal)
 : m_impl(std::move(impl)), m_inFileName(std::move(inFileName)),
   m_showStats(showStats), m_keepOriginal(keepOriginal) {
}
ProgramExecution ProgramExecution::compress(const ProgramParameters::Compress& params) {
  return {std::make_unique<CompressImpl>(params.wordSize), params.inputFile,
              params.verbose, params.keep};
}
ProgramExecution ProgramExecution::decompress(const ProgramParameters::Decompress& params) {
  return {std::make_unique<DecompressImpl>(), params.inputFile,
              params.verbose, params.keep};
}
