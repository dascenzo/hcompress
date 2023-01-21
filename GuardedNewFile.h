#pragma once
#include "CMessage.h"
#include <filesystem>
#include <cstdio>
#include <memory>
#include <streambuf>

/**
 * Creates a new file (that is, the given path cannot
 * already exist) and requires writing to the file to
 * be marked as completed: If the destructor runs
 * before doneWriting() is called (e.g. if an exception
 * is thrown) the file will be automatically deleted.
 */
class GuardedNewFile : public std::streambuf {
public:
  explicit GuardedNewFile(std::filesystem::path file);
  ~GuardedNewFile();
  GuardedNewFile(GuardedNewFile&) = delete;
  GuardedNewFile& operator =(GuardedNewFile&) = delete;

  void fsync();
  void doneWriting();
  auto size() const
    { return file_size(m_filePath); }
  const std::filesystem::path& path() const noexcept
    { return m_filePath; }

protected:
  int_type overflow(int_type ch) override;
  std::streamsize xsputn(const char_type* s, std::streamsize count) override;
  int sync() override;

private:
  CMessage getAlreadyDoneError() const;

  std::filesystem::path m_filePath;
  bool m_doneWriting;
  int m_fd;
};
