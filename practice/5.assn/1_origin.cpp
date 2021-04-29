#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <string>
using namespace llvm;
using namespace std;

namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
	set<pair< BasicBlock *, pair<BasicBlock * , BasicBlock * > > > dom;
	set<pair<pair<BasicBlock *, BasicBlock * > , BasicBlock * > > inv_dom;
	vector<StringRef> inst;
	vector<StringRef> arg;
	for(auto a = F.arg_begin() ; a != F.arg_end() ; a++){
		arg.push_back(a->getName());
	}
	DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
	for(BasicBlock &BBstart : F){
  		BranchInst *TI = dyn_cast<BranchInst>(BBstart.getTerminator());
      		if (!TI) {
      		} else {
			for(int i = 0 ; i < TI->getNumSuccessors() ; i++){
          			BasicBlock *BBNext = TI->getSuccessor(i);
          			BasicBlockEdge BBE(&BBstart, BBNext);
   
  	  			for (BasicBlock &BB : F) {
          				if (DT.dominates(BBE, &BB)) {
	      					dom.insert(make_pair(&BB, make_pair(&BBstart, BBNext)));
						inv_dom.insert(make_pair(make_pair(&BBstart, BBNext) , &BB));
					}
          			}
			}
      		}
	}

	map<StringRef, pair<int, pair<Value * , Value * > > > icmp;
	map<pair<BasicBlock * , BasicBlock * > , pair<Value *, Value *> > edge_pair;
	for (auto &BB : F) {
		for (auto &I : BB) {
			switch (I.getOpcode()) {
			case Instruction::ICmp:{
				ICmpInst *IC = dyn_cast<ICmpInst>(&I);
				if(IC != NULL && (IC->getPredicate() == 32 /*|| IC->getPredicate() == 33*/)){
					Value *V1 = I.getOperand(0);
					Value *V2 = I.getOperand(1);
					StringRef name1 = V1->getName();
					StringRef name2 = V2->getName();
					int index1, type1, index2, type2; 
					for(int i = 0 ; i < arg.size() ; i++){
						if(arg[i] == name1){
							type1 = 1;
							index1 = i;
						}
						else if(arg[i] == name2){
							type2 = 1;
							index2 = i;
						}
					}
					for(int i = 0 ; i < inst.size() ; i++){
						if(inst[i] == name1){
							type1 = 2;
							index1 = i;
						}
						else if(inst[i] == name2){
							type2 = 2;
							index2 = i;
						}
					}
					if(type1 == type2){
						if(index1 < index2) icmp.insert(make_pair(I.getName() , make_pair(IC->getPredicate(),make_pair(V2, V1))));
						else icmp.insert(make_pair(I.getName(), make_pair(IC->getPredicate(), make_pair(V1, V2))));
					} else{
						if(type1 < type2) icmp.insert(make_pair(I.getName() , make_pair(IC->getPredicate(), make_pair(V2, V1))));
						else icmp.insert(make_pair(I.getName(), make_pair(IC->getPredicate(), make_pair(V1, V2))));
					}
				}
				break;
	  		}
	  		case Instruction::Br: {
				if(I.getNumSuccessors() < 2)
					break;
				Value * V1 = I.getOperand(0);
	        		map<StringRef, pair<int, pair<Value * , Value * > > >::iterator iter = icmp.find(V1->getName());
				if(iter != icmp.end()){
					Value *from = (iter->second).second.first;
					Value *to = (iter->second).second.second;
					int typ = (iter->second).first;
					if(typ == 32){
						BasicBlock * next = I.getSuccessor(0);
						edge_pair.insert(make_pair(make_pair(&BB, next), make_pair(from, to)));
					}
					else if(typ == 33){
						BasicBlock * next = I.getSuccessor(1);
						edge_pair.insert(make_pair(make_pair(&BB, next), make_pair(from, to)));
					}
				}
				break;
	  		}
          		default: {
				inst.push_back(I.getName());
                		break;
	  		}
			}
		}
	}
    
	for (auto &BB : F) {
		for(auto dom_edge : dom){
			if(dom_edge.first == &BB){
				if(edge_pair.find(dom_edge.second) != edge_pair.end()){
					Value *from = edge_pair.find(dom_edge.second)->second.first;
					Value *to = edge_pair.find(dom_edge.second)->second.second;
					from->replaceUsesWithIf(to, [&BB](Use &U) {
					auto *In = dyn_cast<Instruction>(U.getUser());
					return !In || In->getParent() == &BB;
					});
				}
			}
		}

  		BranchInst *TI = dyn_cast<BranchInst>(BB.getTerminator());
      		if (!TI) {
      		} else {
		for(int i = 0 ; i < TI->getNumSuccessors() ; i++){
          		BasicBlock *BBNext = TI->getSuccessor(i);
			for(auto dom_edge : inv_dom){
				if(dom_edge.first == make_pair(&BB, BBNext)){
					BasicBlock *target = dom_edge.second;
					if(edge_pair.find(dom_edge.first) != edge_pair.end()){
						Value *from = (edge_pair.find(dom_edge.first)->second).first;
						Value *to = (edge_pair.find(dom_edge.first)->second).second;
						from->replaceUsesWithIf(to, [target](Use &U) {
						auto *In = dyn_cast<Instruction>(U.getUser());
						return !In || In->getParent() == target;
						});
					}
				}
			}
		}
		}
	}
	return PreservedAnalyses::all();
  }
};
}

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "PropagateIntegerEquality", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "prop-int-eq") {
            FPM.addPass(PropagateIntegerEquality());
            return true;
          }
          return false;
        }
      );
    }
  };
}
