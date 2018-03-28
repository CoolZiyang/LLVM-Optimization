#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "231DFA.h"
#include <utility>
#include <vector>
#include <set>

using namespace llvm;

class MayPointToInfo : public Info {
	

public:
	MayPointToInfo() {}
	MayPointToInfo(unsigned pointer, unsigned pointee) {
		std::set<unsigned> pointee_set;
		pointee_set.insert(pointee);
		pointer_map.insert(make_pair(pointer, pointee_set));
	}
	MayPointToInfo(MayPointToInfo& other) : Info(other) {
		setInfo(other.getInfo());
		setMemInfo(other.getMemInfo());
	}

	std::map<unsigned, std::set<unsigned>> pointer_map;
	std::map<unsigned, std::set<unsigned>> mem_pointer_map;

	void print() {
		// Assume there no entry with empty pointee set
		for(std::map<unsigned, std::set<unsigned>>::iterator it = pointer_map.begin(); it != pointer_map.end(); ++it){
			errs() << "R" << it->first << "->" << "(";
			for(std::set<unsigned>::iterator it_ = (it->second).begin(); it_ != (it->second).end(); ++it_){
				errs() << "M" << *it_ << "/";
			}
			errs() << ")" << "|";
		}
		for(std::map<unsigned, std::set<unsigned>>::iterator it = mem_pointer_map.begin(); it != mem_pointer_map.end(); ++it){
			errs() << "M" << it->first << "->" << "(";
			for(std::set<unsigned>::iterator it_ = (it->second).begin(); it_ != (it->second).end(); ++it_){
				errs() << "M" << *it_ << "/";
			}
			errs() << ")" << "|";
		}
		errs() << "\n";
	}

	static bool equals(Info * info1, Info * info2) {
		std::map<unsigned, std::set<unsigned>> map1 = ((MayPointToInfo *)info1)->getInfo();
		std::map<unsigned, std::set<unsigned>> map2 = ((MayPointToInfo *)info2)->getInfo();
		std::map<unsigned, std::set<unsigned>> mem_map1 = ((MayPointToInfo *)info1)->getMemInfo();
		std::map<unsigned, std::set<unsigned>> mem_map2 = ((MayPointToInfo *)info2)->getMemInfo();
		return map1.size() == map2.size() && std::equal(map1.begin(), map1.end(), map2.begin())
		&& mem_map1.size() == mem_map2.size() && std::equal(mem_map1.begin(), mem_map1.end(), mem_map2.begin());
	}

	static MayPointToInfo* join (MayPointToInfo * info1, MayPointToInfo * info2, Info * result) {
		std::map<unsigned, std::set<unsigned>> res;
		std::map<unsigned, std::set<unsigned>> mem_res;
		std::map<unsigned, std::set<unsigned>> input1 = ((MayPointToInfo *)info1)->getInfo();
		std::map<unsigned, std::set<unsigned>> input2 = ((MayPointToInfo *)info2)->getInfo();
		std::map<unsigned, std::set<unsigned>> mem_input1 = ((MayPointToInfo *)info1)->getMemInfo();
		std::map<unsigned, std::set<unsigned>> mem_input2 = ((MayPointToInfo *)info2)->getMemInfo();
		for(std::map<unsigned, std::set<unsigned>>::iterator it = input1.begin(); it != input1.end(); ++it){
			res[it->first] = it->second;
		}
		for(std::map<unsigned, std::set<unsigned>>::iterator it = input2.begin(); it != input2.end(); ++it){
			if(res.find(it->first) == res.end()){
				res[it->first] = it->second;
			}
			else{
				for(auto pointee : it->second){
					res[it->first].insert(pointee);
				}
			}
		}

		for(std::map<unsigned, std::set<unsigned>>::iterator it = mem_input1.begin(); it != mem_input1.end(); ++it){
			mem_res[it->first] = it->second;
		}
		for(std::map<unsigned, std::set<unsigned>>::iterator it = mem_input2.begin(); it != mem_input2.end(); ++it){
			if(mem_res.find(it->first) == mem_res.end()){
				mem_res[it->first] = it->second;
			}
			else{
				for(auto pointee : it->second){
					mem_res[it->first].insert(pointee);
				}
			}
		}

		MayPointToInfo * resMP = new MayPointToInfo();
		resMP->setInfo(res);
		resMP->setMemInfo(mem_res);
		return resMP;
	}

