#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#pragma once

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <vector>
using std::vector;

#include <queue>
using std::queue;

#include "DepMap.h"

class Pipeline;

class Instruction
{
public:
    long address;
    int type;   // Refer to IType
    unsigned long line_num; // The order which instruction arrived
    vector<long> deps; // Instructions that I depend on 
    queue<Instruction*> depQ; // Instructions that depend on me

    Instruction(long, int, unsigned long, vector<long>);
    bool canMoveNext(Stage, Pipeline*); // returns true if instruction can move to the next stage
    void FreeDepQ();

    void print(); // Debugging purposes

    static Instruction* readNextI(std::ifstream&, Pipeline*);
};

#endif