#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <unordered_map>
using std::unordered_map;

#include <vector>
using std::vector;

#include <queue>
using std::queue;
using std::priority_queue;

#include <fstream>
#include <sstream>

using std::string;
// Pipeline stages
enum Stage {None, IF, ID, EX, MEM, WB};

// Instruction type for each input trace instrucion (second CSV value of instruction)
enum IType {
    intI = 1,
    floatI = 2,
    branchI = 3,
    loadI = 4,
    storeI = 5
};

// two int cant go to EX in the same cycle
// two float cant go to EX in the same cycle
// two branch cant go to EX in the same cycle
// two load cant go to MEM in the same cycle
// two store cant go to MEM in the same cycle

// if instruction is branch, delays IF until brach finishes EX or in MEM=
// instruction cannot proceed to EX until all its dependencies are satisfied

// current use status of units
bool ALUOpen = true;
bool FPUOpen = true;
bool BranchOpen = true;
bool ReadOpen = true;
bool WriteOpen = true;

class Instruction {
    public:
        long address;
        int type; 
        vector<long> deps; // Instructions that I depend on 
        queue<Instruction*> depQ; // Instructions that depend on me

        Instruction(long, int, vector<long>, int);
        bool canMoveNext(Stage); // returns true if instruction can move to the next stage
        void FreeDepQ();

};

Instruction::Instruction(long address, int type, vector<long> deps, int depsSize) {
    this->address = address;
    this->type = type;
    this->deps.insert(this->deps.end(), &deps[0], &deps[depsSize]);
}
bool Instruction::canMoveNext(Stage next) {
    if (next == IF || next == ID || next == WB || next == None) return true;
    else if (next == EX) {
        if (type == intI) return ALUOpen && deps.size() == 0;
        else if (type == floatI) return FPUOpen && deps.size() == 0;
        else if (type == branchI) return BranchOpen && deps.size() == 0;
    }
    else if (next == MEM) {
        if (type == loadI) return ReadOpen && deps.size() == 0;
        else if (type == storeI) return WriteOpen && deps.size() == 0;
    }
    return false;
}

// Frees dependecies that were depending on this instruction (called during EX or MEM stage)
void Instruction::FreeDepQ() {
    while (depQ.size() != 0) {
        Instruction* I = depQ.front();
        depQ.pop();
        
        // get rid of fulfilled dependencies in other instructions
        I->deps.erase(std::remove(I->deps.begin(), I->deps.end(), address), I->deps.end());
    }
}

// a single pipeline
class PipeLine {
    public:
        Instruction* getIF() {return IList[0];}
        Instruction* getID() {return IList[1];}
        Instruction* getEX() {return IList[2];}
        Instruction* getMEM() {return IList[3];}
        Instruction* getWB() {return IList[4];}

        void moveTrace(); // move trace to IF (if possible)
        void moveIF(); // move IF to ID and calls moveTrace() if it succeeds.
        void moveID(); // move ID to EX and calls moveIF() if it succeeds.
        void moveEX(); // move EX to MEM and calls moveID() if it succeeds.
        void moveMEM(); // move MEM to WB and calls moveEX() if it succeeds.
        void moveWB(); // move WB out of system and calls moveMEM() if it succeeds.

        void tick(); // need to call every clock cycle to update pipeline
    private: 
                                // IF,  ID,  EX, MEM,  WB 
        Instruction* IList[5] = {NULL,NULL,NULL,NULL,NULL};
};
void PipeLine::tick() {
    moveWB();
}
void PipeLine::moveWB() {
    Instruction* I = getWB();
    if (I->canMoveNext(None)) {
        moveMEM();
    }
}
void PipeLine::moveMEM() {
    Instruction* I = getMEM();
    if (I->type == loadI || I->type == storeI) {
        I->FreeDepQ();
    }
    if (I->canMoveNext(WB)) {

        IList[4] = I;

        moveEX();
    }
}
void PipeLine::moveEX() {
    Instruction* I = getEX();
    
    if (I->type == branchI || I->type == intI || I->type == floatI) {
        I->FreeDepQ();
    }

    if (I->canMoveNext(MEM)) {
        IList[3] = I;

        moveID();
    }
}
void PipeLine::moveID() {
    Instruction* I = getID();

    if (I->canMoveNext(EX)) {
        IList[2] = I;

        moveIF();
    }
}
void PipeLine::moveIF() {
    Instruction* I = getID();

    if (I->canMoveNext(ID)) {
        IList[1] = I;

        moveTrace();
    }
}
void PipeLine::moveTrace() {
   
}




// W wide pipeline class for controlling the individual pipelines
class WPipeline {
    public:
        vector<PipeLine*> pipelines;
        long current_cycle = 0;
        int W;

