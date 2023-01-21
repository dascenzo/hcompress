#pragma once
#include <stdexcept>
#include <array>
#include <string>
#include <string_view>

/**
 * Stores a string message that is truncated if too long.
 * Wont dynamically allocate memory.
 */
class SafeStringExcept : public std::exception {
private:
  constexpr static char truncationString[] = "...";
public:
  SafeStringExcept(const char* format, ...);
  const char* what() const noexcept override;

  constexpr static std::size_t maxMessageLength = 150;
  constexpr static std::size_t maxStringLength =
      maxMessageLength + std::char_traits<char>::length(truncationString);
protected:
  ~SafeStringExcept() = default;
private:
  std::array<char, maxStringLength + 1 /*null terminator*/> m_buffer;
};
/**
 * Command line syntax was incorrect.
 */
class InvalidUsage : public SafeStringExcept {
  using SafeStringExcept::SafeStringExcept;
public:
  InvalidUsage();
};
/**
 * A command line option that takes an argument was given an invalid one.
 */
class InvalidOptionArg : public InvalidUsage {
public:
  InvalidOptionArg(const char* option, const char* arg, const char* why);
};

class IOError : public SafeStringExcept {
  using SafeStringExcept::SafeStringExcept;
};

class FileError : public IOError {
public:
  FileError(const char* operation, const char* fileName, const char* why = nullptr);
};

class FileReadError : public FileError {
public:
  FileReadError(const char* fileName, const char* why = nullptr);
};
class FileWriteError : public FileError {
public:
  FileWriteError(const char* fileName, const char* why = nullptr);
};
class FileOpenError : public FileError {
public:
  FileOpenError(const char* fileName, const char* why = nullptr);
};

class InvalidCompressionHeader : public SafeStringExcept {
public:
  InvalidCompressionHeader(const char* why = nullptr);
};
class HuffmanTreeError : public SafeStringExcept {
public:
  HuffmanTreeError(const char* why = nullptr);
};

class StreamReadFailure : public IOError {
public:
  StreamReadFailure(const char* why = nullptr);
};

class AssertionFailure : public SafeStringExcept {
public:
  AssertionFailure(std::string_view message, const char* file, long line);
};

#define ASSERT(b, message) do { \
    if (!(b)) throw AssertionFailure(message, __FILE__, __LINE__); \
  } while (false)
