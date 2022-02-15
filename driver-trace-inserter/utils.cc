#include "utils.h"

#include <unistd.h>

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

std::string Join(const std::string& str,
                 const std::vector<std::string>& string_list) {
  if (string_list.empty()) return "";
  std::string res = string_list.front();
  for (int i = 1; i < string_list.size(); ++i) res += str + string_list[i];
  return res;
}

int Execute(const std::vector<std::string>& argv, bool shell) {
  std::string command = Join(" ", argv);
  llvm::dbgs() << command << "\n";
  if (shell == false) {
    llvm::dbgs() << std::string(80, '=') << "\n";
    llvm::dbgs() << "\n";
  }
  llvm::dbgs().flush();
  if (shell) return system(command.c_str());
  char** new_argv = new char*[argv.size() + 1];
  for (int i = 0; i < argv.size(); ++i) {
    new_argv[i] = new char[argv[i].length() + 1];
    strcpy(new_argv[i], argv[i].c_str());
  }
  new_argv[argv.size()] = nullptr;
  int ret = execvp(new_argv[0], new_argv);
  exit(ret);
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
