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
#include "llvm/ADT/APInt.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include <stdint.h>

using namespace llvm;

namespace {
struct CountDynamicInstructions : public FunctionPass {
 	static char ID;
  	CountDynamicInstructions() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {

  		Module *M = F.getParent();
  		LLVMContext &context = F.getContext();

  		Constant *updateInstrInfo = M->getOrInsertFunction(
		    "updateInstrInfo",               // name of function
		    Type::getVoidTy(context),        // return type
		    Type::getInt32Ty(context),		   // first parameter type
		    Type::getInt32PtrTy(context),      // second parameter type
		    Type::getInt32PtrTy(context)       // third parameter type
		  );

  		Constant *printOutInstrInfo = M->getOrInsertFunction(
		    "printOutInstrInfo",               // name of function
		    Type::getVoidTy(context)        // return type
		  );

  		for(Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {

  			BasicBlock &BB = *B;
  			std::vector<uint32_t> keys;
  			std::vector<uint32_t> values;

  			for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
  				keys.push_back(I->getOpcode());
  				values.push_back(1);
			}

			IRBuilder<> Builder(&BB);
			Builder.SetInsertPoint(&*B->getTerminator());
			std::vector<Value*> args1;
  			Constant* num = ConstantInt::get(IntegerType::get(context,32), keys.size());

  			ArrayType* arrayTy = ArrayType::get(IntegerType::get(context,32), keys.size());

			GlobalVariable *keyArray = new GlobalVariable(
				    *M,
				    arrayTy,
				    true,
				    GlobalValue::InternalLinkage,
				    ConstantDataArray::get(context, keys),
				    "keyArray");
			GlobalVariable *valueArray = new GlobalVariable(
			    *M,
			    arrayTy,
			    true,
			    GlobalValue::InternalLinkage,
			    ConstantDataArray::get(context, values),
			    "valueArray");

			Value* keyValue = Builder.CreatePointerCast(keyArray, Type::getInt32PtrTy(context));
			Value* valueValue = Builder.CreatePointerCast(valueArray, Type::getInt32PtrTy(context));

			args1.push_back(num);
			args1.push_back(keyValue);
			args1.push_back(valueValue);
			Builder.CreateCall(updateInstrInfo, args1);

  			for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
				if((std::string)I->getOpcodeName() == "ret") {
	  					IRBuilder<> Builder(&BB);
	  					Builder.SetInsertPoint(&*B->getTerminator());
	  					Builder.CreateCall(printOutInstrInfo);
	  			}
	  		}
  		}
  		
	    return false;
  	}
}; // end of struct TestPass
}  // end of anonymous namespace

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi", "Count the occurrence of each instruction dynamically",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);