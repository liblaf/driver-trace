#ifndef DRIVER_TRACE_INSERTER_UTILS_H_
#define DRIVER_TRACE_INSERTER_UTILS_H_

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "llvm/Support/raw_ostream.h"

llvm::raw_ostream& logging();
int Execute(const std::vector<std::string>& argv, bool use_vfork = true);
bool IsSourceFile(const std::filesystem::path& path);
bool IsIRFile(const std::filesystem::path& path);
template <class T>
bool Contains(const std::vector<T>& vec, const T& val) {
  return std::any_of(vec.begin(), vec.end(),
                     [val](const T& elem) { return elem == val; });
}

#endif  // DRIVER_TRACE_INSERTER_UTILS_H_
