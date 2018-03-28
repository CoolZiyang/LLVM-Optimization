#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "231DFA.h"
#include <utility>
#include <vector>
#include <set>

using namespace llvm;

class ReachingInfo : public Info {
private:
	std::set<unsigned> reaching_idx;

public:
	ReachingInfo() {}
	ReachingInfo(unsigned index) {
		reaching_idx.insert(index);
	}
	ReachingInfo(ReachingInfo& other) : Info(other) {
		setInfo(other.getInfo());
	}
	void print() {
		for(std::set<unsigned>::iterator it = reaching_idx.begin(); it != reaching_idx.end(); ++it){
			errs() << *it << "|";
		}
		errs() << "\n";
	}

	static bool equals(Info * info1, Info * info2) {
		if(((ReachingInfo *)info1)->getInfo() == ((ReachingInfo *)info2)->getInfo()) return true;
		else return false;
	}

	static ReachingInfo* join (ReachingInfo * info1, ReachingInfo * info2, Info * result) {
		std::set<unsigned> res;
		std::set<unsigned> input1 = ((ReachingInfo *)info1)->getInfo();
		std::set<unsigned> input2 = ((ReachingInfo *)info2)->getInfo();
		for(std::set<unsigned>::iterator it = input1.begin(); it != input1.end(); ++it){
			res.insert(*it);
		}
		for(std::set<unsigned>::iterator it = input2.begin(); it != input2.end(); ++it){
			res.insert(*it);
		}
		ReachingInfo * resRI = new ReachingInfo();
		resRI->setInfo(res);
		return resRI;
	}

	std::set<unsigned> getInfo(){
		return this->reaching_idx;
	}
	void setInfo(std::set<unsigned> new_reaching_idx){
		this->reaching_idx = new_reaching_idx;
	}
};

template <class Info, bool Direction>
class ReachingDefinitionAnalysis : public DataFlowAnalysis<Info, Direction> {
public:
	ReachingDefinitionAnalysis(ReachingInfo &bottom, ReachingInfo &initialState) : 
		DataFlowAnalysis<Info, Direction>::DataFlowAnalysis(bottom, initialState){}

	void flowfunction(Instruction * I,
    									std::vector<unsigned> & IncomingEdges,
										std::vector<unsigned> & OutgoingEdges,
										std::vector<Info *> & Infos) {

		unsigned idx = this->InstrToIndex[I];
		ReachingInfo *combineInfo = new ReachingInfo();
		for(std::vector<unsigned>::iterator it = IncomingEdges.begin(); it != IncomingEdges.end(); ++it){
			combineInfo = Info::join(combineInfo, this->EdgeToInfo[std::make_pair(*it, idx)], combineInfo);
		}
		std::string instrName = I->getOpcodeName();
		if(instrName == "add" ||
			instrName == "fadd" ||
			instrName == "sub" ||
			instrName == "fsub" ||
			instrName == "mul" ||
			instrName == "fmul" ||
			instrName == "udiv" ||
			instrName == "sdiv" ||
			instrName == "fdiv" ||
			instrName == "urem" ||
			instrName == "srem" ||
			instrName == "frem" ||
			instrName == "shl" ||
			instrName == "lshr" ||
			instrName == "ashr" ||
			instrName == "and" ||
			instrName == "or" ||
			instrName == "xor" ||
			instrName == "alloca" ||
			instrName == "load" ||
			instrName == "getelementptr" ||
			instrName == "icmp" ||
			instrName == "fcmp" ||
			instrName == "select"){
			combineInfo = Info::join(combineInfo, new ReachingInfo(idx), combineInfo);
		}
		else if(instrName == "phi"){
			while(isa<PHINode>(I)){
				combineInfo = Info::join(combineInfo, new ReachingInfo(this->InstrToIndex[I]), combineInfo);
				I = I->getNextNode();
			}
		}
		for(unsigned i=0; i<OutgoingEdges.size(); ++i){
			Infos.push_back(new ReachingInfo(*combineInfo));
		}
	}
};

namespace {
struct ReachingDefinitionAnalysisPass : public FunctionPass {
 	static char ID;
  	ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {
  		ReachingInfo *bottom = new ReachingInfo();
  		ReachingDefinitionAnalysis<ReachingInfo, true> analysis(*bottom, *bottom);
  		analysis.runWorklistAlgorithm(&F);
  		analysis.print();

  		return false;
  	}
}; // end of struct
}  // end of anonymous namespace

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "reaching definition analysis",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

