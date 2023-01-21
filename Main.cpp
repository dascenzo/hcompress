#include "ProgramParameters.h"
#include "ProgramExecution.h"
#include "Exception.h"
#include "Output.h"
#include <variant>
#include <iostream>

struct MainProgram {
  void operator()(const ProgramParameters::Compress& params) {
    ProgramExecution::compress(params)();
  }
  void operator()(const ProgramParameters::Decompress& params) {
    ProgramExecution::decompress(params)();
  }
  void operator()(const ProgramParameters::ShowHelp&) noexcept {
    Output::usage(std::cout);
  }
};
int main(int argc, char* argv[]) {
  Output::setProgramName(argv[0]);
  try {
    const auto params = ProgramParameters{argc, argv};
    std::visit(MainProgram{}, params.variant());
    return 0;
  }
  catch (const InvalidOptionArg& e) {
    Output::error(e.what());
  }
  catch (const InvalidUsage& e) {
    Output::usage(std::cerr);
  }
  catch (const IOError& e) {
    Output::error(e.what());
  }
  catch (const std::exception& e) {
    Output::error("internal error", e.what());
  }
  catch (...) {
    Output::error("unknown internal error");
  }
  return EXIT_FAILURE;
}
