#include "instrument.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"        // verifyModule()
#include "llvm/IRReader/IRReader.h"  // parseIRFile()
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"  // llvm::sys::fs::
#include "llvm/Support/SourceMgr.h"   // SMDiagnostic

bool Instrument(const std::filesystem::path& input,
                const std::filesystem::path& output) {
  using namespace llvm;
  LLVMContext context;
  SMDiagnostic err;
  auto owner = parseIRFile(input.string(), err, context).release();
  if (!(owner)) {
    errs() << "parseIRFile(): " << err.getMessage() << "\n";
    return false;
  }

  RunOnModule(*owner);

  if (verifyModule(*(owner), &errs())) {
    errs() << "Generated module is incorrect!"
           << "\n";
    return false;
  }
  dbgs() << "module correct"
         << "\n";
  dbgs().flush();
  std::error_code err_code;
  auto out = raw_fd_ostream(output.string(), err_code, sys::fs::OF_None);
  owner->print(out, nullptr);
  dbgs() << "write success"
         << "\n";
  dbgs().flush();
  return true;
}

void RunOnModule(llvm::Module& mod) {
  using namespace llvm;
  InsertOnInitDeclaration(mod);
  InsertOnCleanupDeclaration(mod);
  InsertPassingDeclaration(mod);
  for (Function& func : mod) RunOnFunction(func);
}

void RunOnFunction(llvm::Function& func) {
  using namespace llvm;
  if (func.isDeclaration()) return;
  for (BasicBlock& block : func) RunOnBasicBlock(block);
}

void RunOnBasicBlock(llvm::BasicBlock& block) {
  using namespace llvm;
  for (auto iter = block.begin(); iter != block.end(); ++iter) {
    dbgs() << *iter << "\n";
    RunOnInstruction(*iter);
  }
}

void RunOnInstruction(llvm::Instruction& inst) {
  using namespace llvm;
  if (!isa<CallInst>(inst)) return;
  auto& call_inst = *dyn_cast<CallInst>(&inst);
  Function* called_func = call_inst.getCalledFunction();
  if (called_func->getName().startswith("__DriverTrace")) return;
  auto builder = IRBuilder<>(&call_inst);
  Function* caller = call_inst.getFunction();
  Constant* caller_name = builder.CreateGlobalStringPtr(caller->getName());
  Constant* func_name = builder.CreateGlobalStringPtr(called_func->getName());
  LLVMContext& context = inst.getContext();
  IntegerType* int_ty = IntegerType::getInt32Ty(context);
  ConstantInt* num_of_params = ConstantInt::get(int_ty, 0);
  Module& mod = *(inst.getModule());
  FunctionCallee passing_callee = InsertPassingDeclaration(mod);
  CallInst* call_passing = builder.CreateCall(
      passing_callee, {func_name, caller_name, num_of_params});
  call_passing->insertAfter(&inst);
}

llvm::FunctionCallee InsertOnInitDeclaration(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceOnInit(void);
  Type* void_ty = Type::getVoidTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, /*isVarArg=*/false);
  return module.getOrInsertFunction("__DriverTraceOnInit", func_ty);
}

llvm::FunctionCallee InsertOnCleanupDeclaration(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceOnCleanup(void);
  Type* void_ty = Type::getVoidTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, /*isVarArg=*/false);
  return module.getOrInsertFunction("__DriverTraceOnCleanup", func_ty);
}

llvm::FunctionCallee InsertPassingDeclaration(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTracePassing(const char* func_name, const char* caller_name,
  //                           int num_of_params, ...);
  Type* void_ty = Type::getVoidTy(context);
  PointerType* char_ptr_ty = Type::getInt8PtrTy(context);
  IntegerType* int_ty = Type::getInt32Ty(context);
  FunctionType* func_ty = FunctionType::get(
      void_ty, {char_ptr_ty, char_ptr_ty, int_ty}, /*isVarArg=*/true);
  return module.getOrInsertFunction("__DriverTracePassing", func_ty);
}
