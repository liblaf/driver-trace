#ifndef DRIVER_TRACE_INSERTER_UTILS_H_
#define DRIVER_TRACE_INSERTER_UTILS_H_

#include <filesystem>
#include <string>
#include <vector>

std::string Join(const std::string& str,
                 const std::vector<std::string>& string_list);
int Execute(const std::vector<std::string>& argv, bool shell = true);
bool IsSourceFile(const std::filesystem::path& path);
bool IsIRFile(const std::filesystem::path& path);

#endif  // DRIVER_TRACE_INSERTER_UTILS_H_
