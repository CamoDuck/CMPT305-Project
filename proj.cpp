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

#include <fstream>
#include <sstream>

using std::string;

using std::cout;
using std::endl;

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

class Instruction;
class PipeLine;

// two int cant go to EX in the same cycle
// two float cant go to EX in the same cycle
// two branch cant go to EX in the same cycle
// two load cant go to MEM in the same cycle
// two store cant go to MEM in the same cycle

// if instruction is branch, delays IF until brach finishes EX or in MEM=
// instruction cannot proceed to EX until all its dependencies are satisfied


/////////////////////////////////////
//        Global Variables         //
/////////////////////////////////////

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

std::ifstream ifile;

Instruction* readNextI(std::ifstream& ifile);
void createDep(Instruction &I);
void addDep(Instruction &I);
void deleteDep(Instruction &I);
void printDep();

/////////////////////////////////////
//             Classes             //
/////////////////////////////////////

class Instruction {
    public:
        long address;
        int type; 
        vector<long> deps; // Instructions that I depend on 
        queue<Instruction*> depQ; // Instructions that depend on me

        Instruction(long, int, vector<long>);
        bool canMoveNext(Stage); // returns true if instruction can move to the next stage
        void FreeDepQ();

        void print() {
            std::cout << std::hex << address << " "<< type << " " << deps.size() << std::endl;
            cout << "[";
            for (unsigned long i = 0; i < deps.size(); i++) {
                cout << deps[i] << ' ';
            }
            cout << "]"<< endl;
        }
};


Instruction::Instruction(long address, int type, vector<long> deps) {
    this->address = address;
    this->type = type;
    this->deps = deps;
}
bool Instruction::canMoveNext(Stage next) {
    //this->print();
    //cout << "stage: " << next-1 << endl;
    if (next == IF || next == ID || next == WB || next == None) return true;
    else if (next == EX) {
        if (type == intI) return ALUOpen && deps.size() == 0;
        else if (type == floatI) return FPUOpen && deps.size() == 0;
        else if (type == branchI) return BranchOpen && deps.size() == 0;
        return true;
    }
    else if (next == MEM) {
        if (type == loadI) return ReadOpen && deps.size() == 0;
        else if (type == storeI) return WriteOpen && deps.size() == 0;
        return true;
    }
    return false;
}

// Frees dependencies that were depending on this instruction (called during EX or MEM stage)
void Instruction::FreeDepQ() {
    while (depQ.size() != 0) {
        Instruction* I = depQ.front();
        depQ.pop();
        
        // get rid of fulfilled dependencies in other instructions
        I->deps.erase(std::remove(I->deps.begin(), I->deps.end(), address), I->deps.end());
    }
    deleteDep(*this);
}

// a single pipeline
class PipeLine {
    public:
        unsigned long W;
        vector<vector<Instruction*>*> stages;
        unsigned long IDone = 0;

        unsigned long clock_cycle = 0; // current clock cycle
        long ITypeCount[5] = {0,0,0,0,0}; // number of each instruction types ran [intI, floatI, branchI, loadI, storeI]

        PipeLine(unsigned long);
        ~PipeLine();

        vector<Instruction*>* getIF() {return stages.at(0);}
        vector<Instruction*>* getID() {return stages.at(1);}
        vector<Instruction*>* getEX() {return stages.at(2);}
        vector<Instruction*>* getMEM() {return stages.at(3);}
        vector<Instruction*>* getWB() {return stages.at(4);}

        void moveTrace(); // move trace to IF (if possible)
        void moveIF(); // move IF to ID and calls moveTrace() if it succeeds.
        void moveID(); // move ID to EX and calls moveIF() if it succeeds.
        void moveEX(); // move EX to MEM and calls moveID() if it succeeds.
        void moveMEM(); // move MEM to WB and calls moveEX() if it succeeds.
        void moveWB(); // move WB out of system and calls moveMEM() if it succeeds.

        void tick(); // need to call every clock cycle to update pipeline
        
};
PipeLine::PipeLine(unsigned long W) {
    this->W = W;
    for (unsigned long i = 0; i < 5; i++) {
        stages.push_back(new vector<Instruction*>);
    }
}
PipeLine::~PipeLine() {
    while (stages.size() > 0) {
        delete stages.back();
        stages.pop_back();
    }
}

