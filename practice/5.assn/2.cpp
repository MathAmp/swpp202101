#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

// A minor comment about formatting - space after comma in function arguments.
/* the(compact,great,code, ever) -> the(compact, great, code, ever) */
// Except that, Looks great to me.
namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {


private:
  // Replace the uses of b dominated by edge with a
  void replace_operand(Value *a, Value *b, DominatorTree &DT, BasicBlockEdge &edge){
    for (auto itr = b->use_begin(), end = b->use_end(); itr != end;){
      Use &u = *itr++;
      Instruction *userI = dyn_cast<Instruction>(u.getUser());
      if (DT.dominates(edge, userI->getParent())){
        u.set(a);
      }
    }
  }

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    for (auto &BB : F) {
      for (auto &I : BB) {
        Value *x, *y;
        ICmpInst::Predicate pred;
        BasicBlock *bb_true, *bb_false;
        if (match(&I, m_Br(m_ICmp(pred, m_Value(x), m_Value(y)), bb_true, bb_false)) && pred == ICmpInst::ICMP_EQ){
          BasicBlockEdge edge(&BB, bb_true);
          Instruction *i_x = dyn_cast<Instruction>(x);
          Instruction *i_y = dyn_cast<Instruction>(y);

          // x and y are both instructions -> replace the one which is dominated by the other
          if (i_x && i_y){
            if (DT.dominates(i_x,i_y)) replace_operand(x,y,DT, edge);
            else replace_operand(y,x,DT, edge);
          } else if (i_x){
            replace_operand(y,x,DT, edge); //replace instruction x with arg y
          } else if (i_y){
            replace_operand(x,y,DT, edge); //replace insruction y with arg x
          } else { // Replace the trailing argument
            Argument *arg_x = dyn_cast<Argument>(x);
            Argument *arg_y = dyn_cast<Argument>(y);
            if (arg_x->getArgNo() < arg_y->getArgNo()){
              replace_operand(x,y,DT, edge);
            } else {
              replace_operand(y,x,DT, edge);
            }
          }
          break;
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
