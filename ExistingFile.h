#pragma once
#include <filesystem>
#include <cstdio>
#include <streambuf>
#include <array>

/**
 * Opens a stream to a file that must already exist.
 */
class ExistingFile : public std::streambuf {
public:
  explicit ExistingFile(std::filesystem::path file);
  ~ExistingFile();
  ExistingFile(const ExistingFile&) = delete;
  ExistingFile& operator =(const ExistingFile&) = delete;

  void remove();
  auto size() const
    { return file_size(m_filePath); }
  const std::filesystem::path& path() const noexcept
    { return m_filePath; }
protected:
  int_type underflow() override;
  pos_type seekpos(pos_type pos,
                   std::ios_base::openmode which = std::ios_base::in) override;
private:
  static FILE* initFilePtr(const std::filesystem::path&);

  std::filesystem::path m_filePath;
  FILE* m_filePtr;
  std::array<char, BUFSIZ> m_buf;
};
