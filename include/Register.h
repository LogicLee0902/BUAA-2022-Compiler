//
// Created by Leo on 2022/12/9.
//

#ifndef COMPILER_REGISTER_H
#define COMPILER_REGISTER_H
#include <memory>
#include <utility>
#include "IRCodeGenerator.h"
#include "MipsGenerator.h"

using namespace std;
using namespace irCode;

class Register {
private:
    bool isFree;
    shared_ptr<Object> object;
    string id;
public:
     Register(string id) : id(std::move(id)) {
        isFree = true;
    }

    void setFree(bool free) {
        isFree = free;
    }

    bool getFree() {
        return isFree;
    }

    void setObject(shared_ptr<Object> &obj) {
        object = obj;
    }

    shared_ptr<Object>& getObject() {
        return object;
    }

    string getId() {
        return id;
    }

    void rollBack(MipsGenerator &mipsGenerator);

    void storeValue(MipsGenerator &mipsGenerator, shared_ptr<Object> &obj);

    static void moveTo(shared_ptr<Register> &reg,  shared_ptr<Object> &obj);

};


#endif //COMPILER_REGISTER_H
