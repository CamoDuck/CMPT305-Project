#ifndef PIPELINE_H
#define PIPELINE_H

#pragma once

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
using std::vector;

#include "DepMap.h"

class Instruction;

class Pipeline
{
public:
    vector<vector<Instruction*>*> stages;
    unsigned long W;    // width of pipeline
    unsigned long inst_done = 0;    // number of instructions completed
    unsigned long inst_count; // max num of instructions to read

    unsigned long nextEXLineNum = 0; // next Instruction (by line) that is allowed to enter EX stage
    unsigned long nextMEMLineNum = 0; // next Instruction (by line) that is allowed to enter MEM stage

    unsigned long clock_cycle = 0; // current clock cycle
    long ITypeCount[5] = {0,0,0,0,0}; // number of each instruction types ran [intI, floatI, branchI, loadI, storeI]

    bool ALUOpen = true;
    bool FPUOpen = true;
    bool BranchOpen = true;
    bool ReadOpen = true;
    bool WriteOpen = true;

    bool BranchExist = false;

    int current_line = 0;

    Pipeline(unsigned long, unsigned long);
    ~Pipeline();

    vector<Instruction*>* getIF() {return stages.at(0);}
    vector<Instruction*>* getID() {return stages.at(1);}
    vector<Instruction*>* getEX() {return stages.at(2);}
    vector<Instruction*>* getMEM() {return stages.at(3);}
    vector<Instruction*>* getWB() {return stages.at(4);}

    void traceToIF(); // move trace to IF (if possible)
    void IFtoID(); // move IF to ID and calls traceToIF() if it succeeds.
    void IDtoEX(); // move ID to EX and calls IFtoID() if it succeeds.
    void EXtoMEM(); // move EX to MEM and calls IDtoEX() if it succeeds.
    void MEMtoWB(); // move MEM to WB and calls EXtoMEM() if it succeeds.
    void WBtoDone(); // move WB out of system and calls MEMtoWB() if it succeeds.

    void tick(); // need to call every clock cycle to update pipeline

    void print(); // for testing
};

#endif