#ifndef DEPMAP_H
#define DEPMAP_H

#pragma once

#include <unordered_map>
using std::unordered_map;

#include <queue>
using std::queue;

#include <fstream>
#include <sstream>

class Instruction;

enum Stage {None, IF, ID, EX, MEM, WB};

// Instruction type for each input trace instrucion (second CSV value of instruction)
enum IType {
    intI = 1,
    floatI = 2,
    branchI = 3,
    loadI = 4,
    storeI = 5
};

class DepMap
{
public:
    static std::ifstream ifile;

    static void createDep(Instruction &I);
    static void addDep(Instruction &I);
    static void deleteDep(Instruction &I);
    static void printDep();

    static unordered_map<long, queue<Instruction*>*> dep_map; // Dependencies map 
    /*  Every time a new instruction (lets call this A) is loaded (IF) its address will be saved as
    key along with an empty queue. Then when ever a new instruction (lets call this B) 
    that has a dependency for instruction "A" is loaded (IF) a pointer to instruction "B"
    will be stored in the queue that has address "A" as its key. Then when "A" finishes its
    EX or MEM (depending on type of instruction) we will remove all depenecies under key with 
    address of "A".
    */  
private:

};

#endif