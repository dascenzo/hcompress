#include "GuardedNewFile.h"
#include "Exception.h"
#include "Output.h"
#include <utility>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

// TODO: open() file with O_TMPFILE flag, then linkat() in doneWriting().
// This is so destructor can't fail: the file wont exist
// until doneWriting() is called, and then there's nothing to do in the destructor:
//   doneWriting() called => leave file in place
//   doneWriting() !called => file doesn't exist
// MacOS seems not to support even: open(), unlink(), linkat(/dev/fd/<my fd>)
// so we have a destructor that may fail (when trying to remove the file)

GuardedNewFile::GuardedNewFile(std::filesystem::path file)
  : m_filePath{std::move(file)},
    m_doneWriting{false}
{
  m_fd = open(m_filePath.c_str(), O_WRONLY | O_CREAT | O_EXCL,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (m_fd < 0) {
    throw FileOpenError(m_filePath.c_str(), std::strerror(errno));
  }
}
// what else can we do but print out errors
GuardedNewFile::~GuardedNewFile() {
  if (close(m_fd) != 0) {
    Output::error("couldn't close file", std::strerror(errno));
  }
  if (!m_doneWriting) {
    if (unlink(m_filePath.c_str()) != 0) {
      Output::error("couldn't delete file", std::strerror(errno));
    }
  }
}
GuardedNewFile::int_type GuardedNewFile::overflow(int_type ch) {
  ASSERT(!m_doneWriting, getAlreadyDoneError());
  if (traits_type::eq_int_type(ch, traits_type::eof())) {
    return traits_type::not_eof(ch);
  }
  const char_type c = traits_type::to_char_type(ch);
  if (write(m_fd, &c, 1) == 1) {
    return ch;
  }
  throw FileWriteError(m_filePath.c_str(), std::strerror(errno));
}
std::streamsize GuardedNewFile::xsputn(const char_type* s, std::streamsize count) {
  ASSERT(!m_doneWriting, getAlreadyDoneError());
  std::streamsize ret = 0;
  if (ret < count) {
    const auto written = write(m_fd, s, count);
    if (written != -1) {
      ret = written;
    }
    else {
      throw FileWriteError(m_filePath.c_str(), std::strerror(errno));
    }
  }
  return ret;
}
int GuardedNewFile::sync() {
  fsync();
  return 0;
}
void GuardedNewFile::doneWriting() {
  m_doneWriting = true;
}
void GuardedNewFile::fsync() {
#ifdef __APPLE__
  const auto ret = fcntl(m_fd, F_FULLFSYNC);
  if (ret == -1) {
    throw FileError("disk sync", m_filePath.c_str(), std::strerror(errno));
  }
#else
  #error "file sync unimplemented for this system"
#endif
}
CMessage GuardedNewFile::getAlreadyDoneError() const {
  return CMessage("can't write to ")
	   << m_filePath.c_str()
	   << ": already marked as done for writing";
}