void PipeLine::tick() {
   // std::cout << "ran1" << std::endl; 
    clock_cycle++;
    moveWB();
}
void PipeLine::moveWB() {
   // std::cout << "ran2" << std::endl; 
    vector<Instruction*>* IList = getWB();
    auto i = IList->begin();
    while (i != IList->end()) {
        Instruction* I = *i;
        if (I->canMoveNext(None)) {
            i = IList->erase(i);
            ITypeCount[I->type - 1]++;
            IDone++;
            delete I;
        }
        else {
            i++;
        }
    }

    moveMEM();
        
}
void PipeLine::moveMEM() {
 //   std::cout << "ran3" << std::endl; 
    vector<Instruction*>* IList = getMEM();
    vector<Instruction*>* nextStage = getWB();
    auto i = IList->begin();
    while (i != IList->end() && nextStage->size() < W) {
        Instruction* I = *i;
        if (I->type == loadI || I->type == storeI) {
            if (this->IDone > 0) {
                cout << endl << "FREE:";
                I->print();
                cout << endl;
            }
            I->FreeDepQ();
        }

        if (I->canMoveNext(WB)) {
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }
    moveEX();
}
void PipeLine::moveEX() {
   // std::cout << "ran4" << std::endl; 
    vector<Instruction*>* IList = getEX();
    vector<Instruction*>* nextStage = getMEM();
    auto i = IList->begin();
    while (i != IList->end() && nextStage->size() < W) {
        Instruction* I = *i;
        //I->print();
        if (I->type == branchI || I->type == intI || I->type == floatI) {
            if (this->IDone > 0) {
                cout << endl << "FREE:";
                I->print();
                cout << endl;
            }
            I->FreeDepQ();
        }

        if (I->canMoveNext(MEM)) {
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }
    
    moveID();
}
void PipeLine::moveID() {
   // std::cout << "ran5" << std::endl; 
    vector<Instruction*>* IList = getID();
    vector<Instruction*>* nextStage = getEX();
    auto i = IList->begin();
    while (i != IList->end() && nextStage->size() < W) {
        //std::cout << "ran5in" << std::endl;
        Instruction* I = *i;
        //I->print();
        if (I->canMoveNext(EX)) {
            //std::cout << "ran5in2" << std::endl;
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }
    moveIF();
}
void PipeLine::moveIF() {
   // std::cout << "ran6" << std::endl; 
    vector<Instruction*>* IList = getIF();
    vector<Instruction*>* nextStage = getID();
    auto i = IList->begin();
    while (i != IList->end() && nextStage->size() < W) {
        //std::cout << "ran6in" << std::endl;
        Instruction* I = *i;
        if (I->canMoveNext(ID)) {
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }

    moveTrace();
}
void PipeLine::moveTrace() {
  //  std::cout << "ran7" << std::endl; 
    vector<Instruction*>* nextStage = getIF();
    while (nextStage->size() < W) {
        //std::cout << "ran7in" << std::endl;
        Instruction* I = readNextI(ifile);
        addDep(*I);
        createDep(*I);
        nextStage->push_back(I);
    }
    // if (this->IDone > 0) {
    //     cout << getIF()->size() << " " << getID()->size()<< " " << getEX()->size()<< " " << getMEM()->size()<< " " << getWB()->size() << endl;

    //     auto L = *getIF();
    //     cout << "IF" << endl;
    //     for (unsigned long i = 0; i < L.size(); i++) {
    //         L[i]->print();
    //     } 
    //      L = *getID();
    //     cout << "ID" <<endl;
    //     for (unsigned long i = 0; i < L.size(); i++) {
    //         L[i]->print();
    //     } 
    //     cout << endl;
    // }
}


/////////////////////////////////////
//           Functions             //
/////////////////////////////////////


// create new dependency tracker (See DepMap for better explanation)
void createDep(Instruction &I) {
    DepMap[I.address] = &I.depQ;
}

// add to an existing dependency list (See DepMap for better explanation)
void addDep(Instruction &I) { 
    // cout << "HI I AM " << std::hex << I.address << " AND MY DEPENDENCIES ARE ";
    // for(size_t i = 0; i < I.deps.size(); i++){
    //     cout << I.deps[i] << " ";
    // }
    // cout << endl;

    auto i = I.deps.begin();
    while (i != I.deps.end()) {
        long address = *i;
        // if dependency exists add instruction to it
        if (DepMap.find(address) != DepMap.end()) {
            DepMap.at(address)->push(&I);
            i++;
        }
        else { // else dependency has already been fulfilled
            // remove dependency from instruction dependencies
            i = I.deps.erase(i); 
        }
    }

    // printDep();
}

void deleteDep(Instruction &I) {
    DepMap.erase(I.address);
}

void printDep() {
    cout << "PRINTING DEP MAP" << endl;
    for (auto const &pair: DepMap) {
        std::cout << "{" << std::hex << pair.first << ": ";
        cout << "[";
        size_t size = pair.second->size();
        for(size_t i = 0; i < size; i++) {
            Instruction* I = pair.second->front();
            pair.second->pop();
            cout << I->address << " ";
            pair.second->push(I);
        }
        cout << "]";
        cout << "}\n";
    }
    cout << endl;
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

        // for (unsigned long j = 0; j < vect.size(); j++) {
        //     std::cout << vect[j] << " ";
        // }
        // std::cout << std::endl;

        long address = strtol(vect[0].c_str(), NULL, 16);
        int type = stoi(vect[1]);
        vector<long> dependencies;
        for(std::size_t j = 2; j < vect.size(); j++) {
            long dep_address = strtol(vect[j].c_str(), NULL, 16);
            dependencies.push_back(dep_address);
        }

        Instruction* instruction = new Instruction(address, type, dependencies);
        return instruction;
        // move instruction to IF
    }
    return NULL;
}

// Main simulator function
void Simulation(std::ifstream& ifile, int startInst, int InstNum, int W) {
    PipeLine* P = new PipeLine(W);
    // instructions in WB retire and leave the pipeline (and the instruction window)
    //2778649 2778686
    while (P->IDone < 14) { 
        P->tick();
    }
    std::cout << "clock cycles: " << P->clock_cycle << std::endl;
    printf("Instruction Types ran : int = %li, float = %li, branch = %li, load = %li, store = %li\n", P->ITypeCount[0], P->ITypeCount[1], P->ITypeCount[2], P->ITypeCount[3], P->ITypeCount[4]);
}

// example: ./proj sample_trace/compute_fp_1 10000000 1000000 2
int main(int argc, char* argv[]) {
    // input arguments trace file name, starting instruction, instruction count, W
    if(argc == 5){
		char* file = argv[1];
        int startInst = atoi(argv[2]);
        int InstNum = atoi(argv[3]);
        int W = atoi(argv[4]);

        ifile.open(file);
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