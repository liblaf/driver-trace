#include "inserter.h"

#include "llvm/IR/IRBuilder.h"         // IRBuilder
#include "llvm/IR/Instructions.h"      // CallInst
#include "llvm/IR/Type.h"              // Type, PointerType
#include "llvm/IR/Verifier.h"          // verifyModule()
#include "llvm/IRReader/IRReader.h"    // parseIRFile()
#include "llvm/Support/FileSystem.h"   // sys::fs::
#include "llvm/Support/SourceMgr.h"    // SMDiagnostic
#include "llvm/Support/raw_ostream.h"  // raw_fd_ostream

bool Inserter::Instrument(const std::filesystem::path& input,
                          const std::filesystem::path& output) {
  using namespace llvm;
  LLVMContext context;
  SMDiagnostic err;
  auto owner = parseIRFile(input.string(), err, context);
  if (!(owner)) {
    errs() << "parseIRFile(): " << err.getMessage() << "\n";
    return false;
  }

  this->RunOnModule(*(owner));

  if (verifyModule(*(owner), &errs())) {
    errs() << "Generated module is incorrect!"
           << "\n";
    return false;
  }
  dbgs() << "verifyModule(): success"
         << "\n";
  dbgs().flush();
  std::error_code err_code;
  auto out = raw_fd_ostream(output.string(), err_code, sys::fs::OF_None);
  owner->print(out, nullptr);
  dbgs() << "write to file success"
         << "\n";
  dbgs().flush();
  return true;
}

void Inserter::RunOnModule(llvm::Module& mod) {
  using namespace llvm;
  this->on_init_ = Inserter::InsertOnInitDeclaration(mod);
  this->on_cleanup_ = Inserter::InsertOnCleanupDeclaration(mod);
  this->passing_ = Inserter::InsertPassingDeclaration(mod);
  for (Function& func : mod) this->RunOnFunction(func);
}

void Inserter::RunOnFunction(llvm::Function& func) {
  using namespace llvm;
  if (func.isDeclaration()) return;
  IRBuilder<> builder(&*func.getEntryBlock().getFirstInsertionPt());
  Constant* caller_name = builder.CreateGlobalStringPtr(func.getName());
  for (BasicBlock& block : func) RunOnBasicBlock(block, caller_name);
}

void Inserter::RunOnBasicBlock(llvm::BasicBlock& block,
                               llvm::Constant* caller_name) {
  using namespace llvm;
  for (auto iter = block.begin(); iter != block.end(); ++iter)
    this->RunOnInst(*iter, caller_name);
}

void Inserter::RunOnInst(llvm::Instruction& inst, llvm::Constant* caller_name) {
  using namespace llvm;
  dbgs() << inst << "\n";
  dbgs().flush();
  if (isa<CallInst>(inst))
    this->RunOnCallInst(*dyn_cast<CallInst>(&inst), caller_name);
}

void Inserter::RunOnCallInst(llvm::CallInst& inst,
                             llvm::Constant* caller_name) {
  using namespace llvm;
  Function* called_function = inst.getCalledFunction();
  if (!called_function) return;
  if (called_function->getName() == "__DriverTracePassing") return;
  auto builder = IRBuilder<>(&inst);
  Constant* func_name =
      builder.CreateGlobalStringPtr(inst.getCalledFunction()->getName());
  LLVMContext& context = inst.getContext();
  IntegerType* int_ty = IntegerType::getInt32Ty(context);
  auto num_of_params = ConstantInt::get(int_ty, inst.arg_size());
  CallInst* passing = builder.CreateCall(
      this->passing_, {func_name, caller_name, num_of_params});
  passing->insertAfter(&inst);
}

llvm::FunctionCallee Inserter::InsertOnInitDeclaration(llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceOnInit(void);
  Type* void_ty = Type::getVoidTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, /*isVarArg=*/false);
  return module.getOrInsertFunction("__DriverTraceOnInit", func_ty);
}

llvm::FunctionCallee Inserter::InsertOnCleanupDeclaration(
    llvm::Module& module) {
  using namespace llvm;
  LLVMContext& context = module.getContext();

  // void __DriverTraceOnCleanup(void);
  Type* void_ty = Type::getVoidTy(context);
  FunctionType* func_ty = FunctionType::get(void_ty, /*isVarArg=*/false);
  return module.getOrInsertFunction("__DriverTraceOnCleanup", func_ty);
}

llvm::FunctionCallee Inserter::InsertPassingDeclaration(llvm::Module& module) {
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
