#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include <stdint.h>

using namespace llvm;

namespace {
struct BranchBias : public FunctionPass {
 	static char ID;
  	BranchBias() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {

  		Module *M = F.getParent();
  		LLVMContext &context = F.getContext();

  		Constant *updateBranchInfo = M->getOrInsertFunction(
		    "updateBranchInfo",               // name of function
		    Type::getVoidTy(context),        // return type
		    Type::getInt1Ty(context)		   // first parameter type
		  );

  		Constant *printOutBranchInfo = M->getOrInsertFunction(
		    "printOutBranchInfo",               // name of function
		    Type::getVoidTy(context)        // return type
		  );

  		for(Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
  			BasicBlock &BB = *B;
  			for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
  				if((std::string)I->getOpcodeName() == "br") {
  					BranchInst *bi = dyn_cast<BranchInst>(&*I);
  					if(bi->isConditional()) {
  						IRBuilder<> Builder(&*I);
  						std::vector<Value*> args1;
  						args1.push_back(bi->getCondition());
  						Builder.CreateCall(updateBranchInfo, args1);
  					}
  				}
			}

  			for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
				if((std::string)I->getOpcodeName() == "ret") {
	  					IRBuilder<> Builder(&BB);
	  					Builder.SetInsertPoint(&*B->getTerminator());
	  					Builder.CreateCall(printOutBranchInfo);
	  			}
	  		}
  		}
  		
	    return false;
  	}
}; // end of struct TestPass
}  // end of anonymous namespace

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Compute the branch bias on a per-function basis",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);