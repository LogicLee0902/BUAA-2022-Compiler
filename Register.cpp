//
// Created by Leo on 2022/12/9.
//

#include "Register.h"
#include "MipsGenerator.h"

void Register::rollBack(MipsGenerator &mipsGenerator) {
    mipsGenerator.save(object, id);
    isFree = true;
}

void Register::storeValue(MipsGenerator &mipsGenerator, shared_ptr<Object> &obj) {
    mipsGenerator.load(obj, id);
    object = obj;
    isFree = false;
}

void Register::moveTo(shared_ptr<Register> &reg, shared_ptr<Object> &obj) {
    reg->setFree(false);
    reg->setObject(obj);
}