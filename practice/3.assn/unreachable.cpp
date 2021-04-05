#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <set>
#include <queue>
#include <map>
using namespace llvm;

namespace {
class MyUnreachablePass : public PassInfoMixin<MyUnreachablePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    std::set<StringRef> unusedSet;
    std::queue<StringRef> blockQueue;
    std::map<StringRef, BasicBlock*> blockMap;
    BasicBlock &entry = F.getEntryBlock();
    blockQueue.push(entry.getName());

    for (BasicBlock &BB : F) {
      blockMap.insert(std::make_pair(BB.getName(), &BB));
      unusedSet.insert(BB.getName());
    }

    while (!blockQueue.empty()) {
      StringRef &s = blockQueue.front();
      blockQueue.pop();
      if (unusedSet.count(s)) {
        unusedSet.erase(s);
        BasicBlock &b = *blockMap[s];
        for (Instruction &I : b) {
          if (I.getOpcode() == Instruction::Br) {
            for (unsigned int i = 0; i < I.getNumOperands(); i++) {
              if (I.getNumOperands() == 1) {
                blockQueue.push(I.getOperand(0)->getName());
              } else {
                blockQueue.push(I.getOperand(1)->getName());
                blockQueue.push(I.getOperand(2)->getName());
              }
            }
          }
        }
      }
    }

    for (StringRef s : unusedSet) {
      outs() << s << "\n";
    }

    /*
    outs() << "Entry : " << F.getEntryBlock().getName() << "\n";
    outs() << "Front : " << F.front().getName() << "\n\n";
    for (BasicBlock &BB : F) {
      outs() << "Basic Block :" << BB.getName() << "\n";
      for (Instruction &I : BB) {
        for (unsigned int i = 0; i < I.getNumOperands(); i++) {
          Value *v = I.getOperand(i);
          outs() << "\t" << v->getValueID() << "\n";
        }
        outs() << "\t" << (I.getOpcode() == Instruction::Ret) << "\n";
      }
    }
    */
    return PreservedAnalyses::all();
  }
};
}

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "MyUnreachablePass", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "my-unreachable") {
            FPM.addPass(MyUnreachablePass());
            return true;
          }
          return false;
        }
      );
    }
  };
}
