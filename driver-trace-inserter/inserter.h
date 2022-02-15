#ifndef INSERTER_H_
#define INSERTER_H_

#include <filesystem>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"  // CallInst
#include "llvm/IR/Module.h"

class Inserter {
 public:
  bool Instrument(const std::filesystem::path& input,
                  const std::filesystem::path& output);

 protected:
  void RunOnModule(llvm::Module& mod);
  void RunOnFunction(llvm::Function& func);
  void RunOnInit(llvm::Function& func);
  void RunOnCleanup(llvm::Function& func);
  void RunOnBasicBlock(llvm::BasicBlock& block, llvm::Constant* caller_name);
  void RunOnInst(llvm::Instruction& inst, llvm::Constant* caller_name);
  void RunOnCallInst(llvm::CallInst& inst, llvm::Constant* caller_name);

 protected:
  static llvm::FunctionCallee InsertOnInitDeclaration(llvm::Module& mod);
  static llvm::FunctionCallee InsertOnCleanupDeclaration(llvm::Module& mod);
  static llvm::FunctionCallee InsertPassingDeclaration(llvm::Module& mod);

 private:
  llvm::FunctionCallee on_init_, on_cleanup_, passing_;
};

#endif  // INSERTER_H_
