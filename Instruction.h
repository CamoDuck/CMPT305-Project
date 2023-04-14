#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#pragma once

#include <algorithm>
#include <iostream>

#include <vector>
using std::vector;

#include <queue>
using std::queue;

#include <string>
using std::string;

#include "DepMap.h"

class Pipeline;

class Instruction
{
public:
    string address; // Hex address of instruction
    int type; // Refer to IType
    unsigned long line_num; // The order which instruction arrived
    vector<string> deps; // Instructions that I depend on 
    queue<Instruction*> depQ; // Instructions that depend on me

    /*
    address: address of instruction,
    type: instruction type (refer to IType),
    line_num: order which instruction arrived,
    deps: vector of instruction dependencies
    */
    Instruction(string, int, unsigned long, vector<string>);

    // Checks if there are any hazards or dependencies
    bool canMoveNext(Stage, Pipeline*);
    // Frees dependencies that were depending on this instruction (called during EX or MEM stage)
    void freeDepQ();
    // Debugging purposes
    void print();
    // Read next instruction from file and return it
    static Instruction* readNextI(std::ifstream&, Pipeline*);
};

#endif