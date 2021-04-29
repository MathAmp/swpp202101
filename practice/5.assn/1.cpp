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

// This code could be divided in 3 steps.
// 1) Save all domination tree for Edge(BranchInst->Next) to BasicBlock.
// 2) For all instruction, mark (1) what to replace (2) which part to replace
// 3) For each blocks, do replace based on the information found in step 2)
// Please keep consistent indent rule and exploit more abstraction methods.
// There are unnecessary structures. I recommend using more llvm templates.
// I explained in code what kinds of counterexamples each code can cause.
// There is not enough space for counterexamples, so I attached them next.
/* EXAMPLE 1 [ -> is result of given code, and => is true result ]
 * define void @f(i32 %x, i32 %y) {
 *   %cond1 = icmp eq i32 %x, %x
 *   %cond2 = icmp eq i32 %y, %y
 *   %cond = icmp eq i1 %cond1, %cond2  -> %cond2, %cond2 => %cond1, %cond1
 *   br i1 %cond, label %BB_equal, label %BB_not (omitted)
 * BB_equal:
 *   %two1 = add i1 %cond1, %cond2 -> %cond2, %cond2 (depend on initial value)
 *   ret void                      => %cond1, %cond1
 * } */
/* EXAMPLE 2 [ -> is result of given code, and => is true result ]
 * define i32 @f(i32 %a, i32 %b, i32 %c, i32 %d)
 *   %cond = icmp eq i32 %a, %b
 *   br i1 %cond, label %bb_true, label %bb_exit
 * bb_exit:
 *   ret i32 %a
 * bb_true3:
 *   call i32 @f(i32 %a, i32 %b, i32 %c, i32 %d)    -> @f(%a, %a, %b, %b)
 *   br label %bb_exit                              => @f(%a, %a, %a, %a)
 * bb_true2:
 *   call i32 @f(i32 %a, i32 %b, i32 %b, i32 %d)    -> @f(%a, %a, %b, %d)
 *                                                  => @f(%a, %a, %a, %d)
 *   %cond3 = icmp eq i32 %b, %d                    -=> %a, %d
 *   br i1 %cond3, label %bb_true3, label %bb_exit
 * bb_true:
 *   call i32 @f(i32 %a, i32 %b, i32 %c, i32 %d)    -=> @f(%a, %a, %c, %d)
 *   %cond2 = icmp eq i32 %b, %c                    -=> %a, %c
 *   br i1 %cond2, label %bb_true2, label %bb_exit
 * } */
namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    // Do not use TOO many nested pairs. Wrap them in a struct or class.
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
				// Predicate Num is hardcoded. Use llvm constant like next.
				/* IC->getPredicate() == CmpInst::ICMP_EQ */
				if(IC != NULL && (IC->getPredicate() == 32 /*|| IC->getPredicate() == 33*/)){
					Value *V1 = I.getOperand(0);
					Value *V2 = I.getOperand(1);
					StringRef name1 = V1->getName();
					StringRef name2 = V2->getName();
					int index1, type1, index2, type2; 
					// Next two loops can be abstracted with a Search method.
					// !If name1(or 2) not in inst or arg, undefined behavior!
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
					// Dividing the case of [arg] and [inst] gets complicated.
					// [arg] always precedes [inst], so think [arg] + [inst].
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
	  		    // Better to check dyn_cast<BranchInst>(&I)->isConditional()
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
	  		// ! It does not push icmp inst. THIS MAY CAUSE COUNTER EXAMPLE. !
          		default: {
				inst.push_back(I.getName());
                		break;
	  		}
			}
		}
	}
    
	// Saving domination relation only used here is unnecessary.
	// This algorithm search 2|F| dom relations but you should search at least
	// |F| ^ 2 dom relations for unordered basic blocks. THIS MAY CAUSE C.E. !
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
