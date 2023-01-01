//
// Created by Leo on 2022/11/26.
//

#ifndef COMPILER_BASICBLOCK_H
#define COMPILER_BASICBLOCK_H

#include <iostream>
#include <memory>
#include <vector>
#include "IRCodeGenerator.h"

using namespace std;
using namespace irCode;

class BasicBlock {
private:
    string id;
    vector<shared_ptr<IRCode> > irCodes;
public:
    BasicBlock(string id) : id(id) {
        irCodes.clear();
    }

    void addCode(shared_ptr<IRCode> & code) {
        irCodes.push_back(code);
    }

    vector<shared_ptr<IRCode> > & getIRCodes() {
        return irCodes;
    }
};

class GenerateBlock {
private:
    vector<shared_ptr<BasicBlock> > basicBlocks;
    // over of the decl
    int start;
public:
    GenerateBlock() {
        basicBlocks.clear();
    }

    void run();

    void setStart(int start_t) { this->start = start_t; }

    int getStart() { return start; }

    vector<shared_ptr<BasicBlock> > &getBasicBlocks() { return basicBlocks; }
};

#endif //COMPILER_BASICBLOCK_H
