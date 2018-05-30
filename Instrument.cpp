#include "Instrument.hpp"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <llvm/Support/raw_ostream.h>

#include <fstream>
#include <iostream>

int instcount=0;
bool Instrument::doInitialization(Module &module) {	
  return false;
}

bool Instrument::runOnModule(Module &module) {
  for(Module::iterator f = module.begin(), fe = module.end(); f != fe; f++) {
    runOnFunction(module, *f);
  }
  errs() << "  --" << instcount << " instruction(s) instrumented\n";
  return true;
}

static void createCallInst(Module &module, Instruction *instruction) {
  IRBuilder<> tmp( instruction );
  /***
  ** STEP-2: Retrieving the Array Size
  **/
 
  //convert the instruction into element pointer instruction 
  GetElementPtrInst* eleInstr = dyn_cast<GetElementPtrInst>( instruction );
  //get the element type to count the size of array
  
  PointerType * ptrtype = cast<PointerType>( eleInstr->getPointerOperandType() ); 
  Type* elemtype = ptrtype ->getElementType();
  unsigned array_size = (unsigned) dyn_cast<ArrayType>(elemtype)->getNumElements();
  


  /*
  errs() << "  Inside: " << *instruction << "\n";
  unsigned array_size = 0;
  */
  /***
  ** STEP-3: Retrieving the Index
  **/

  Value* index = ( isa<Instruction>( eleInstr->getOperand(2) ) ) ? 
    dyn_cast<Value>( eleInstr->getOperand(2) ):
    tmp.getInt64( (uint64_t) dyn_cast<ConstantInt>(eleInstr->getOperand(2))->getSExtValue() );

  /***
  ** STEP-4: Retrieving the Code Line Number
  **/

  ConstantInt* line = tmp.getInt64( (uint64_t) dyn_cast<DILocation>(instruction->getMetadata("dbg"))->getLine() );


  /***
  ** STEP-5: Creating and Passing the Arguments 
  ** Pack the retrieved arguments to pass to the Out-of-Bounds checking function
  ** and modify the check.c file accordingly.
  ** Note: feel free to not use the code that is commented out below
  **/
  std::vector<Value *> args;
  
  args.push_back(tmp.getInt64( (uint64_t) array_size ));
  args.push_back( index );
  args.push_back( line );
  args.push_back(tmp.CreateGlobalStringPtr(dyn_cast<DILocation>(instruction->getMetadata("dbg"))->getFilename()));

 
  Function * callee = module.getFunction("OutofBoundsError");  
  if (callee) {
    CallInst::Create(callee, args, "", instruction);
  }

  return;
}


bool Instrument::runOnFunction(Module &module, Function &function) {

  if (! function.isDeclaration()) {
    for(Function::iterator b = function.begin(), be = function.end(); b != be; b++) {
      for(BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
        /***
    	** STEP-1: Instrumenting the Array Access Instructions
	    ** Reimplement the following if-condition statement 
    	** to instrument the Out-of-Bounds check function after each array access instruction.
	    ***/
        Instruction* inst = dyn_cast<Instruction>(i);
	    if (isa<GetElementPtrInst>(inst)) { 
	      createCallInst(module, inst);
          
          /* Keep the following code to count and print the instrumented instructions. */
	      errs() << "  Instruction: " << *inst << "\n";
          instcount++;
	    }
      }
    }
  }
  return true;

}

void Instrument::getAnalysisUsage(AnalysisUsage &AU __attribute__((unused))) const {
  return;
}

char Instrument::ID = 0;
static const RegisterPass<Instrument> registration("instrument", "Instruments array accesses for out-of-bound checks");
