#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {

private:
  // Keep from value(replacee) and to value(replacer).
  // For extensibility, keep "is the instruction 'eq' icmp?". False, then ne
  class ChangeScheme {
    public:
      bool isEqual;
      Value *replacer;
      Value *replacee;
      ChangeScheme() {}
      ChangeScheme(bool _isEqual, Value *_replacer, Value *_replacee) 
        : isEqual(_isEqual), replacer(_replacer), replacee(_replacee) {}  

  };

  // Query preceded string
  StringRef &precedeString(SmallVector<StringRef> &vec, StringRef &s1, StringRef &s2) {
    for (StringRef &s: vec) {
      if ((s == s1) || (s == s2)) return s;
    }
    return s1;
  }

  // For value v1 and v2, select preceded string and make ChangeScheme
  ChangeScheme createChangeScheme(bool isEqual, SmallVector<StringRef> &vec, Value *v1, Value *v2) {
    StringRef s1 = v1->getName();
    StringRef s2 = v2->getName();

    StringRef &s = precedeString(vec, s1, s2);
    bool isFirst = (s == s1);

    Value *replacer = isFirst ? v1 : v2;
    Value *replacee = isFirst ? v2 : v1;
    return ChangeScheme(isEqual, replacer, replacee);
  }

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    SmallVector<StringRef> symbols;
    SmallDenseMap<StringRef, ChangeScheme> schemeMap;

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
              ChangeScheme scheme = createChangeScheme(isEqual, symbols, v1, v2);
              schemeMap.insert(make_pair(name, scheme));
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
          (schemeMap.find(brInst->getOperand(0)->getName()) != schemeMap.end())) {
        Value *cond = brInst->getOperand(0);
        ChangeScheme scheme = schemeMap[cond->getName()];

        // only icmp eq, replace
        if (scheme.isEqual) {
          unsigned idx = 0; 
          BasicBlock *next = term->getSuccessor(idx);
          BasicBlockEdge BBE(&BB, next);

          // if block edge (block -> true condition block) is dominates, replace
          for (BasicBlock &basicBlock: F)
            if (DT.dominates(BBE, &basicBlock))
              for (Instruction &inst: basicBlock)
                inst.replaceUsesOfWith(scheme.replacee, scheme.replacer);
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
