#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "231DFA.h"
#include <utility>
#include <vector>
#include <set>

using namespace llvm;

class LivenessInfo : public Info {
	

public:
	LivenessInfo() {}
	LivenessInfo(unsigned index) {
		liveness_idx.insert(index);
	}
	LivenessInfo(LivenessInfo& other) : Info(other) {
		setInfo(other.getInfo());
	}

	std::set<unsigned> liveness_idx;

	void print() {
		for(std::set<unsigned>::iterator it = liveness_idx.begin(); it != liveness_idx.end(); ++it){
			errs() << *it << "|";
		}
		errs() << "\n";
	}

	static bool equals(Info * info1, Info * info2) {
		if(((LivenessInfo *)info1)->getInfo() == ((LivenessInfo *)info2)->getInfo()) return true;
		else return false;
	}

	static LivenessInfo* join (LivenessInfo * info1, LivenessInfo * info2, Info * result) {
		std::set<unsigned> res;
		std::set<unsigned> input1 = ((LivenessInfo *)info1)->getInfo();
		std::set<unsigned> input2 = ((LivenessInfo *)info2)->getInfo();
		for(std::set<unsigned>::iterator it = input1.begin(); it != input1.end(); ++it){
			res.insert(*it);
		}
		for(std::set<unsigned>::iterator it = input2.begin(); it != input2.end(); ++it){
			res.insert(*it);
		}
		LivenessInfo * resLIVE = new LivenessInfo();
		resLIVE->setInfo(res);
		return resLIVE;
	}

	void remove(unsigned idx){
		this->liveness_idx.erase(idx);
	}

	void insert(unsigned idx){
		this->liveness_idx.insert(idx);
	}

	std::set<unsigned> getInfo(){
		return this->liveness_idx;
	}
	void setInfo(std::set<unsigned> new_liveness_idx){
		this->liveness_idx = new_liveness_idx;
	}
};

template <class Info, bool Direction>
class LivenessAnalysis : public DataFlowAnalysis<Info, Direction> {
public:
	LivenessAnalysis(LivenessInfo &bottom, LivenessInfo &initialState) : 
		DataFlowAnalysis<Info, Direction>::DataFlowAnalysis(bottom, initialState){}

	void flowfunction(Instruction * I,
    									std::vector<unsigned> & IncomingEdges,
										std::vector<unsigned> & OutgoingEdges,
										std::vector<Info *> & Infos) {

		unsigned idx = this->InstrToIndex[I];
		LivenessInfo *combineInfo = new LivenessInfo();
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
			combineInfo->remove(idx);
			for(Use &U : I->operands()){
				Instruction *inst = (Instruction*) U.get();
				if(this->InstrToIndex.find(inst) != this->InstrToIndex.end()){
					unsigned operand_idx = this->InstrToIndex[cast<Instruction>(inst)];
					combineInfo = Info::join(combineInfo, new LivenessInfo(operand_idx), combineInfo);
				}
			}
			for(unsigned i=0; i<OutgoingEdges.size(); ++i){
				Infos.push_back(new LivenessInfo(*combineInfo));
			}
		}
		else if(instrName == "phi"){
			Instruction *I_ = I;
			while(isa<PHINode>(I_)){
				combineInfo->remove(this->InstrToIndex[I_]);
				I_ = I_->getNextNode();
			}
			for(unsigned i=0; i<OutgoingEdges.size(); ++i){
				Infos.push_back(new LivenessInfo(*combineInfo));
			}
			I_ = I;
			while(isa<PHINode>(I_)){
				PHINode* phi_inst = (PHINode*) I_;
				for(unsigned i = 0; i<phi_inst->getNumIncomingValues(); i++){
					if(this->InstrToIndex.find((Instruction*)(phi_inst->getIncomingValue(i))) == this->InstrToIndex.end()){
						continue;
					}
					Instruction *pre_inst = (phi_inst->getIncomingBlock(i))->getTerminator();
					for(unsigned j = 0; j<OutgoingEdges.size(); j++){
						if(OutgoingEdges[j] == this->InstrToIndex[pre_inst]){
							unsigned value_inst = this->InstrToIndex[(Instruction*)(phi_inst->getIncomingValue(i))];
							Infos[j]->insert(value_inst);
						}
					}
				}
				I_ = I_->getNextNode();
			}
		}
		else{
			for(Use &U : I->operands()){
				Instruction *inst = (Instruction*) U.get();
				if(this->InstrToIndex.find(inst) != this->InstrToIndex.end()){
					unsigned operand_idx = this->InstrToIndex[cast<Instruction>(inst)];
					combineInfo = Info::join(combineInfo, new LivenessInfo(operand_idx), combineInfo);
				}
			}
			for(unsigned i=0; i<OutgoingEdges.size(); ++i){
				Infos.push_back(new LivenessInfo(*combineInfo));
			}
		}
	}
};

namespace {
struct LivenessAnalysisPass : public FunctionPass {
 	static char ID;
  	LivenessAnalysisPass() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {
  		LivenessInfo *bottom = new LivenessInfo();
  		LivenessAnalysis<LivenessInfo, false> analysis(*bottom, *bottom);
  		analysis.runWorklistAlgorithm(&F);
  		analysis.print();

  		return false;
  	}
}; // end of struct
}  // end of anonymous namespace

char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> X("cse231-liveness", "liveness analysis",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

