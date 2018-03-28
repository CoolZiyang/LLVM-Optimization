#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <map>
#include <iterator>

using namespace llvm;

namespace {
struct CountStaticInstructions : public FunctionPass {
 	static char ID;
  	CountStaticInstructions() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {
  		std::map <std::string, int> countMap;
  		for(inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I) {
  			const char* codeName = I->getOpcodeName();
	  			if(countMap.find(codeName) == countMap.end()) {
	  				countMap[codeName] = 1;
	  			}
	  			else {
	  				countMap[codeName]++;
	  			}
  		}	
  		std::map <std::string, int>::iterator it = countMap.begin();
  		while(it != countMap.end()) {
  			errs() << it->first << "\t" << it->second << "\n";
  			it++;
  		}
	    return false;
  	}
}; // end of struct TestPass
}  // end of anonymous namespace

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi", "Count the occurrence of each instruction statically",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);