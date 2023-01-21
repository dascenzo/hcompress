#include "ExistingFile.h"
#include "Exception.h"
#include "CMessage.h"
#include "Output.h"
#include <utility>
#include <unistd.h>

ExistingFile::ExistingFile(std::filesystem::path file)
  : m_filePath(std::move(file)),
    m_filePtr(initFilePtr(m_filePath))
{
}
ExistingFile::~ExistingFile() {
  if (m_filePtr) {
    if (std::fclose(m_filePtr) == EOF) {
      Output::error("couldn't close file", std::strerror(errno));
    }
  }
}
FILE* ExistingFile::initFilePtr(const std::filesystem::path& filePath) {
  FILE* filePtr = std::fopen(filePath.c_str(), "rb");
  if (!filePtr) {
    throw FileOpenError(filePath.c_str(), std::strerror(errno));
  }
  if (!is_regular_file(filePath)) {
    auto message = CMessage("not a regular file");
    if (std::fclose(filePtr) == EOF) {
      const auto err = errno;
      message << "; " << "couldn't close file"
              << ": " << std::strerror(err);
    }
    throw FileOpenError(filePath.c_str(), message.c_str());
  }
  return filePtr;
}
void ExistingFile::remove() {
  if (m_filePtr) {
    FILE* file = m_filePtr;
    m_filePtr = nullptr;
    if (std::fclose(file) == EOF) {
      int fcloseError = errno, unlinkError = 0;
      if (unlink(m_filePath.c_str()) != 0) {
        unlinkError = errno;
      }
      else {
        throw FileError("delete", m_filePath.c_str(), std::strerror(fcloseError));
      }
      const auto message = CMessage(std::strerror(fcloseError))
                               << "; " << std::strerror(unlinkError);
      throw FileError("delete", m_filePath.c_str(), message.c_str());
    }
    if (unlink(m_filePath.c_str()) != 0) {
      throw FileError("delete", m_filePath.c_str(), std::strerror(errno));
    }
  }
  else {
    throw FileError("delete", m_filePath.c_str(), "file already deleted");
  }
}
ExistingFile::int_type ExistingFile::underflow() {
  if (m_filePtr == nullptr) {
    setg(nullptr, nullptr, nullptr);
    ASSERT(false, CMessage("tried to read removed file: ") << m_filePath.c_str());
  }
  const auto read = std::fread(m_buf.data(), 1, BUFSIZ, m_filePtr);
  if (read < BUFSIZ) {
    if (std::ferror(m_filePtr)) {
      setg(nullptr, nullptr, nullptr);
      throw FileReadError(m_filePath.c_str());
    }
    if (read == 0) {
      setg(nullptr, nullptr, nullptr);
      return traits_type::eof();
    }
  }
  setg(m_buf.data(), m_buf.data(), m_buf.data() + read);
  return traits_type::to_int_type(m_buf.front());
}
ExistingFile::pos_type ExistingFile::seekpos(pos_type pos, std::ios_base::openmode which) {
  ASSERT(!(which & std::ios_base::out),
      CMessage("tried to seek on output stream: ") << m_filePath.c_str());
  ASSERT(m_filePtr,
      CMessage("tried to read removed file: ") << m_filePath.c_str());
  ASSERT(pos <= LONG_MAX /* fseek offset type */,
      CMessage("tried to seek with invalid offset: ")
          << static_cast<std::streamoff>(pos));

  if (std::fseek(m_filePtr, pos, SEEK_SET) != 0) {
    throw FileError("seek", m_filePath.c_str());
    //return pos_type(off_type(-1));
  }
  setg(nullptr, nullptr, nullptr);
  return pos;
}
