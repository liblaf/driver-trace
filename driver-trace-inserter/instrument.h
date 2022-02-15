#ifndef INSTRUMENT_H_
#define INSTRUMENT_H_

#include <filesystem>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"

bool Instrument(const std::filesystem::path& input,
                const std::filesystem::path& output);
void RunOnModule(llvm::Module& mod);
void RunOnFunction(llvm::Function& func);
void RunOnBasicBlock(llvm::BasicBlock& block);
void RunOnInstruction(llvm::Instruction& inst);

llvm::FunctionCallee InsertOnInitDeclaration(llvm::Module& mod);
llvm::FunctionCallee InsertOnCleanupDeclaration(llvm::Module& mod);
llvm::FunctionCallee InsertPassingDeclaration(llvm::Module& mod);

#endif  // INSTRUMENT_H_
