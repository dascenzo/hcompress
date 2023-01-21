#include "FrequencyData.h"
#include "Exception.h"
#include "SafeMath.h"

FrequencyData::FrequencyData(WordSize wordSize)
  : m_totalWords(0), m_wordSize(wordSize) {
  ASSERT(m_wordSize > 0, "invalid word size");
}
FrequencyData::FrequencyData(std::istream& in, WordSize wordSize)
  : FrequencyData(wordSize) {
  in >> *this;
}
FrequencyData::FrequencyData(FrequencyData&& other)
  : m_wordToFrequency(std::move(other.m_wordToFrequency)),
    m_totalWords(other.m_totalWords),
    m_wordSize(other.m_wordSize),
    m_noWord(std::move(other.m_noWord))
{
  other.m_wordToFrequency.clear();
  other.m_totalWords = 0;
  other.m_noWord = NoWord();
}
FrequencyData& FrequencyData::operator=(FrequencyData&& other)
{
  m_wordToFrequency = std::move(other.m_wordToFrequency);
  m_totalWords = other.m_totalWords;
  m_wordSize = other.m_wordSize;
  m_noWord = std::move(other.m_noWord);

  other.m_wordToFrequency.clear();
  other.m_totalWords = 0;
  other.m_noWord = NoWord();
  return *this;
}
/**
 * Emptys in. Works the same as code like (i.e. read till encountering EOF):
 * stringstream ss("a"); string s; ss >> s;
 * An exception is thrown if:
 * - the stream fails or it was eof initially.
 *     rationale: in this case the function did not read all of in,
                  so the frequency data would be incorrect.
 * - there's too much data in the stream ( >= 2^64 bytes (18 exabytes)).
 * If an exception is thrown, data is unchanged.
 */
std::istream& operator>>(std::istream& in, FrequencyData& data) {
  auto table = decltype(data.m_wordToFrequency)();
  auto totalWords = WordCount(0);
  auto buf = std::string();
  char c;
  while (true) {
    if (in.peek() == EOF) {
      break;
    }
    in.get(c);
    buf.push_back(c);
    if (buf.size() == data.m_wordSize) {
      if (!safeAddAssign(totalWords, 1u)) {
        throw IOError("too much input data");
      }
      ++table[Word(std::move(buf))];
      buf.clear();
    }
  }
  data.m_wordToFrequency = std::move(table);
  data.m_totalWords = totalWords;
  data.m_noWord = NoWord(std::move(buf));
  return in;
}
