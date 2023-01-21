#pragma once
#include <unordered_map>
#include <string>
#include "Definitions.h"
#include <istream>

class FrequencyData {
public:
  FrequencyData(std::istream& in, WordSize wordSize);
  FrequencyData(FrequencyData&&);
  FrequencyData& operator=(FrequencyData&&);
  auto begin() const { return m_wordToFrequency.begin(); }
  auto end() const { return m_wordToFrequency.end(); }
  auto uniqueWords() const { return m_wordToFrequency.size(); }
  auto totalWords() const { return m_totalWords; }
  auto wordSize() const { return m_wordSize; }
  const  auto& noWord() const { return m_noWord; }
  friend std::istream& operator>>(std::istream&, FrequencyData&);
private:
  explicit FrequencyData(WordSize wordSize);
  std::unordered_map<Word, WordCount, Word::Hash> m_wordToFrequency;
  WordCount m_totalWords;
  WordSize m_wordSize;
  NoWord m_noWord;
};
