#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

// For opening files
#include <iostream>
using std::iostream;
#include <fstream>
using std::fstream;
#include <unordered_map>
using std::unordered_map;

#include <vector>
using std::vector;

#include <queue>
using std::queue;
using std::priority_queue;

// Pipeline stages
enum Stage { None, IF, ID, EX, MEM, WB};

// Instruction type for each input trace instrucion (second CSV value of instruction)
enum IType {
    intI = 1,
    floatI = 2,
    branchI = 3,
    loadI = 4,
    storeI = 5
};

class Instruction {
    public:
        long address;
        IType type; 
        Stage stage = None; // current pipeline stage
        vector<long> deps;

        Instruction(long, IType, Stage, long [], int);
};
Instruction::Instruction(long address, IType type, Stage stage, long deps[], int depsSize) {
    this->address = address;
    this->type = type;
    this->stage = stage;
    this->deps.insert(this->deps.end(), &deps[0], &deps[depsSize]);
}

/////////////////////////////////////
//        Global Variables         //
/////////////////////////////////////
long current_cycle = 0;
long instTypeCount[] = {0,0,0,0,0}; // number of each instruction types ran [intI, floatI, branchI, loadI, storeI]


unordered_map<long, queue<Instruction*>> DepMap; // Dependencies map 
/*  Every time a new instruction (lets call this A) is loaded (IF) its address will be saved as
    key along with an empty queue. Then when ever a new instruction (lets call this B) 
    that has a dependency for instruction "A" is loaded (IF) a pointer to instruction "B"
    will be stored in the queue that has address "A" as its key. Then when "A" finishes its
    EX or MEM (depending on type of instruction) we will remove all depenecies under key with 
    address of "A".
*/ 

queue<Instruction> IF_Q; // instruction fetch queue
queue<Instruction> ID_Q; // instruction decode queue
queue<Instruction> WB_Q; // write back queue

queue<Instruction> Int_Q; // Integer ALU unit queue (used in EX stage)
queue<Instruction> Float_Q; // floating point unit queue (used in EX stage)
queue<Instruction> Branch_Q; // branch execution unit queue (used in EX stage)
queue<Instruction> Load_Q; // L1 read port queue (used in MEM stage)
queue<Instruction> Store_Q; // L1 write port queue (used in MEM stage)

/////////////////////////////////////
//           Functions             //
/////////////////////////////////////


// create new dependency tracker (See DepMap for better explanation)
void createDep(Instruction &I) {
    queue<Instruction*> Q = *(new queue<Instruction*>);
    DepMap[I.address] = Q;
}

// add to an existing dependency list (See DepMap for better explanation)
void addDep(Instruction &I) { 
    vector<long>::iterator i = I.deps.begin();
    while (i != I.deps.end()) {
        long address = I.deps.at(0);
        // if dependency exists add instruction to it
        if (DepMap.find(address) != DepMap.end()) {
            DepMap.at(address).push(&I);
            ++i;
        }
        else { // else dependency has already been fulfilled
            // remove dependency from instruction dependencies
            I.deps.erase(i++); 
        }
    }
}

// delete dependency tracker (See DepMap for better explanation)
void deleteDep(long address) {
    queue<Instruction*> depender_Q = DepMap.at(address);

    while (depender_Q.size() != 0) {
        Instruction* I = depender_Q.front();
        depender_Q.pop();

        // get rid of fulfilled dependencies 
        I->deps.erase(std::remove(I->deps.begin(), I->deps.end(), address), I->deps.end());
    }

    delete &depender_Q;
}

std::ifstream openFile(std::string name) {
    std::ifstream file(name);
    return file;
}
// Read next instruction from file and return it
Instruction readNextI() {
    throw;
}

bool TraceToIF() {
    return false;
}

void StartIF() {

}

bool IFToID() {
    return false;
}

void StartID() {}

bool IDToEX() {
    return false;
}

void StartEX() {}

bool EXtoMEM() {
    return false;
}

void StartMEM() {}

bool MEMToWB() {
    return false;
}

void StartWB() {}

bool WBToRetire() {
    return false;
}

// Main simulator function
void Simulation() {


}

int main(int argc, char* argv[]) {
    // input arguments trace file name, starting instruction, instruction count, W
    if(argc == 5){
		char* file = argv[1];
        int startInst = atoi(argv[2]);
        int InstNum = atoi(argv[3]);
        int W = atoi(argv[4]);
   
		// Error checks for input variables here, exit if incorrect input
        if (startInst < 0 || InstNum < 0 || W <= 0) {
            printf("Input Error. Terminating Simulation...\n");
            return 1;
        }

		// Start Simulation
		printf("Simulating with file = '%s' , startInst = %d, InstNum = %d, W = %d\n", file, startInst, InstNum, W); 
		Simulation();
        
	}
	else printf("Insufficient number of arguments provided!\n");
   
	return 0;
}