#include "ProgramParameters.h"
#include "Exception.h"
#include "SafeMath.h"
#include <cctype>
#include <limits>
#include <type_traits>
#include <climits>
#include <memory>
#include <span>

const char* const ProgramParameters::usage =
    "[-dkhv] [-b num] [file]\n"
    "  -d     Decompress file, else compress.\n"
    "  -b num Specify symbol length for encoding (during compression) to be num bytes. 1 by default.\n"
    "  -k     Keep input file. Normally input file is deleted.\n"
    "  -v     Verbose mode. Print statistics.\n"
    "  -h     Show this help text.";

static_assert(CHAR_BIT == 8,
  "A byte must be a fixed size so files compressed on one system can be decompressed on another.");

/**
 * Terminology:
 * $ programName -a -b -c 123 -def hello world
 *               \    switches   / \ arguments /
 */

enum class SwitchType { Flag /** e.g. -a */ , Arg /** e.g. -b <something> */};

/**
 * Interface to set a variable when a command line switch is found
 */
class SwitchSetter {
public:
  virtual void set(int index, const char* const* argv) = 0;
  virtual ~SwitchSetter() = default;
};
/**
 * Sets a boolean flag to true.
 */
class FlagSetter : public SwitchSetter {
public:
  FlagSetter(bool& flag) : m_flag(flag)
  {}
  void set(int, const char* const*) override { m_flag = true; }
private:
  bool& m_flag;
};
/**
 * Parses and sets the word size value passed in '-b <value>'.
 */
class WordSizeSetter : public SwitchSetter {
public:
  WordSizeSetter(WordSize& wordSize) : m_wordSize(wordSize)
  {}
  void set(int index, const char* const* argv) override {
    static_assert(std::is_unsigned<WordSize>::value);
    const char* arg = argv[index + 1];
    ASSERT(arg != nullptr, "Argument shouldn't be null.");
    if (*arg == '\0') {
      throw InvalidOptionArg("-b", arg, "empty");
    }
    auto total = WordSize{0};
    for (auto cp = arg; *cp; ++cp) {
      if (!std::isdigit(static_cast<unsigned char>(*cp))) {
        throw InvalidOptionArg("-b", arg, "contains non-digits");
      }
      // total = total * 10 + (arg[i] - '0')
      if (total > std::numeric_limits<WordSize>::max() / 10) {
        throw InvalidOptionArg("-b", arg, "too big");
      }
      total *= 10;

      const auto digit = static_cast<WordSize>(*cp - '0');

      if (!safeAddAssign(total, digit)) {
        throw InvalidOptionArg("-b", arg, "too big");
      }
    }
    if (total == 0) {
      throw InvalidOptionArg("-b", arg, "can't be 0");
    }
    m_wordSize = total;
  }
private:
  WordSize& m_wordSize;
};
struct SwitchSpec {
  char ch;
  SwitchType switchType;
  std::unique_ptr<SwitchSetter> setter;
  bool found = false;
};
template<class SwitchIt, class ValidIt>
static bool validateSwitches(SwitchIt sw_first, SwitchIt sw_last,
                             ValidIt  v_first, ValidIt v_last) {
  return std::any_of(v_first,  v_last, [&](const char* valid)
    {
      return std::all_of(sw_first, sw_last, [&](const SwitchSpec& sw)
        {
          return !sw.found || std::strchr(valid, sw.ch) != nullptr;
        });
    });
}
template<class ArgIt>
static bool validateArguments(ArgIt first, ArgIt last) {
  const auto size = last - first;
  return size == 0 || (size == 1 && std::strlen(*first) != 0);
}
ProgramParameters::ProgramParameters(int argc, char* argv[]) {
  bool decompress = false,  keep = false, verbose = false, help = false;
  WordSize wordSize = 1;
  SwitchSpec switches[] = {
    {'d', SwitchType::Flag, std::make_unique<FlagSetter>(decompress) },
    {'k', SwitchType::Flag, std::make_unique<FlagSetter>(keep)       },
    {'v', SwitchType::Flag, std::make_unique<FlagSetter>(verbose)    },
    {'h', SwitchType::Flag, std::make_unique<FlagSetter>(help)       },
    {'b', SwitchType::Arg,  std::make_unique<WordSizeSetter>(wordSize)}
  };
  const char* validSwitches[] = {
    "dkv", "kvb", "h"
  };
  int i = 1;
  for ( ; i < argc; ++i) {
    if (std::strlen(argv[i]) == 0) {
      throw InvalidUsage();
    }
    if (std::strcmp(argv[i], "-") == 0) {
      throw InvalidUsage();
    }
    if (std::strcmp(argv[i], "--") == 0) {
      ++i;
      break;
    }
    if (argv[i][0] != '-') {
      break;
    }
    // parse switch group
    for (const char* cp = argv[i] + 1; *cp; ++cp) {
      auto it = std::find_if(std::begin(switches), std::end(switches), [ch = *cp](const SwitchSpec& sw)
        {
          return sw.ch == ch && !sw.found;
        });
      if (it != std::end(switches)) {
        it->found = true;
        const auto idx = i;
        if (it->switchType == SwitchType::Arg && (*(cp + 1) != '\0' || ++i == argc)) {
          throw InvalidUsage();
        }
        it->setter->set(idx, argv);
      }
      else {
        throw InvalidUsage();
      }
    }
  }
  if (!validateSwitches(std::begin(switches), std::end(switches),
                        std::begin(validSwitches), std::end(validSwitches))) {
    throw InvalidUsage();
  }
  const auto arguments = std::span(argv + i, argv + argc);
  if (!validateArguments(std::begin(arguments), std::end(arguments))) {
    throw InvalidUsage();
  }
  // argument/switch compatibility
  if (arguments.empty() != help) {
    throw InvalidUsage();
  }
  // create variant
  if (help) {
    m_variant = ShowHelp();
  }
  else if (decompress) {
    auto decom = Decompress();
    decom.inputFile = arguments[0];
    decom.keep = keep;
    decom.verbose = verbose;
    m_variant = decom;
  }
  else {
    auto com = Compress();
    com.inputFile = arguments[0];
    com.keep = keep;
    com.verbose = verbose;
    com.wordSize = wordSize;
    m_variant = com;
  }
}
