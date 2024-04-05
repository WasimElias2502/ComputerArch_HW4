/* 046267 Computer Architecture - HW #4 */

#include "core_api.h"
#include "sim_api.h"
#include <vector>

#include <stdio.h>

#define IdleCore -1

class Thread{

    tcontext context ;
    void cmdAdd(int dst_ind , int src1 , int src2);
    void cmdAddImmediate(int dst_ind , int src1 , int imm);
    void cmdSub(int dst_ind , int src1 , int src2);
    void cmdSubImmediate(int dst_ind , int src1 , int imm);
    void cmdStore(int dst , int src1 ,int src2 , bool isImmediate);
    void cmdLoad(int dst , int src1 ,int src2 , bool isImmediate);
    void cmdHalt();
    void cmdNop();

    int cyclesToWait ;
    int LoadLatency ;
    int StoreLatency ;
    bool isHalt ;
    uint32_t codeLine;


public:

    Thread();
    ~Thread() = default;

    void ExecuteInst(Instruction* instruction);
    bool ReadyToExecute();
    bool isHalted();
    void newCycle();
    uint32_t getCodeLine();



};

/////////////////////////// Implementation of Halt Class /////////////////////////////////////////////


                                                                                                                         //Have to Check to which value we should initialize the registers

Thread::Thread() :
 isHalt(false) , cyclesToWait(0) , codeLine(0){

    LoadLatency = SIM_GetLoadLat();
    StoreLatency = SIM_GetStoreLat();

    for(int i=0 ; i<REGS_COUNT ; i++)
        context.reg[i] = 0;
}

void Thread::ExecuteInst(Instruction* instruction){



    cmd_opcode myOpcode = instruction->opcode;
    if(myOpcode == CMD_NOP){
        cmdNop();
    }
    else if(myOpcode == CMD_ADD){
        cmdAdd(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm);
    }
    else if(myOpcode == CMD_ADDI){
        cmdAddImmediate(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm);
    }

    else if(myOpcode == CMD_SUB){
        cmdSub(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm);
    }

    else if(myOpcode == CMD_SUBI){
        cmdSubImmediate(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm);
    }
    else if(myOpcode == CMD_LOAD){
        cmdLoad(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm , instruction->isSrc2Imm);
    }
    else if(myOpcode == CMD_STORE){
        cmdStore(instruction->dst_index , instruction->src1_index , instruction->src2_index_imm , instruction->isSrc2Imm);
    }
    else if(myOpcode == CMD_HALT){
        cmdHalt();
    }


    //getNext code Line
                                                                                                                        // Check if we Should Align the address to 4 bytes (written in homework)
    codeLine += 32;
                                                                                                                        // check if none of the opcode
}


void Thread::cmdAdd(int dst_ind, int src1, int src2) {
    context.reg[dst_ind] = context.reg[src1] + context.reg[src2];
}

void Thread::cmdAddImmediate(int dst_ind, int src1, int imm) {
    context.reg[dst_ind] = context.reg[src1] + imm;
}

void Thread::cmdSub(int dst_ind, int src1, int src2) {
    context.reg[dst_ind] = context.reg[src1] - context.reg[src2];
}


void Thread::cmdSubImmediate(int dst_ind, int src1, int imm){
    context.reg[dst_ind] = context.reg[src1] - imm;
}

void Thread::cmdNop() {
    return;
}

void Thread::cmdHalt() {
    isHalt = true;
    return;
}

void Thread::cmdLoad(int dst, int src1, int src2, bool isImmediate) {

    int address = 0;

    //Calculating Address in Memory
    if(isImmediate == true){
        address = context.reg[src1] + src2 ;
    }
    else{
        address = context.reg[src1] + context.reg[src2] ;
    }

    //Load Data from memory[address] to tmp

    int32_t tmp = 0;

    SIM_MemDataRead((uint32_t)address, &tmp);                                                                             // Check if we Should Align the address to 4 bytes (written in homework)

    //Assining the dst register to tmp value

    context.reg[dst] = tmp;

    cyclesToWait = LoadLatency;

}

void Thread::cmdStore(int dst, int src1, int src2, bool isImmediate) {

    int address = 0;

    //Calculating Address in Memory
    if(isImmediate == true){
        address = context.reg[dst] + src2 ;
    }
    else{
        address = context.reg[dst] + context.reg[src2] ;
    }

    // Write The value to The Address
                                                                                                                         // Check if we Should Align the address to 4 bytes (written in homework)

    SIM_MemDataWrite((uint32_t)address , (int32_t)context.reg[src1]);

    cyclesToWait = StoreLatency;

}

bool Thread::ReadyToExecute(){
    return ( !isHalt && cyclesToWait == 0);
}

void Thread::newCycle() {
    if(cyclesToWait > 0) cyclesToWait -- ;
}


bool Thread::isHalted() {
    return isHalt;
}

uint32_t Thread::getCodeLine(){
    return codeLine;
}





//////////////////////Implmenattion of Core ////////////////////////////////////


class Core{

    int threadsNum ;
    std::vector<Thread> threadsList ;
    int numOfCycles ;
    int contextSwitchCycles ;
    int numOfInstructions ;

    virtual int Schedule() = 0;
    void newCycleinCore() ;
    bool Finish();


    public:

    Core() ;
    ~Core()  = default ;

    void Execute();

};


Core::Core() : threadsNum(threadsNum) , threadsList(threadsNum) , numOfInstructions(0) {
    threadsNum = SIM_GetThreadsNum();
    contextSwitchCycles = SIM_GetSwitchCycles();
}

void Core::newCycleinCore() {
    numOfCycles++;
    for (Thread element : threadsList) {
        element.newCycle();
    }
}

void Core::Execute(){

    Instruction *instruction = nullptr;

    while (!Finish()) {
        //Update that new Cycle is currently running
        newCycleinCore();

        // Find next Thread to execute command
        int nextThread = Schedule();


        // No Thread is Available
        if (nextThread == IdleCore) return;

        // There is Thread to work
        SIM_MemInstRead(threadsList[nextThread].getCodeLine(), instruction, nextThread);
        threadsList[nextThread].ExecuteInst(instruction);
        numOfInstructions++;
    }
}

bool Core::Finish() {
    for (Thread element : threadsList) {
        if(!element.isHalted()) return false;
    }

    return true;
}


/////////////////////// Mt_Blocked ///////////////////////////////////////////////////////////////////////








///////////////////////////Mt_Fine_Grained /////////////////////////////////////////////////////////////










////////////////////////// Home Work Functions //////////////////////////////////////////////////////
void CORE_BlockedMT() {
}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
