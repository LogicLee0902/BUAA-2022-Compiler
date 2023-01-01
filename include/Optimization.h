//
// Created by Leo on 2022/11/16.
//

#ifndef COMPILER_OPTIMIZATION_H
#define COMPILER_OPTIMIZATION_H
#include <iostream>
#include <vector>
#include <memory>
#include "IRCodeGenerator.h"
#include "BasicBlock.h"

using namespace std;

class Optimization{
public:
    GenerateBlock run();
    void Print();
    void analyzeFunction();
    void analyzeLoop();
};


#endif //COMPILER_OPTIMIZATION_H
