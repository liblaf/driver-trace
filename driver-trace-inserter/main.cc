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
#define CC "clang-12"
#endif

int main(int argc, char** argv) {
  logging() << std::string(80, '-') << "\n";
  for (int i = 0; i < argc; ++i) logging() << argv[i] << " ";
  logging() << "\n";
  logging().flush();

  std::vector<std::string> args;
  std::string output;
  args.push_back(CC);
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      output = argv[++i];
      continue;
    }
    args.push_back(argv[i]);
  }

  // Execute(args, false);

  if ((std::any_of(args.begin(), args.end(),
                   [](const std::string& arg) {
                     static const std::vector<std::string> kActions = {
                         "-E",         "--preprocess",  "-S",
                         "--assemble", "-fsyntax-only", "--precompile"};
                     return Contains(kActions, arg);
                   }) == false) &&
      std::any_of(args.begin(), args.end(), IsSourceFile)) {
    args.push_back("--assemble");
    args.push_back("-emit-llvm");
    int ret = Execute(args);
    if (ret) exit(ret);
    for (int i = 0; i < 2; ++i) args.pop_back();
    for (int i = 0; i < args.size(); ++i) {
      auto ref = llvm::StringRef(args[i]);
      // if (ref.startswith("-Wp,")) {
      //   args.erase(args.begin() + (i--));
      //   continue;
      // }
      // if (args[i] == "-isystem" || args[i] == "-include") {
      //   args.erase(args.begin() + i, args.begin() + i + 2);
      //   --i;
      //   continue;
      // }
      // if (ref.startswith("-fmacro-prefix-map=")) {
      //   args.erase(args.begin() + (i--));
      //   continue;
      // }
      if (IsSourceFile(args[i])) {
        auto path = std::filesystem::path(args[i]);
        path = path.filename();
        path = path.replace_extension(".ll");
        Instrument(path, path);
        args[i] = path.string();
      }
    }
  }

  if (!output.empty()) {
    args.push_back("-o");
    args.push_back(output);
  }

  int return_value = Execute(args, /*use_vfork=*/false);

  logging() << std::string(80, '=') << "\n";
  logging() << "\n";
  logging().flush();
  return return_value;
}
