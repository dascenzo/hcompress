#include "Exception.h"
#include "Output.h"
#include <cstdlib>
#include <cstdarg>
#include <cstring>

SafeStringExcept::SafeStringExcept(const char* format, ...) {
  va_list vlist;
  va_start(vlist, format);
  auto n = std::vsnprintf(m_buffer.data(), maxMessageLength + 1, format, vlist); 
  va_end(vlist);
  ASSERT(!(n < 0), "invalid formatting");
  if (n > maxMessageLength) {
    std::strcpy(m_buffer.data() + maxMessageLength, truncationString);
  }
}
const char* SafeStringExcept::what() const noexcept {
  return m_buffer.data();
}

AssertionFailure::AssertionFailure(std::string_view message, const char* file,
                                   long line)
  : SafeStringExcept("%.*s - %s (ln %ld)", PRINTF_STRING_VIEW(message), file, line)
{
}
InvalidUsage::InvalidUsage()
  : SafeStringExcept("invalid usage")
{
}
InvalidOptionArg::InvalidOptionArg(const char* option, const char* arg, const char* why)
  : InvalidUsage("invalid argument to %s option: \"%s\" (%s)", option, arg, why)
{
}
InvalidCompressionHeader::InvalidCompressionHeader(const char* why)
  : SafeStringExcept(
        (why ? "invalid compression header: %s" : "invalid compression header"), why)
{
}
HuffmanTreeError::HuffmanTreeError(const char* why)
  : SafeStringExcept(
        (why ? "huffman tree error: %s" : "huffman tree error"), why)
{
}
FileError::FileError(const char* operation, const char* fileName, const char* why)
  : IOError((why
      ? "error occurred while trying to %s file \"%s\": %s"
      : "error occurred while trying to %s file \"%s\""),
      operation, fileName, why)
// XXX per C spec, excess args to fprintf and thus vsnprintf are ignored
{
}
FileReadError::FileReadError(const char* fileName, const char* why)
  : FileError("read", fileName, why)
{
}
FileWriteError::FileWriteError(const char* fileName, const char* why)
  : FileError("write", fileName, why)
{
}
FileOpenError::FileOpenError(const char* fileName, const char* why)
  : FileError("open", fileName, why)
{
}

StreamReadFailure::StreamReadFailure(const char* why)
  : IOError((why
      ? "error occurred while trying to %s stream: %s"
      : "error occurred while trying to %s stream"),
      "read", why)
{
}
//StreamWriteFailure::StreamWriteFailure(const char* why)
//  : StreamFailure("write", why)
//{
//}
