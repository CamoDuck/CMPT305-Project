#include "Pipeline.h"
#include "Instruction.h"
#include "DepMap.h"

Pipeline::Pipeline(unsigned long W, unsigned long inst_count) {
    this->W = W;
    this->inst_count = inst_count;
    for (unsigned long i = 0; i < 5; i++) {
        stages.push_back(new vector<Instruction*>);
    }
}

Pipeline::~Pipeline() {
    while (stages.size() > 0) {
        vector<Instruction*>* curStage = stages.back();
        stages.pop_back();

        while (curStage->size() > 0) {
            delete curStage->back();
            curStage->pop_back();
        }

        delete curStage;
    }
}

void Pipeline::tick() {
    clock_cycle++;
    WBtoDone();
}

void Pipeline::WBtoDone() {
    vector<Instruction*>* IList = getWB();
    auto i = IList->begin();
    while (i != IList->end()) {
        Instruction* I = *i;
        if (I->canMoveNext(Stage::None, this)) {
            i = IList->erase(i);
            ITypeCount[I->type - 1]++;
            inst_done++;
            delete I;

            if (inst_done == inst_count) {return;} // stops pipeline when max instructions are complete
        }
        else {
            i++;
        }
    }

    MEMtoWB();
        
}

void Pipeline::MEMtoWB() {
    vector<Instruction*>* IList = getMEM();
    vector<Instruction*>* nextStage = getWB();
    auto i = IList->begin();
    while (i != IList->end()) {
        Instruction* I = *i;
        if (I->type == loadI || I->type == storeI) {
            I->freeDepQ();

            // free up units
            if (I->type == loadI) {
                ReadOpen = true;
            }
            else if (I->type == storeI) {
                WriteOpen = true;
            }
        }

        if (I->canMoveNext(Stage::WB, this) && nextStage->size() < W) {
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }


    EXtoMEM();
}

void Pipeline::EXtoMEM() {
    vector<Instruction*>* IList = getEX();
    vector<Instruction*>* nextStage = getMEM();
    int change = 1;
    while (change != 0) {
        change = 0;
        auto i = IList->begin();
        while (i != IList->end()) {
            Instruction* I = *i;
            if (I->type == branchI || I->type == intI || I->type == floatI) {
                I->freeDepQ();

                // free up units
                if (I->type == intI) {
                    ALUOpen = true;
                }
                else if (I->type == floatI) {
                    FPUOpen = true;
                }
                else if (I->type == branchI) {
                    BranchOpen = true;
                    BranchExist = false;
                } 
            }

            if (I->line_num == nextMEMLineNum && I->canMoveNext(Stage::MEM, this) && nextStage->size() < W) {
                nextMEMLineNum++;
                nextStage->push_back(I);
                i = IList->erase(i);
                change++;

                // occupy units
                if (I->type == loadI) {
                    ReadOpen = false;
                }
                else if (I->type == storeI) {
                    WriteOpen = false;
                }
            }
            else {
                i++;
            }
        }
    }
    
    IDtoEX();
}

void Pipeline::IDtoEX() {
    vector<Instruction*>* IList = getID();
    vector<Instruction*>* nextStage = getEX();
    int change = 1;
    while (change != 0) {
        change = 0;
        auto i = IList->begin();
        while (i != IList->end() && nextStage->size() < W) {
            Instruction* I = *i;
            if (I->line_num == nextEXLineNum && I->canMoveNext(Stage::EX, this)) {
                nextEXLineNum++;
                nextStage->push_back(I);
                i = IList->erase(i);
                change++;

                // occupy units
                if (I->type == intI) {
                    ALUOpen = false;
                }
                else if (I->type == floatI) {
                    FPUOpen = false;
                }
                else if (I->type == branchI) {
                    BranchOpen = false;
                }
            }
            else {
                i++;
            }
        }
    }
    IFtoID();
}

void Pipeline::IFtoID() {
    vector<Instruction*>* IList = getIF();
    vector<Instruction*>* nextStage = getID();
    auto i = IList->begin();
    while (i != IList->end() && nextStage->size() < W) {;
        Instruction* I = *i;
        if (I->canMoveNext(Stage::ID, this)) {
            nextStage->push_back(I);
            i = IList->erase(i);
        }
        else {
            i++;
        }
    }

    traceToIF();
}

void Pipeline::traceToIF() {
    vector<Instruction*>* nextStage = getIF();
    // if current branchs have finishes EX stage && IF stage has space
    while (!BranchExist && nextStage->size() < W) {
        Instruction* I = Instruction::readNextI(DepMap::ifile, this);
        // file end of line
        if (I == NULL) {
            break;
        }

        if (I->type == branchI) {
            BranchExist = true;
        }

        DepMap::addDep(*I);
        DepMap::createDep(*I);
        nextStage->push_back(I);
    }
}

void Pipeline::print() {
    std::cout << getIF()->size() << " " << getID()->size()<< " " << getEX()->size()<< " " << getMEM()->size()<< " " << getWB()->size() << std::endl;
    auto L = *getIF();
    std::cout << "-------------------IF" << std::endl;
    for (unsigned long i = 0; i < L.size(); i++) {
        L[i]->print();
    }
    L = *getID();
    std::cout << "-------------------ID" << std::endl;
    for (unsigned long i = 0; i < L.size(); i++) {
        L[i]->print();
    }
    L = *getEX();
    std::cout << "-------------------EX" << std::endl;
    for (unsigned long i = 0; i < L.size(); i++) {
        L[i]->print();
    }
    L = *getMEM();
    std::cout << "-------------------MEM" << std::endl;
    for (unsigned long i = 0; i < L.size(); i++) {
        L[i]->print();
    }
    L = *getWB();
    std::cout << "-------------------WB" << std::endl;
    for (unsigned long i = 0; i < L.size(); i++) {
        L[i]->print();
    }
    std::cout << std::endl;
}