	void insert(unsigned pointer, unsigned pointee){
		if(this->pointer_map.find(pointer) == this->pointer_map.end()){
			std::set<unsigned> pointee_set;
			pointee_set.insert(pointee);
			this->pointer_map.insert(make_pair(pointer, pointee_set));
		}
		else{
			this->pointer_map[pointer].insert(pointee);
		}
	}

	void insertStore(unsigned mem_pointer, unsigned mem_pointee){
		if(this->mem_pointer_map.find(mem_pointer) == this->mem_pointer_map.end()){
			std::set<unsigned> mem_pointee_set;
			mem_pointee_set.insert(mem_pointee);
			this->mem_pointer_map.insert(make_pair(mem_pointer, mem_pointee_set));
		}
		else{
			this->mem_pointer_map[mem_pointer].insert(mem_pointee);
		}
	}

	std::map<unsigned, std::set<unsigned>> getInfo(){
		return this->pointer_map;
	}

	std::map<unsigned, std::set<unsigned>> getMemInfo(){
		return this->mem_pointer_map;
	}

	void setInfo(std::map<unsigned, std::set<unsigned>> new_pointer_map){
		this->pointer_map = new_pointer_map;
	}

	void setMemInfo(std::map<unsigned, std::set<unsigned>> new_pointer_map){
		this->mem_pointer_map = new_pointer_map;
	}
};

template <class Info, bool Direction>
class MayPointToAnalysis : public DataFlowAnalysis<Info, Direction> {
public:
	MayPointToAnalysis(MayPointToInfo &bottom, MayPointToInfo &initialState) : 
		DataFlowAnalysis<Info, Direction>::DataFlowAnalysis(bottom, initialState){}