        WPipeline(int);
        ~WPipeline();
        void tick(); // calls tick() on individual pipelines
};
WPipeline::WPipeline(int W) {
    this->W = W;
    for (int i = 0; i < W; i++) {
        pipelines.push_back(new PipeLine());
    }
}
WPipeline::~WPipeline() {
    while (pipelines.size() > 0) {
        delete pipelines.back();
        pipelines.pop_back();
    }
}
void WPipeline::tick() {
    current_cycle++;
    int length = std::min(current_cycle, (long)W);
    for (int i = 0; i < length; i++) {
        pipelines[i]->tick();
    }
}



/////////////////////////////////////
//        Global Variables         //
/////////////////////////////////////

long instTypeCount[] = {0,0,0,0,0}; // number of each instruction types ran [intI, floatI, branchI, loadI, storeI]


unordered_map<long, queue<Instruction*>*> DepMap; // Dependencies map 
/*  Every time a new instruction (lets call this A) is loaded (IF) its address will be saved as
    key along with an empty queue. Then when ever a new instruction (lets call this B) 
    that has a dependency for instruction "A" is loaded (IF) a pointer to instruction "B"
    will be stored in the queue that has address "A" as its key. Then when "A" finishes its
    EX or MEM (depending on type of instruction) we will remove all depenecies under key with 
    address of "A".
*/ 


/////////////////////////////////////
//           Functions             //
/////////////////////////////////////


// create new dependency tracker (See DepMap for better explanation)
void createDep(Instruction &I) {
    DepMap[I.address] = &I.depQ;
}

// add to an existing dependency list (See DepMap for better explanation)
void addDep(Instruction &I) { 
    vector<long>::iterator i = I.deps.begin();
    while (i != I.deps.end()) {
        long address = I.deps.at(0);
        // if dependency exists add instruction to it
        if (DepMap.find(address) != DepMap.end()) {
            DepMap.at(address)->push(&I);
            ++i;
        }
        else { // else dependency has already been fulfilled
            // remove dependency from instruction dependencies
            I.deps.erase(i++); 
        }
    }
}

// delete dependency tracker (See DepMap for better explanation)
// void deleteDep(long address) {
//     if (DepMap.count(address) == 0) {return;} // if key does not exist return

//     queue<Instruction*>* depender_Q = DepMap.at(address);
//     while (depender_Q->size() != 0) {
//         Instruction* I = depender_Q->front();
//         depender_Q->pop();
//         // get rid of fulfilled dependencies 
//         I->deps.erase(std::remove(I->deps.begin(), I->deps.end(), address), I->deps.end());
//     }
// }

// Read next instruction from file and return it
Instruction* readNextI(std::ifstream& ifile) {
    // reads file line by line
    // then reads line separated by comma and pushes to vector to read from
    for(int i = 0; i < 1; i++) {
        std::string trace_file_line;
        if(!std::getline(ifile, trace_file_line)) break;

        std::stringstream ss(trace_file_line);
        std::vector<std::string> vect;
        while(ss) {
            std::string s;
            if (!getline(ss, s, ',')) break;
            vect.push_back(s);
        }

        long address = strtol(vect[0].c_str(), NULL, 16);
        int type = stoi(vect[1]);
        vector<long> dependencies;
        for(std::size_t j = 0; j < vect.size(); j++) {
            long dep_address = strtol(vect[j].c_str(), NULL, 16);
            dependencies.push_back(dep_address);
        }

        Instruction* instruction = new Instruction(address, type, dependencies, 0);
        return instruction;
        // move instruction to IF
    }
}

// Main simulator function
void Simulation(std::ifstream& ifile, int startInst, int InstNum, int W) {
    WPipeline* P = new WPipeline(W);
    // instructions in WB retire and leave the pipeline (and the instruction window)
    for(int i = 0; i < 1000; i++) {
        P->tick();
    }
}

// example: ./proj sample_trace/compute_fp_1 10000000 1000000 2
int main(int argc, char* argv[]) {
    // input arguments trace file name, starting instruction, instruction count, W
    if(argc == 5){
		char* file = argv[1];
        int startInst = atoi(argv[2]);
        int InstNum = atoi(argv[3]);
        int W = atoi(argv[4]);

        std::ifstream ifile(file);
		// Error checks for input variables here, exit if incorrect input
        if (!ifile.is_open() || startInst < 0 || InstNum < 0 || W <= 0) {
            printf("Input Error. Terminating Simulation...\n");
            return 0;
        }

		// Start Simulation
		printf("Simulating with file = '%s' , startInst = %d, InstNum = %d, W = %d\n", file, startInst, InstNum, W); 
		Simulation(ifile, startInst, InstNum, W);
        ifile.close();
	}
	else printf("Insufficient number of arguments provided!\n");
	return 0;
}