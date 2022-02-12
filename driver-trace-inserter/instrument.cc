#include "instrument.h"

#include "llvm/IR/Type.h"              // Type, PointerType
#include "llvm/IR/Verifier.h"          // verifyModule()
#include "llvm/IRReader/IRReader.h"    // parseIRFile()
#include "llvm/Support/FileSystem.h"   // sys::fs::
#include "llvm/Support/SourceMgr.h"    // SMDiagnostic
#include "llvm/Support/raw_ostream.h"  // raw_fd_ostream

bool Instrument(const std::filesystem::path& input,
                const std::filesystem::path& output) {
  using namespace llvm;
  LLVMContext context;
  SMDiagnostic err;
  auto Owner = llvm::parseIRFile(input.string(), err, context);
  if (!Owner) {
    llvm::errs() << "ParseIRFile failed\n" << err.getMessage() << "\n";
    return false;
  }

  RunOnModule(*Owner);

  if (llvm::verifyModule(*Owner, &llvm::errs())) {
    llvm::errs() << "Generated module is not correct!\n";
    return false;
  }
  std::error_code ec;
  llvm::raw_fd_ostream out(output.string(), ec, sys::fs::OF_None);
  Owner->print(out, nullptr);
  return true;
}

void RunOnModule(llvm::Module& module) {
  InjectFuncDeclaration(module);
  for (auto& func : module) RunOnFunction(func);
}

void RunOnFunction(llvm::Function& func) {
  if (func.isDeclaration()) return;
}

void InsertFuncDeclaration(llvm::Module& module) {
  using namespace llvm;
  InsertPassingFunction(module);
  InsertRecordParameter(module);
  InsertFinishingFunction(module);
}

void InsertPassingFunction(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTracePassing(const char* function_name,
  //                           const char* caller_name, const char* type,
  //                           const void* return_value);
  Type* void_ty = Type::getVoidTy(context);
  PointerType* char_ptr_ty = PointerType::getInt8PtrTy(context);
  PointerType* void_ptr_ty = PointerType::getInt8PtrTy(context);
  FunctionType* func_ty = FunctionType::get(
      void_ty, {char_ptr_ty, char_ptr_ty, char_ptr_ty, void_ptr_ty},
      /*isVarArg=*/false);
  FunctionCallee callee =
      module.getOrInsertFunction("__DriverTracePassing", func_ty);

  // add param attr
  Function* func = dyn_cast<Function>(callee.getCallee());
  func->addParamAttr(0, Attribute::NoCapture);
  func->addParamAttr(0, Attribute::ReadOnly);
  func->addParamAttr(1, Attribute::NoCapture);
  func->addParamAttr(1, Attribute::ReadOnly);
  func->addParamAttr(2, Attribute::NoCapture);
  func->addParamAttr(2, Attribute::ReadOnly);
  func->addParamAttr(3, Attribute::NoCapture);
  func->addParamAttr(3, Attribute::ReadOnly);
}

void InsertRecordParameter(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceRecordParameter(const char* type, const void* value);
  Type* void_ty = Type::getVoidTy(context);
  PointerType* char_ptr_ty = PointerType::getInt8PtrTy(context);
  PointerType* void_ptr_ty = PointerType::getInt8PtrTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, {char_ptr_ty, void_ptr_ty},
                                            /*isVarArg=*/false);
  FunctionCallee callee =
      module.getOrInsertFunction("__DriverTraceRecordParameter", func_ty);

  // add param attr
  Function* func = dyn_cast<Function>(callee.getCallee());
  func->addParamAttr(0, Attribute::NoCapture);
  func->addParamAttr(0, Attribute::ReadOnly);
  func->addParamAttr(1, Attribute::NoCapture);
  func->addParamAttr(1, Attribute::ReadOnly);
}

void InsertFinishingFunction(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceFinishing();
  Type* void_ty = Type::getVoidTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, /*isVarArg=*/false);
  FunctionCallee callee =
      module.getOrInsertFunction("__DriverTraceFinishing", func_ty);
}
