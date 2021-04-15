#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {

private:
  class ChangeInfo {
    public:
      bool isEqual;
      Value *replacer;
      Value *replacee;
      ChangeInfo() {}
      ChangeInfo(bool _isEqual, Value *_replacer, Value *_replacee) 
        : isEqual(_isEqual), replacer(_replacer), replacee(_replacee) {}  

  };

  StringRef &precedeString(SmallVector<StringRef> &vec, StringRef &s1, StringRef &s2) {
    for (StringRef &s: vec) {
      if ((s == s1) || (s == s2)) return s;
    }
    return s1;
  }

  ChangeInfo createChangeInfo(bool isEqual, SmallVector<StringRef> &vec, Value *v1, Value *v2) {
    StringRef s1 = v1->getName();
    StringRef s2 = v2->getName();

    StringRef &s = precedeString(vec, s1, s2);
    bool isFirst = (s == s1);

    Value *replacer = isFirst ? v1 : v2;
    Value *replacee = isFirst ? v2 : v1;
    return ChangeInfo(isEqual, replacer, replacee);
  }

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    SmallVector<StringRef> symbols;
    SmallDenseMap<StringRef, ChangeInfo> infoMap;

    // push function arguments
    for (Argument &arg: F.args()) {
      symbols.push_back(arg.getName());
    }

    // For all BasicBlocks,
    // Check all instructions to
    // 0) push new variable
    // 1) check icmp instruction that can be used to replace
    // And about terminator
    // 0) Replace the variable if edge with BB and next is dominate
    for (BasicBlock &BB: F) {
      // All Insturctions, do 0) and 1)
      for (Instruction &I: BB) {
        ICmpInst *icmpInst;
        StringRef name = I.getName();

        // if there is new variable
        if (!name.empty()) {
          // 0) push variable
          symbols.push_back(name);

          // if icmp eq/neq instruction
          if ((icmpInst = dyn_cast<ICmpInst>(&I)) &&
              (icmpInst->isEquality() &&
              (icmpInst->getNumOperands() == 2))) {
            CmpInst::Predicate pred = icmpInst->getPredicate();
            Value *v1 = icmpInst->getOperand(0);
            Value *v2 = icmpInst->getOperand(1);

            // for different operands name (if equal then no need to change)
            if (v1->getName() != v2->getName()) {
              bool isEqual = (pred == CmpInst::ICMP_EQ); // if not, it is neq becuase checked isEquality()
              ChangeInfo info = createChangeInfo(isEqual, symbols, v1, v2);
              infoMap.insert(make_pair(name, info));
            }
          }
        }
      }

      // About terminator
      // replace variable
      BranchInst *brInst;
      Instruction *term = BB.getTerminator();
      // if terminator exist and conditional branch instruction
      if (term && (brInst = dyn_cast<BranchInst>(term)) && 
          (brInst->getNumOperands() > 1) &&
          (infoMap.find(brInst->getOperand(0)->getName()) != infoMap.end())) {
        Value *cond = brInst->getOperand(0);
        ChangeInfo info = infoMap[cond->getName()];

        // only icmp eq, replace
        if (info.isEqual) {
          unsigned idx = 0; 
          BasicBlock *next = term->getSuccessor(idx);
          BasicBlockEdge BBE(&BB, next);

          // if block edge (block -> true condition block) is dominates, replace
          for (BasicBlock &basicBlock: F)
            if (DT.dominates(BBE, &basicBlock))
              for (Instruction &inst: basicBlock)
                inst.replaceUsesOfWith(info.replacee, info.replacer);
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
