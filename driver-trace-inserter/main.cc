#include <unistd.h>

#include <algorithm>   // std::any_of()
#include <cstdlib>     // system()
#include <filesystem>  // std::filesystem
#include <fstream>     // std::fstream
#include <string>      // std::string
#include <vector>      // std::vector

#include "instrument.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "utils.h"

#ifndef CC
#define CC "clang-13"
#endif

static std::string log_path;

bool GetEnv();

int main(int argc, char** argv) {
  GetEnv();

  if (log_path.empty() == false) freopen(log_path.c_str(), "a", stderr);
  llvm::errs() << std::string(80, '-') << "\n";
  for (int i = 0; i < argc; ++i) llvm::errs() << argv[i] << " ";
  llvm::errs() << "\n";
  llvm::errs().flush();

  std::vector<std::string> new_argv;
  std::string output;
  new_argv.push_back(CC);
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      output = argv[++i];
      continue;
    }
    new_argv.push_back(argv[i]);
  }

  if (std::any_of(new_argv.begin(), new_argv.end(), IsSourceFile)) {
    new_argv.push_back("-S");
    new_argv.push_back("-emit-llvm");
    int ret = Execute(new_argv);
    if (ret) exit(ret);
    new_argv.pop_back();
    new_argv.pop_back();
    for (std::string& arg : new_argv) {
      if (IsSourceFile(arg)) {
        auto path = std::filesystem::path(arg);
        path = path.filename();
        path = path.replace_extension(".ll");
        Instrument(path, path);
        llvm::dbgs() << "ready to change params"
                     << "\n";
        llvm::dbgs().flush();
        arg = path.string();
      }
    }
    llvm::dbgs() << new_argv.back() << "\n";
    llvm::dbgs().flush();
    new_argv.push_back(
        "/home/liblaf/Desktop/driver-trace/driver-trace-recorder/"
        "driver-trace-recorder.ko");
  }
  llvm::dbgs() << new_argv.back() << "\n";
  llvm::dbgs().flush();
  if (!output.empty()) {
    new_argv.push_back("-o");
    new_argv.push_back(output);
  }
  Execute(new_argv, /*shell=*/false);
  llvm::dbgs() << std::string(80, '=') << "\n";
  llvm::dbgs() << "\n";
  llvm::dbgs().flush();
  return 0;
}

bool GetEnv() {
  char* ptr;
  ptr = getenv("LOG_PATH");
  log_path = (ptr == NULL) ? "/tmp/driver-trace-inserter.log" : ptr;
  return true;
}