	void flowfunction(Instruction * I,
    									std::vector<unsigned> & IncomingEdges,
										std::vector<unsigned> & OutgoingEdges,
										std::vector<Info *> & Infos) {

		unsigned idx = this->InstrToIndex[I];
		MayPointToInfo *combineInfo = new MayPointToInfo();
		for(std::vector<unsigned>::iterator it = IncomingEdges.begin(); it != IncomingEdges.end(); ++it){
			combineInfo = Info::join(combineInfo, this->EdgeToInfo[std::make_pair(*it, idx)], combineInfo);
		}
		std::string instrName = I->getOpcodeName();

		Instruction *Rv;
		Instruction *Rp;
		Instruction *R1;
		Instruction *R2;

		if(instrName == "alloca"){
			combineInfo->insert(idx, idx);
		}

		else if(instrName == "bitcast"){
			Rv = (Instruction*)(((CastInst*)I)->getOperand(0));
			if(this->InstrToIndex.find(Rv) != this->InstrToIndex.end()){
				unsigned Rv_idx = this->InstrToIndex[Rv];
				if((combineInfo->getInfo()).find(Rv_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set = combineInfo->getInfo()[Rv_idx];
					for(auto pointee : pointee_set)
						combineInfo->insert(idx, pointee);
				}
			}
		}

		else if(instrName == "getelementptr"){
			Rv = (Instruction*)(((GetElementPtrInst*)I)->getPointerOperand());
			if(this->InstrToIndex.find(Rv) != this->InstrToIndex.end()){
				unsigned Rv_idx = this->InstrToIndex[Rv];
				if((combineInfo->getInfo()).find(Rv_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set = combineInfo->getInfo()[Rv_idx];
					for(auto pointee : pointee_set)
						combineInfo->insert(idx, pointee);
				}
			}
		}

		else if(instrName == "load"){
			Rp = (Instruction*)(((LoadInst*)I)->getPointerOperand());
			if(this->InstrToIndex.find(Rv) != this->InstrToIndex.end()){
				unsigned Rv_idx = this->InstrToIndex[Rv];
				if((combineInfo->getInfo()).find(Rv_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set1 = combineInfo->getInfo()[Rv_idx];
					for(auto X : pointee_set1){
						if((combineInfo->getMemInfo()).find(X) != (combineInfo->getMemInfo()).end()){
							std::set<unsigned> pointee_set2 = combineInfo->getMemInfo()[X];
							for(auto Y : pointee_set2){
								combineInfo->insert(idx, Y);
							}
						}
					}
				}
			}
		}

		else if(instrName == "store"){
			Rv = (Instruction*)(((StoreInst*)I)->getValueOperand());
			Rp = (Instruction*)(((StoreInst*)I)->getPointerOperand());
			if(this->InstrToIndex.find(Rv) != this->InstrToIndex.end() 
				&& this->InstrToIndex.find(Rp) != this->InstrToIndex.end()){
				unsigned Rv_idx = this->InstrToIndex[Rv];
				unsigned Rp_idx = this->InstrToIndex[Rp];
				if((combineInfo->getInfo()).find(Rv_idx) != (combineInfo->getInfo()).end()
					&& (combineInfo->getInfo()).find(Rp_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set1 = combineInfo->getInfo()[Rv_idx];
					std::set<unsigned> pointee_set2 = combineInfo->getInfo()[Rp_idx];
					for(auto X : pointee_set1){
						for(auto Y : pointee_set2){
							combineInfo->insertStore(Y, X);
						}
					}
				}
			}
		}

		else if(instrName == "select"){
			R1 = (Instruction*)(((SelectInst*)I)->getTrueValue());
			R2 = (Instruction*)(((SelectInst*)I)->getFalseValue());
			if(this->InstrToIndex.find(R1) != this->InstrToIndex.end()){
				unsigned R1_idx = this->InstrToIndex[R1];
				if((combineInfo->getInfo()).find(R1_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set1 = combineInfo->getInfo()[R1_idx];
					for(auto pointee : pointee_set1)
						combineInfo->insert(idx, pointee);
				}
			}
			if(this->InstrToIndex.find(R2) != this->InstrToIndex.end()){
				unsigned R2_idx = this->InstrToIndex[R2];
				if((combineInfo->getInfo()).find(R2_idx) != (combineInfo->getInfo()).end()){
					std::set<unsigned> pointee_set2 = combineInfo->getInfo()[R2_idx];
					for(auto pointee : pointee_set2)
						combineInfo->insert(idx, pointee);
				}
			}
		}

		else if(instrName == "phi"){
			while(isa<PHINode>(I)){
				PHINode* phi_inst = (PHINode*) I;
				for(unsigned i = 0; i<phi_inst->getNumIncomingValues(); i++){
					Rv = (Instruction*)(phi_inst->getIncomingValue(i));
					if(this->InstrToIndex.find(Rv) != this->InstrToIndex.end()){
						unsigned Rv_idx = this->InstrToIndex[Rv];
						if((combineInfo->getInfo()).find(Rv_idx) != (combineInfo->getInfo()).end()){
							std::set<unsigned> pointee_set = combineInfo->getInfo()[Rv_idx];
							for(auto pointee : pointee_set)
								combineInfo->insert(idx, pointee);
						}
					}
				}
				I = I->getNextNode();
			}
		}

		for(unsigned i=0; i<OutgoingEdges.size(); ++i){
			Infos.push_back(new MayPointToInfo(*combineInfo));
		}
	}
};

namespace {
struct MayPointToAnalysisPass : public FunctionPass {
 	static char ID;
  	MayPointToAnalysisPass() : FunctionPass(ID) {}

  	bool runOnFunction(Function &F) override {
  		MayPointToInfo *bottom = new MayPointToInfo();
  		MayPointToAnalysis<MayPointToInfo, true> analysis(*bottom, *bottom);
  		analysis.runWorklistAlgorithm(&F);
  		analysis.print();

  		return false;
  	}
}; // end of struct
}  // end of anonymous namespace

char MayPointToAnalysisPass::ID = 0;
static RegisterPass<MayPointToAnalysisPass> X("cse231-maypointto", "may-point-to analysis",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

