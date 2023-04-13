#include "DepMap.h"
#include "Instruction.h"

std::ifstream DepMap::ifile;
unordered_map<long, queue<Instruction*>*> DepMap::dep_map;

void DepMap::createDep(Instruction &I) {
    dep_map[I.address] = &I.depQ;
}

void DepMap::addDep(Instruction &I) { 
    auto i = I.deps.begin();
    while (i != I.deps.end()) {
        long address = *i;
        // if dependency exists add instruction to it
        if (dep_map.find(address) != dep_map.end()) {
            dep_map.at(address)->push(&I);
            i++;
        }
        else { // else dependency has already been fulfilled
            // remove dependency from instruction dependencies
            i = I.deps.erase(i); 
        }
    }
}

void DepMap::deleteDep(Instruction &I) {
    dep_map.erase(I.address);
}

void DepMap::print() {
    std::cout << "PRINTING DEP MAP" << std::endl;
    for (auto const &pair: dep_map) {
        std::cout << "{" << std::hex << pair.first << ": ";
        std::cout << "[";
        size_t size = pair.second->size();
        for(size_t i = 0; i < size; i++) {
            Instruction* I = pair.second->front();
            pair.second->pop();
            std::cout << I->address << " ";
            pair.second->push(I);
        }
        std::cout << "]";
        std::cout << "}\n";
    }
    std::cout << std::endl;
}