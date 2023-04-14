#include "Instruction.h"
#include "Pipeline.h"
#include "DepMap.h"

Instruction::Instruction(string address, int type, unsigned long line_num, vector<string> deps) {
    this->address = address;
    this->type = type;
    this->line_num = line_num;
    this->deps = deps;
}

bool Instruction::canMoveNext(Stage next, Pipeline* pipeline) {
    if (next == IF || next == ID || next == WB || next == None) return true;
    else if (next == EX) {
        if (deps.size() != 0) return false;
        else if (type == intI) return pipeline->ALUOpen;
        else if (type == floatI) return pipeline->FPUOpen;
        else if (type == branchI) return pipeline->BranchOpen;
        return true;
    }
    else if (next == MEM) {
        if (type == loadI) return pipeline->ReadOpen;
        else if (type == storeI) return pipeline->WriteOpen;
        return true;
    }
    return false;
}

void Instruction::freeDepQ() {
    while (depQ.size() != 0) {
        Instruction* I = depQ.front();
        depQ.pop();
        
        // get rid of fulfilled dependencies in other instructions
        I->deps.erase(std::remove(I->deps.begin(), I->deps.end(), address), I->deps.end());
    }
    DepMap::deleteDep(*this);
}

void Instruction::print() {
    std::cout << std::hex << address << std::dec << " "<< type << " " << deps.size() << std::endl;
    std::cout << "[";
    for (unsigned long i = 0; i < deps.size(); i++) {
        std::cout << deps[i] << ' ';
    }
    std::cout << "]"<< std::endl;
}

Instruction* Instruction::readNextI(std::ifstream& ifile, Pipeline* pipeline) {
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

        string address = vect[0].c_str();
        int type = stoi(vect[1]);
        vector<string> dependencies;
        for(std::size_t j = 2; j < vect.size(); j++) {
            string dep_address = vect[j].c_str();
            dependencies.push_back(dep_address);
        }

        Instruction* instruction = new Instruction(address, type, pipeline->current_line++, dependencies);
        return instruction;
    }
    return NULL;
}