#ifndef DRIVER_TRACE_INSERTER_INSTRUMENT_H_
#define DRIVER_TRACE_INSERTER_INSTRUMENT_H_

#include <filesystem>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

bool Instrument(const std::filesystem::path& input,
                const std::filesystem::path& output);
void RunOnModule(llvm::Module& module);
void RunOnFunction(llvm::Function& func);

void InsertFuncDeclaration(llvm::Module& module);
void InsertPassingFunction(llvm::Module& module);
void InsertRecordParameter(llvm::Module& module);
void InsertFinishingFunction(llvm::Module& module);

#endif  // DRIVER_TRACE_INSERTER_INSTRUMENT_H_
