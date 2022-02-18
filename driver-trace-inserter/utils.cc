#include "utils.h"

#include <unistd.h>

#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

llvm::raw_ostream& logging() {
  char* log_path = getenv("DRIVER_TRACE_LOG_PATH");
  if (!log_path) log_path = "/tmp/driver-trace-inserter.log";
  std::error_code ec;
  static auto log_stream =
      llvm::raw_fd_ostream(log_path, ec, llvm::sys::fs::OF_Append);
  return log_stream;
}

int Execute(const std::vector<std::string>& args, bool use_vfork) {
  for (auto&& arg : args) logging() << arg << " ";
  logging() << "\n";
  sync();
  int return_value = 0;
  if (use_vfork) {
    std::string command;
    for (auto&& arg : args) command += arg + " ";
    return_value = system(command.c_str());
  } else {
    char** argv = new char*[args.size() + 1];
    for (int i = 0; i < args.size(); ++i)
      argv[i] = const_cast<char*>(args[i].c_str());
    argv[args.size()] = NULL;
    logging() << std::string(80, '=') << "\n";
    logging().flush();
    return_value = execvp(argv[0], argv);
    delete[] argv;
  }
  sync();
  return return_value;
}

bool IsSourceFile(const std::filesystem::path& path) {
  if (std::filesystem::exists(path) == false) return false;
  if (path.extension() != ".c") return false;
  std::filesystem::path stem = path.stem();
  if (stem.extension() == ".mod") return false;
  return true;
}

bool IsIRFile(const std::filesystem::path& path) {
  if (std::filesystem::exists(path) == false) return false;
  if (path.extension() != ".ll") return false;
  std::filesystem::path stem = path.stem();
  if (stem.extension() == ".mod") return false;
  return true;
}
