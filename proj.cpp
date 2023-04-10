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

class Instruction {
    public:
        long address;
        int type; 
        int stage; // current pipeline stage
        vector<long> deps; // Instructions that I depend on 
        queue<Instruction*> depQ; // Instructions that depend on me

        Instruction(long, int, long [], int);
        void FreeDepQ();

};
Instruction::Instruction(long address, int type, long deps[], int depsSize) {
    this->address = address;
    this->type = type;
    this->stage = 0;
    this->deps.insert(this->deps.end(), &deps[0], &deps[depsSize]);
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
        void moveIF();  // move IF to ID and calls moveTrace() if it succeeds.
        void moveID(); // move ID to EX and calls moveIF() if it succeeds.
        void moveEX(); // 
        void moveMEM(); // 
        void moveWB(); // 

        void tick(); // need to call every clock cycle to update pipeline
    private:
        Instruction* IList[5] = {NULL,NULL,NULL,NULL,NULL};;
};
void PipeLine::tick() {
    moveWB();
}
void PipeLine::moveWB() {
    bool success = false;

    // check for dependencies etc

    if (success) {
        moveMEM();
    }
}
void PipeLine::moveMEM() {
    bool success = false;

    // check for dependencies etc

    if (success) {
        moveEX();
    }
}
void PipeLine::moveTrace() {}
void PipeLine::moveIF() {}
void PipeLine::moveID() {}
void PipeLine::moveEX() {}


/////////////////////////////////////
//        Global Variables         //
/////////////////////////////////////
long current_cycle = 0;
long instTypeCount[] = {0,0,0,0,0}; // number of each instruction types ran [intI, floatI, branchI, loadI, storeI]


unordered_map<long, queue<Instruction*>*> DepMap; // Dependencies map 
/*  Every time a new instruction (lets call this A) is loaded (IF) its address will be saved as
    key along with an empty queue. Then when ever a new instruction (lets call this B) 
    that has a dependency for instruction "A" is loaded (IF) a pointer to instruction "B"
    will be stored in the queue that has address "A" as its key. Then when "A" finishes its
    EX or MEM (depending on type of instruction) we will remove all depenecies under key with 
    address of "A".
*/ 

// current use status of units
bool ALUOpen = true;
bool FPUOpen = true;
bool BranchOpen = true;
bool ReadOpen = true;
bool WriteOpen = true;

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
// void readNextWI(std::ifstream& ifile, int W) {
//     // reads file line by line
//     // then reads line separated by comma and pushes to vector to read from
//     for(int i = 0; i < W; i++) {
//         std::string trace_file_line;
//         if(!std::getline(ifile, trace_file_line)) break;

//         std::stringstream ss(trace_file_line);
//         std::vector<std::string> vect;
//         while(ss) {
//             std::string s;
//             if (!getline(ss, s, ',')) break;
//             vect.push_back(s);
//         }

//         // for(std::size_t j = 0; j < vect.size(); j++)
//         //     std::cout << vect[j] << " ";

//         Instruction instruction = Instruction(strtol(vect[0], NULL, 16), stoi(vect[1]), 0, {}, 0);
//         // move instruction to IF
        
//         // std::cout << std::endl;
//     }
// }

// Instruction readNextI(std::ifstream& ifile) {
//     // reads file line by line
//     // then reads line separated by comma and pushes to vector to read from
//     vector<Instruction> data;
//     std::string trace_file_line;
//     std::getline(ifile, trace_file_line);

//     std::stringstream ss(trace_file_line);
//     std::vector<std::string> vect;
//     while(ss) {
//         std::string s;
//         if (!getline(ss, s, ',')) break;
//         vect.push_back(s);
//     }

//     Instruction instruction = Instruction(strtol(vect[0], NULL, 16), stoi(vect[1]), 0, {}, 0);
//     return instruction;
// }

// Main simulator function
void Simulation(std::ifstream& ifile, int startInst, int InstNum, int W) {
    // instructions in WB retire and leave the pipeline (and the instruction window)
    for(int i = 0; i < 1000; i++) {
        
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