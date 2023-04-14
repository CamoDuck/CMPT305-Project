#ifndef PIPELINE_H
#define PIPELINE_H

#pragma once

#include <vector>
using std::vector;

#include <string>
using std::string;

#include "DepMap.h"

class Instruction;

class Pipeline
{
public:
    vector<vector<Instruction*>*> stages; // Each vector represents a pipeline for each stage of width W
    unsigned long W; // Width of pipeline
    unsigned long inst_done = 0; // Number of instructions completed
    unsigned long inst_count; // Max num of instructions to read, based on input

    unsigned long nextEXLineNum = 0; // Next Instruction (by line) that is allowed to enter EX stage
    unsigned long nextMEMLineNum = 0; // Next Instruction (by line) that is allowed to enter MEM stage

    unsigned long clock_cycle = 0; // Current clock cycle
    long ITypeCount[5] = {0,0,0,0,0}; // Number of each instruction types ran [intI, floatI, branchI, loadI, storeI]

    /*
    Current use status of units
    */
    bool ALUOpen = true;
    bool FPUOpen = true;
    bool BranchOpen = true;
    bool ReadOpen = true;
    bool WriteOpen = true;

    bool BranchExist = false;
    bool BranchInEx = false;

    int current_line = 0; // The line number of the instructions read in file input

    /*
    W: Width of pipeline
    inst_count: Max num of instructions to read
    */
    Pipeline(unsigned long, unsigned long);
    ~Pipeline();

    /*
    Returns the Instructions inside the respective stage of the pipeline
    */
    vector<Instruction*>* getIF() {return stages.at(0);}
    vector<Instruction*>* getID() {return stages.at(1);}
    vector<Instruction*>* getEX() {return stages.at(2);}
    vector<Instruction*>* getMEM() {return stages.at(3);}
    vector<Instruction*>* getWB() {return stages.at(4);}

    // Move Trace to IF (if possible)
    void traceToIF();
    // Move IF to ID and calls traceToIF() if it succeeds.
    void IFtoID();
    // Move ID to EX and calls IFtoID() if it succeeds.
    void IDtoEX();
    // Move EX to MEM and calls IDtoEX() if it succeeds.
    void EXtoMEM();
    // Move MEM to WB and calls EXtoMEM() if it succeeds.
    void MEMtoWB();
    // Move WB out of system and calls MEMtoWB() if it succeeds.
    void WBtoDone();
    // Is called every clock cycle to update pipeline
    void tick();
    // Debugging purposes
    void print();
};

#endif