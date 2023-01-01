//
// Created by Leo on 2022/10/23.
//

#ifndef COMPILER_MIPSGENERATOR_H
#define COMPILER_MIPSGENERATOR_H

#include "IRCodeGenerator.h"
#include "SymbolTable.h"
#include <iostream>
#include <memory>
#include "BasicBlock.h"


using namespace std;
using namespace irCode;

#define SYSCALL output<<"syscall"<<endl

class MipsGenerator {
private:
    fstream output;
    void load(shared_ptr<Object> &obj, const string& reg);
    void save(shared_ptr<Object> &obj, const string& reg);
    friend class Register;

public:

    explicit MipsGenerator(const string &out);
    void run();
    void dealPlusAndMinus(shared_ptr<IRCode> &ircode, const string& calc);
    void dealMulAndDiv(shared_ptr<IRCode> &ircode, const string& calc);
    void work(GenerateBlock &blockGenerator);
    // friend void rollBack(Register& reg);

};


#endif //COMPILER_MIPSGENERATOR_H
