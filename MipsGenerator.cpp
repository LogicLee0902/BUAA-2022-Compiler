//
// Created by Leo on 2022/10/23.
//
#include <iostream>
#include <stack>
#include <queue>
#include "include/BigInteger.h"
#include "include/MipsGenerator.h"
#include "include/Register.h"
#include "include/BasicBlock.h"

using namespace std;
using namespace irCode;


stack<shared_ptr<Object> > funcParams;

MipsGenerator::MipsGenerator(const string &out) {
    cout << "Outputfile: " << out << endl;
    this->output.open(out, ios::out);
}

vector<shared_ptr<Register> > tempRegs;
vector<shared_ptr<Register> > GlobalRegs;
int freeTempCnt, freeGlobalCnt;

void MipsGenerator::run() {
    cout << "-----------" << endl;
    cout << "MipsGenerator" << endl;
    output << ".data" << endl;
    for (auto &attributeData: attributeDataList) {
        output << attributeData.label << ": .asciiz " << "  \"" << attributeData.content << "\"" << endl;
    }
    output << R"(endl: .asciiz "\n")" << endl;
    output << endl << ".text" << endl;
    for (auto &ircode: IRCodeList) {
        //cout << operatorName[ircode->getOp()] << endl;
        switch (ircode->getOp()) {
            case PrintIntOT: {
                load(ircode->getObj(0), "$a0");
                output << "li $v0, 1" << endl;
                SYSCALL;
                break;
            }
            case PrintOT: {
                output << "la $a0, " << ircode->getObj(0)->getName() << endl;
                output << "li $v0, 4" << endl;
                SYSCALL;
                break;
            }
            case GetIntOT: {
                output << "li $v0, 5" << endl;
                SYSCALL;
                save(ircode->getObj(0), "$v0");
                break;
            }
            case VarOT: {
                if (ircode->getObj(2) != nullptr) {
                    load(ircode->getObj(2), "$t0");
                    save(ircode->getObj(1), "$t0");
                }
                break;
            }
            case ConstOT: {
                load(ircode->getObj(2), "$t0");
                save(ircode->getObj(1), "$t0");
                break;
            }
            case FuncOT: {
                shared_ptr<FuncIRCode> &func = ircode->getObj(1)->getFunc();
                output << ("function_" + ircode->getObj(1)->getName()) << ":" << endl;
                if (func->getName() == "main") {
                    output << "move $fp, $sp" << endl;
                    output << "addi $sp, $sp, " << -(4 * func->getOffset() + 8) << endl;
                }
                break;
            }
            case PushOT: {
                funcParams.push(ircode->getObj(0));
                break;
            }
            case CallOT: {
                shared_ptr<Object> object = ircode->getObj(0);
                shared_ptr<FuncIRCode> func = object->getFunc();
                vector<shared_ptr<VarIRCode> > &exps = func->getExps();
                int num = (int) exps.size();
                int offset = (4 * func->getOffset() + 8);
                string name = func->getName();
                output << "addi $s0, $sp, " << -offset << endl;
                while (num > 0) {
                    shared_ptr<Object> param = funcParams.top();
                    // param is real, exps is set
                    funcParams.pop();
                    shared_ptr<VarIRCode> &var = param->getVar();
                    if (var != nullptr and var->getDimension() not_eq 0
                        and (var->getDimension() == exps[num - 1]->getDimension()
                             or (var->getDimension() == 2 and exps[num - 1]->getDimension() == 1))) {
                        int domain = var->getDomain();
                        string t_reg = domain == 0 ? "$gp" : "$sp";
                        if (var->getIsArrayPara()) {
                            // the parameter used as the other function's parameter
                            if (var->getDimension() == exps[num - 1]->getDimension()) {
                                output << "lw $t0, " << var->getOffset() * 4 << "($sp)" << endl;
                                output << "sw $t0, " << num * 4 << "($s0)" << endl;
                            } else {
                                // a is 2D and use as 1D, int fun(int a[][3]) {func(a[0]);}
                                output << "lw $t0, " << var->getOffset() * 4 << "($sp)" << endl;
                                if (param->getCategory() == 4) {
                                    int index = param->getNumericIndex() * var->getSpectrum()[1] * 4;
                                    output << "subi $t0, $t0, " << index << endl;
                                } else {
                                    load(param->getIndex(), "$t1");
                                    output << "li $t2, " << var->getSpectrum()[1] * 4 << endl;
                                    output << "mult $t1, $t2" << endl;
                                    output << "mflo $t2" << endl;
                                    output << "subu $t0, $t0, $t2" << endl;
                                }
                                output << "sw $t0, " << num * 4 << "($s0)" << endl;
                            }
                        } else if (exps[num - 1]->getDimension() == var->getDimension()) {
                            exps[num - 1]->setOffset(var->getOffset());
                            output << "li $t0, " << var->getOffset() * 4 << endl;
                            output << "addu $t0, " << t_reg << ", $t0" << endl;
                            output << "sw $t0, " << 4 * num << "($s0)" << endl;
                        } else if (exps[num - 1]->getDimension() == 1 and var->getDimension() == 2) {
                            if (param->getCategory() == 4) {
                                exps[num - 1]->setOffset(
                                        var->getOffset() - param->getNumericIndex() * var->getSpectrum()[1]);
                                output << "li $t0, " << exps[num - 1]->getOffset() * 4 << endl;
                                output << "addu $t0, " << t_reg << ", $t0" << endl;
                                output << "sw $t0, " << 4 * num << "($s0)" << endl;
                            } else {
                                load(param->getIndex(), "$t0");
                                output << "li $t1, " << var->getSpectrum()[1] * 4 << endl;
                                output << "mult $t0, $t1" << endl;
                                output << "mflo $t2" << endl;
                                output << "li $t1, " << var->getOffset() * 4 << endl;
                                output << "subu $t0, $t1, $t2" << endl;
                                output << "addu $t0, $t0, " << t_reg << endl;
                                output << "sw $t0, " << 4 * num << "($s0)" << endl;
                            }
                        }
                    } else {
                        load(param, "$t0");
                        output << "sw $t0, " << 4 * num << "($s0)" << endl;
                    }
                    --num;
                }
                output << "sw $ra, ($sp)" << endl;
                output << "sw $fp, -4($sp)" << endl;
                output << "addi $sp, $sp, " << -offset << endl;
                output << "addi $fp, $sp," << offset << endl;
                output << "jal function_" << name << endl;
                output << "addi $sp, $sp, " << offset << endl;
                output << "lw $fp, -4($sp)" << endl;
                output << "lw $ra, ($sp)" << endl;
                break;
            }
            case ReturnOT: {
                if (ircode->getObj(0) != nullptr) {
                    load(ircode->getObj(0), "$v0");
                }
                output << "jr $ra" << endl;
                break;
            }
            case ExitOT: {
                output << "li $v0, 10" << endl;
                SYSCALL;
                break;
            }
            case PLUSOT: {
                if (ircode->getObj(1)->getCategory() == 5) {
                    load(ircode->getObj(2), "$t1");
                    output << "addi $t2, $t1, " << ircode->getObj(1)->getNum() << endl;
                    save(ircode->getObj(0), "$t2");
                } else if (ircode->getObj(2)->getCategory() == 5) {
                    load(ircode->getObj(1), "$t0");
                    output << "addi $t2, $t0, " << ircode->getObj(2)->getNum() << endl;
                    save(ircode->getObj(0), "$t2");
                } else {
                    load(ircode->getObj(1), "$t0");
                    load(ircode->getObj(2), "$t1");
                    output << "addu $t2, $t0, $t1" << endl;
                    save(ircode->getObj(0), "$t2");
                }
                break;
            }
            case BITANDOT: {
                if (ircode->getObj(1)->getCategory() == 5) {
                    load(ircode->getObj(2), "$t1");
                    output << "andi $t2, $t1, " << ircode->getObj(1)->getNum() << endl;
                    save(ircode->getObj(0), "$t2");
                } else if (ircode->getObj(2)->getCategory() == 5) {
                    load(ircode->getObj(1), "$t0");
                    output << "andi $t2, $t0, " << ircode->getObj(2)->getNum() << endl;
                    save(ircode->getObj(0), "$t2");
                } else {
                    load(ircode->getObj(1), "$t0");
                    load(ircode->getObj(2), "$t1");
                    output << "and $t2, $t0, $t1" << endl;
                    save(ircode->getObj(0), "$t2");
                }
                break;
            }
            case MINUSOT: {
                if (ircode->getObj(2)->getCategory() == 5) {
                    load(ircode->getObj(1), "$t0");
                    output << "subi $t2, $t0, " << ircode->getObj(2)->getNum() << endl;
                    save(ircode->getObj(0), "$t2");
                } else {
                    load(ircode->getObj(1), "$t0");
                    load(ircode->getObj(2), "$t1");
                    output << "subu $t2, $t0, $t1" << endl;
                    save(ircode->getObj(0), "$t2");
                }
                break;
            }
            case MULTOT: {
                load(ircode->getObj(1), "$t0");
                load(ircode->getObj(2), "$t1");
                output << "mult $t0, $t1" << endl;
                output << "mflo $t2" << endl;
                save(ircode->getObj(0), "$t2");
                break;
            }
            case MULTHOT: {
                load(ircode->getObj(1), "$t0");
                load(ircode->getObj(2), "$t1");
                output << "mult $t0, $t1" << endl;
                output << "mfhi $t2" << endl;
                save(ircode->getObj(0), "$t2");
                break;
            }
            case DIVOT: {
//#ifndef OPTIMIZE_DIVIDE
                load(ircode->getObj(1), "$t0");
                load(ircode->getObj(2), "$t1");
                output << "div $t0, $t1" << endl;
                output << "mflo $t2" << endl;
                save(ircode->getObj(0), "$t2");
//#else

//#endif
                break;
            }
            case MODOT: {
                load(ircode->getObj(1), "$t0");
                load(ircode->getObj(2), "$t1");
                output << "div $t0, $t1" << endl;
                output << "mfhi $t2" << endl;
                save(ircode->getObj(0), "$t2");
                break;
            }
            case AssignOT: {
                if (ircode->getObj(1)->getName() == "RET")
                    // return value
                    output << "move $t0, $v0" << endl;
                else
                    load(ircode->getObj(1), "$t0");
                save(ircode->getObj(0), "$t0");
                break;
            }
            case MainOT: {
                output << endl << "j function_main" << endl;
                break;
            }
            case BGEOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "bge $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case BGTOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "bgt $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case BLEOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "ble $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case BLTOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "blt $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case BEQOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "beq $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case BNEOT: {
                load(ircode->getObj(0), "$t0");
                load(ircode->getObj(1), "$t1");
                output << "bne $t0, $t1, " << ircode->getObj(2)->getName() << endl;
                break;
            }
            case JumpOT: {
                output << "j " << ircode->getObj(0)->getName() << endl;
                break;
            }
            case LabelOT: {
                output << ircode->getObj(0)->getName() << ":" << endl;
                break;
            }
            case NOTOT: {
                load(ircode->getObj(2), "$t0");
                output << "seq $t0 $t0, $zero" << endl;
                save(ircode->getObj(0), "$t0");
                break;
            }
            case SNEOT:
            case SEQOT:
            case SLTOT:
            case SLEOT:
            case SGTOT:
            case SGEOT:
            case ANDOT:
            case OROT:
            case SLLVOT:
            case SRLVOT:
            case SRAVOT: {
                load(ircode->getObj(1), "$t0");
                load(ircode->getObj(2), "$t1");
                output << operatorName[ircode->getOp()] << " $t0, $t0, $t1" << endl;
                save(ircode->getObj(0), "$t0");
                break;
            }
            case SRAOT:
            case SLLOT:
            case SRLOT: {
                load(ircode->getObj(1), "$t0");
                output << operatorName[ircode->getOp()] << " $t0, $t0, " << ircode->getObj(2)->getNum() << endl;
                save(ircode->getObj(0), "$t0");
                break;
            }
            case ParamOT:
            case ArrayOT: {
                break;
            }
            default: {
                cout << "error:unknown" << endl;
                cout << operatorName[ircode->getOp()] << endl;
                break;
            }
        }
    }
}


// use $t8, $t9 for arrays to calculate address
void MipsGenerator::load(shared_ptr<Object> &obj1, const string& reg) {
    if (obj1->getCategory() == 5) {
        output << "li " << reg << ", " << obj1->getNum() << endl;
        return;
    } else if (obj1->getCategory() == 6) {
        output << "li " << reg << ", " << obj1->getLongNum() << endl;
        return;
    }
    shared_ptr<VarIRCode> var = obj1->getVar();
    if (var->getDimension() == 0) {
        int address = var->getOffset() * 4;
        string pointeer_reg = (var->getDomain() == 0) ? "$gp" : "$sp";
        output << "lw " << reg << ", " << address << "(" << pointeer_reg << ")" << endl;

    } else {
        int address = var->getOffset() * 4;
        string pointeer_reg = (var->getDomain() == 0) ? "$gp" : "$sp";
        if (obj1->getCategory() == 3) {
            // a[n]
            shared_ptr<Object> &index = obj1->getIndex();
            load(index, "$t9");
            // t9 for index
            if (var->getIsArrayPara()) {
                output << "lw $t8, " << (var->getOffset()) * 4 << "($sp)" << endl;
            } else {
                output << "addi $t8, " << pointeer_reg << ", " << address << endl;
            }
            output << "sll $t9, $t9, 2" << endl;
            output << "subu $t9, $t8, $t9" << endl;
            output << "lw " << reg << ", 0($t9)" << endl;
        } else if (obj1->getCategory() == 4) {
            // a[10]
            int numericIndex = obj1->getNumericIndex();
            if (var->getIsArrayPara()) {
                output << "lw $t8, " << (var->getOffset()) * 4 << "($sp)" << endl;
                output << "lw " << reg << ", " << -numericIndex * 4 << "($t8)" << endl;
            } else {
                address -= numericIndex * 4;
                output << "lw " << reg << ", " << address << "(" << pointeer_reg << ")" << endl;
            }
        }
    }
}

void MipsGenerator::save(shared_ptr<Object> &obj, const string& reg) {
    shared_ptr<VarIRCode> var = obj->getVar();
    if (var->getDimension() == 0) {
        int address = var->getOffset() * 4;
        string pointeer_reg = var->getDomain() == 0 ? "$gp" : "$sp";
        output << "sw " << reg << ", " << address << "(" << pointeer_reg << ")" << endl;
    } else {
        int address = var->getOffset() * 4;
        string pointeer_reg = (var->getDomain() == 0) ? "$gp" : "$sp";
        if (obj->getCategory() == 3) {
            // a[n]
            shared_ptr<Object> &index = obj->getIndex();
            load(index, "$t9");
            // t9 for index
            if (var->getIsArrayPara()) {
                output << "lw $t8, " << (var->getOffset()) * 4 << "($sp)" << endl;
            } else {
                output << "addi $t8, " << pointeer_reg << ", " << address << endl;
            }
            output << "sll $t9, $t9, 2" << endl;
            output << "subu $t9, $t8, $t9" << endl;
            output << "sw " << reg << ", 0($t9)" << endl;
        } else if (obj->getCategory() == 4) {
            // a[10]
            int numericIndex = obj->getNumericIndex();
            if (var->getIsArrayPara()) {
                output << "lw $t8, " << (var->getOffset()) * 4 << "($sp)" << endl;
                output << "sw " << reg << ", " << -numericIndex * 4 << "($t8)" << endl;
            } else {
                address -= numericIndex * 4;
                output << "sw " << reg << ", " << address << "(" << pointeer_reg << ")" << endl;
            }
        }
    }
}

map<shared_ptr<VarIRCode>, int> var2reg;
int pop_order = 0;

void InitRegs() {
    for (int i = 0; i < 8; ++i) {
        shared_ptr<Register> reg = make_shared<Register>("$t" + to_string(i));
        tempRegs.push_back(reg);
        freeTempCnt += 1;
    }
}

void rollBaclRegs(MipsGenerator &mipsGenerator) {
    for (int i = 0; i < 8; ++i) {
        if (!tempRegs[i]->getFree()) {
            //cout << tempRegs[i]->getObject()->getName() << endl;
            tempRegs[i]->rollBack(mipsGenerator);
        }
    }
    var2reg.clear();
    freeTempCnt = 8;
    //cout << "Rollback REG OVer" << endl;
}

void loadTempVar(MipsGenerator &mipsGenerator, shared_ptr<Object> &obj) {
    if (freeTempCnt > 0) {
        tempRegs[8 - freeTempCnt]->storeValue(mipsGenerator, obj);
        //var2reg.erase(obj->getVar());
        var2reg[obj->getVar()] = 8 - freeTempCnt;
        freeTempCnt -= 1;
    } else {
        var2reg.erase(tempRegs[pop_order]->getObject()->getVar());
        tempRegs[pop_order]->rollBack(mipsGenerator);
        tempRegs[pop_order]->storeValue(mipsGenerator, obj);
        var2reg[obj->getVar()] = pop_order;
        pop_order = (pop_order + 1) % 8;
    }
}

int moveTempVar(MipsGenerator &mipsGenerator, int idx, int idx2, fstream &output, shared_ptr<Object> &obj) {
    if (idx2 == -1) {
        if (freeTempCnt > 0) {
            output << "move " << tempRegs[8 - freeTempCnt]->getId() << ", " << tempRegs[idx]->getId()
                   << endl;
            tempRegs[idx]->moveTo(tempRegs[8 - freeTempCnt], obj);
            freeTempCnt -= 1;
            return 7 - freeTempCnt;
        } else {
            // accidentally same as the one to be moved, go to the next
            if (pop_order == idx)  pop_order = (pop_order + 1) % 8;
            tempRegs[pop_order]->rollBack(mipsGenerator);
            tempRegs[idx]->moveTo(tempRegs[pop_order], obj);
            output << "move " << tempRegs[pop_order]->getId() << ", " << tempRegs[idx]->getId() << endl;
            int last = pop_order;
            pop_order = (pop_order + 1) % 8;
            return last;
        }
    } else if (idx != idx2){
        output << "move " << tempRegs[idx2]->getId() << ", " << tempRegs[idx]->getId() << endl;
        tempRegs[idx]->moveTo(tempRegs[idx2], obj);
    }
    return idx2;
}

void MipsGenerator::dealPlusAndMinus(shared_ptr<IRCode>& ircode, const string& calc) {
    int numIndex= -1, varIndex = -1;
    if (calc!="sub" and ircode->getObj(1)->getCategory() == 5) {
        numIndex = 1;
        varIndex = 2;
    } else if (ircode->getObj(2)->getCategory() == 5) {
        numIndex = 2;
        varIndex = 1;
    }
    if (numIndex != -1) {
        //cout << "numIndex " << numIndex << endl;
        if (ircode->getObj(varIndex)->getVar() != nullptr
            and ircode->getObj(varIndex)->getVar()->getIsTemp()) {
            // a = t1 + c
            if (var2reg.find(ircode->getObj(varIndex)->getVar()) == var2reg.end()) {
                // if t1 is not in register, assign one to it.
                loadTempVar(*this, ircode->getObj(varIndex));
            }
            //cout << "load a new one" << endl;
            if (ircode->getObj(0)->getVar() != nullptr
                and ircode->getObj(0)->getVar()->getIsTemp()) {
                // tn = t1 + c
                int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                          -1 : var2reg[ircode->getObj(0)->getVar()];
                // check whether tn is in register
               // cout << "idx = " << idx << endl;
                int index = moveTempVar(*this, var2reg[ircode->getObj(varIndex)->getVar()], idx, output, ircode->getObj(0));
               // cout << "index = " << index << endl;
                var2reg[ircode->getObj(0)->getVar()] = index;
                output << calc+"i "  << tempRegs[index]->getId() << ", "
                       << tempRegs[index]->getId() << ", "
                       << ", " << ircode->getObj(numIndex)->getNum() << endl;
            } else {
                output << calc+"i $s2" << ", " << var2reg[ircode->getObj(varIndex)->getVar()] << ", "
                       << ircode->getObj(1)->getNum() << endl;
                save(ircode->getObj(0), "$s2");
            }
        } else {
            load(ircode->getObj(varIndex), "$s2");
            output << calc+"i $s3, $s2, " << ircode->getObj(numIndex)->getNum() << endl;
            save(ircode->getObj(0), "$s3");
        }
    } else {
        // a = b + c
        int tmpCnt = 0, tmpIndex;
        if (ircode->getObj(1)->getVar()!= nullptr
            and ircode->getObj(1)->getVar()->getIsTemp()) {
            tmpCnt += 1;
            if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
                loadTempVar(*this, ircode->getObj(1));
            }
            tmpIndex = 1;
        }
        if (ircode->getObj(2)->getVar()!= nullptr
            and ircode->getObj(2)->getVar()->getIsTemp()) {
            tmpCnt += 1;
            if (var2reg.find(ircode->getObj(2)->getVar()) == var2reg.end()) {
                loadTempVar(*this, ircode->getObj(2));
            }
            tmpIndex = 2;
        }
        if (tmpCnt == 2) {
            // a = t1 + t2
            if (ircode->getObj(0)->getVar()!= nullptr
                and ircode->getObj(0)->getVar()->getIsTemp()) {
                // tn = t1 + t2
                int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                          -1 : var2reg[ircode->getObj(0)->getVar()];
                // check whether tn is in register
                //cout << var2reg[ircode->getObj(1)->getVar()] << " " << var2reg[ircode->getObj(2)->getVar()] << endl;
                int index = moveTempVar(*this, var2reg[ircode->getObj(1)->getVar()], idx, output, ircode->getObj(0));
                var2reg[ircode->getObj(0)->getVar()] = index;
                output << calc+"u " << tempRegs[index]->getId() << ", "
                       << tempRegs[index]->getId() << ", "
                       << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
            } else {
                output << calc+"u $s2, " << tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId() << ", "
                       << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
                save(ircode->getObj(0), "$s2");
            }
        } else if (tmpCnt == 1) {
            // a = t1 + b
            load(ircode->getObj(3 - tmpIndex), "$s2");
            if (ircode->getObj(0)->getVar() != nullptr
                and ircode->getObj(0)->getVar()->getIsTemp()) {
                // tn = t1 + b
                cout << var2reg.size()  << endl;

                int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                          -1 : var2reg[ircode->getObj(0)->getVar()];
                // check whether tn is in register
                int index = moveTempVar(*this, var2reg[ircode->getObj(tmpIndex)->getVar()], idx, output, ircode->getObj(0));
                var2reg[ircode->getObj(0)->getVar()] = index;
                //cout << "idx = " << idx << " index = " << index << endl;
                if (tmpIndex == 1) {
                    output << calc+"u " << tempRegs[index]->getId() << ", "
                       << tempRegs[index]->getId() << ", "
                       << "$s2" << endl;
                } else {
                    output << calc+"u " << tempRegs[index]->getId() << ", "
                       << "$s2" << ", "
                       << tempRegs[index]->getId() << endl;
                }
            } else {
                //load(ir
                output << calc+"u $s2, " << tempRegs[var2reg[ircode->getObj(tmpIndex)->getVar()]]->getId() << ", "
                       << "$s2" << endl;
                save(ircode->getObj(0), "$s2");
            }
        } else {
            // a = b + c
            load(ircode->getObj(1), "$s2");
            load(ircode->getObj(2), "$s3");
            output << calc+"u $s2, $s2, $s3" << endl;
            save(ircode->getObj(0), "$s2");
        }
    }
}

void MipsGenerator::dealMulAndDiv(shared_ptr<IRCode> &ircode, const string& calc) {

    int tmpCnt = 0, tmpIndex;
    if (ircode->getObj(1)->getVar() != nullptr
        and ircode->getObj(1)->getVar()->getIsTemp()) {
        tmpCnt += 1;
        if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
            loadTempVar(*this, ircode->getObj(1));
        }
        tmpIndex = 1;
    }
    if (ircode->getObj(2)->getVar()!= nullptr
        and ircode->getObj(2)->getVar()->getIsTemp()) {
        tmpCnt += 1;
        if (var2reg.find(ircode->getObj(2)->getVar()) == var2reg.end()) {
            loadTempVar(*this, ircode->getObj(2));
        }
        tmpIndex = 2;
    }
    if(tmpCnt == 2) {
        // t1*t2
        output << calc+" " << tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId() << ", "
               << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
    } else if(tmpCnt == 1) {
        // t1*b
        load(ircode->getObj(3 - tmpIndex), "$s2");
        if (tmpIndex == 1) {
            output << calc + " " << tempRegs[var2reg[ircode->getObj(tmpIndex)->getVar()]]->getId() << ", "
                   << "$s2" << endl;
        } else {
           output << calc + " " << "$s2, "
                   << tempRegs[var2reg[ircode->getObj(tmpIndex)->getVar()]]->getId() << endl;
        }
    } else {
        // b*c
        load(ircode->getObj(1), "$s2");
        load(ircode->getObj(2), "$s3");
        output << calc+" " << "$s2" << ", " << "$s3" << endl;
    }
}

void MipsGenerator::work(GenerateBlock &blockGenerator) {
    InitRegs();
    //cout << "-----------" << endl;
    //cout << "New MipsGenerator" << endl;
    output << ".data" << endl;
    for (auto &attributeData: attributeDataList) {
        output << attributeData.label << ": .asciiz " << "  \"" << attributeData.content << "\"" << endl;
    }
    output << R"(endl: .asciiz "\n")" << endl;
    output << endl << ".text" << endl;
    for (int i = 0; i <= blockGenerator.getStart(); ++i) {
        // global variable definitions
        auto &ircode = IRCodeList[i];
        if (ircode->getOp() == VarOT or ircode->getOp() == ConstOT) {
            if (ircode->getObj(2) != nullptr) {
                load(ircode->getObj(2), "$t0");
                save(ircode->getObj(1), "$t0");
            }
        }
        else if (ircode->getOp() == MainOT) {
            output << endl << "j function_main" << endl;
        }
        else {
            cout << "Error: MipsGenerator::work" << endl;
        }
    }
    //cout << "MAIN BEGIN" << endl;
    for (auto & i : blockGenerator.getBasicBlocks()) {
        rollBaclRegs(*this);
       // cout << "A New Block" << endl;
        for (auto &ircode: i->getIRCodes()) {
            //cout << "No: " << ircode->getOp() << endl;
            cout << operatorName[ircode->getOp()] << endl;
            switch (ircode->getOp()) {
                case PrintOT: {
                    output << "la $a0, " << ircode->getObj(0)->getName() << endl;
                    output << "li $v0, 4" << endl;
                    SYSCALL;
                    break;
                }
                case PrintIntOT: {
                    if (ircode->getObj(0)->getVar() != nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) != var2reg.end()) {
                            output << "move $a0, " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                        } else {
                            load(ircode->getObj(0), "$a0");
                        }
                    } else {
                        load(ircode->getObj(0), "$a0");
                    }
                    output << "li $v0, 1" << endl;
                    SYSCALL;
                    break;
                }
                case GetIntOT: {
                    output << "li $v0, 5" << endl;
                    SYSCALL;
                    save(ircode->getObj(0), "$v0");
                    break;
                }
                case VarOT: {
                    //output << "VAR DEFINE" << endl;
                    if (ircode->getObj(2) != nullptr) {
                        if (ircode->getObj(2)->getVar()!= nullptr
                        and ircode->getObj(2)->getVar()->getIsTemp()) {
                            if (var2reg.find(ircode->getObj(2)->getVar()) == var2reg.end()) {
                                //save(ircode->getObj(1), var2reg[ircode->getObj(2)->getVar()]);
                                tempRegs[var2reg[ircode->getObj(2)->getVar()]]->storeValue(*this, ircode->getObj(2));
                            }
                            output << "move $s2, " << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
                        }
                        else {
                            load(ircode->getObj(2), "$s2");
                        }
                        save(ircode->getObj(1), "$s2");
                    }
                    break;
                }
                case ConstOT: {
                    if (ircode->getObj(2) != nullptr) {
                        if (ircode->getObj(2)->getVar()->getIsTemp()) {
                            if (var2reg.find(ircode->getObj(2)->getVar()) != var2reg.end()) {
                                //save(ircode->getObj(1), var2reg[ircode->getObj(2)->getVar()]);
                                tempRegs[var2reg[ircode->getObj(2)->getVar()]]->storeValue(*this, ircode->getObj(1));
                            } else {
                                load(ircode->getObj(2), "$s2");
                                save(ircode->getObj(1), "$s2");
                            }
                        } else {
                            load(ircode->getObj(2), "$s2");
                            save(ircode->getObj(1), "$s2");
                        }
                    }
                    break;
                }
                case FuncOT: {
                    shared_ptr<FuncIRCode> &func = ircode->getObj(1)->getFunc();
                    output << ("function_" + ircode->getObj(1)->getName()) << ":" << endl;
                    if (func->getName() == "main") {
                        output << "move $fp, $sp" << endl;
                        output << "addi $sp, $sp, " << -(4 * func->getOffset() + 8) << endl;
                    }
                    break;
                }
                case PushOT: {
                    funcParams.push(ircode->getObj(0));
                    if (ircode->getObj(0)->getVar() != nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()
                        and var2reg.find(ircode->getObj(0)->getVar()) != var2reg.end()) {
                            tempRegs[var2reg[ircode->getObj(0)->getVar()]]->rollBack(*this);
                    }
                    break;
                }
                case CallOT: {
                    shared_ptr<Object> object = ircode->getObj(0);
                    shared_ptr<FuncIRCode> func = object->getFunc();
                    vector<shared_ptr<VarIRCode> > &exps = func->getExps();
                    int num = (int) exps.size();
                    int offset = (4 * func->getOffset() + 8);
                    string name = func->getName();
                    output << "addi $s0, $sp, " << -offset << endl;
                    while (num > 0) {
                        shared_ptr<Object> param = funcParams.top();
                        // param is real, exps is set
                        funcParams.pop();
                        shared_ptr<VarIRCode> &var = param->getVar();
                        if (var != nullptr and var->getDimension() not_eq 0
                            and (var->getDimension() == exps[num - 1]->getDimension()
                                 or (var->getDimension() == 2 and exps[num - 1]->getDimension() == 1))) {
                            int domain = var->getDomain();
                            string t_reg = domain == 0 ? "$gp" : "$sp";
                            if (var->getIsArrayPara()) {
                                // the parameter used as the other function's parameter
                                if (var->getDimension() == exps[num - 1]->getDimension()) {
                                    output << "lw $t8, " << var->getOffset() * 4 << "($sp)" << endl;
                                    output << "sw $t8, " << num * 4 << "($s0)" << endl;
                                } else {
                                    // a is 2D and use as 1D, int fun(int a[][3]) {func(a[0]);}
                                    output << "lw $t8, " << var->getOffset() * 4 << "($sp)" << endl;
                                    if (param->getCategory() == 4) {
                                        int index = param->getNumericIndex() * var->getSpectrum()[1] * 4;
                                        output << "subi $t8, $t8, " << index << endl;
                                    } else {
                                        load(param->getIndex(), "$t9");
                                        output << "li $s1, " << var->getSpectrum()[1] * 4 << endl;
                                        output << "mult $t9, $s1" << endl;
                                        output << "mflo $s1" << endl;
                                        output << "subu $t8, $t8, $s1" << endl;
                                    }
                                    output << "sw $t8, " << num * 4 << "($s0)" << endl;
                                }
                            } else if (exps[num - 1]->getDimension() == var->getDimension()) {
                                exps[num - 1]->setOffset(var->getOffset());
                                output << "li $t8, " << var->getOffset() * 4 << endl;
                                output << "addu $t8, " << t_reg << ", $t8" << endl;
                                output << "sw $t8, " << 4 * num << "($s0)" << endl;
                            } else if (exps[num - 1]->getDimension() == 1 and var->getDimension() == 2) {
                                if (param->getCategory() == 4) {
                                    exps[num - 1]->setOffset(
                                            var->getOffset() - param->getNumericIndex() * var->getSpectrum()[1]);
                                    output << "li $t8, " << exps[num - 1]->getOffset() * 4 << endl;
                                    output << "addu $t8, " << t_reg << ", $t8" << endl;
                                    output << "sw $t8, " << 4 * num << "($s0)" << endl;
                                } else {
                                    load(param->getIndex(), "$t8");
                                    output << "li $t9, " << var->getSpectrum()[1] * 4 << endl;
                                    output << "mult $t8, $t9" << endl;
                                    output << "mflo $s1" << endl;
                                    output << "li $t9, " << var->getOffset() * 4 << endl;
                                    output << "subu $t8, $t9, $s1" << endl;
                                    output << "addu $t8, $t8, " << t_reg << endl;
                                    output << "sw $t8, " << 4 * num << "($s0)" << endl;
                                }
                            }
                        } else {
                            load(param, "$t8");
                            output << "sw $t8, " << 4 * num << "($s0)" << endl;
                        }
                        --num;
                    }
                    output << "sw $ra, ($sp)" << endl;
                    output << "sw $fp, -4($sp)" << endl;
                    output << "addi $sp, $sp, " << -offset << endl;
                    output << "addi $fp, $sp," << offset << endl;
                    output << "jal function_" << name << endl;
                    output << "addi $sp, $sp, " << offset << endl;
                    output << "lw $fp, -4($sp)" << endl;
                    output << "lw $ra, ($sp)" << endl;
                    break;
                }
                case ReturnOT: {
                    if (ircode->getObj(0) != nullptr) {
                        if (ircode->getObj(0)->getVar() != nullptr
                            and ircode->getObj(0)->getVar()->getIsTemp()) {
                            if (var2reg.find(ircode->getObj(0)->getVar())!= var2reg.end()) {
                                output << "move $v0, " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                            } else {
                                load(ircode->getObj(0), "$v0");
                            }
                        }
                        else
                            load(ircode->getObj(0), "$v0");
                    }
                    output << "jr $ra" << endl;
                    break;
                }
                case ExitOT: {
                    output << "li $v0, 10" << endl;
                    SYSCALL;
                    break;
                }
                case PLUSOT: {
                    dealPlusAndMinus(ircode, "add");
                    break;
                }
                case MINUSOT: {
                    dealPlusAndMinus(ircode, "sub");
                    break;
                }
                case MULTOT: {
                    dealMulAndDiv(ircode, "mult");
                    if (ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(0));
                        }
                        output << "mflo " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                    } else {
                        output << "mflo $s4" << endl;
                        save(ircode->getObj(0), "$s4");
                    }
                    break;
                }
                case MULTHOT:{
                    dealMulAndDiv(ircode, "mult");
                    if (ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(0));
                        }
                        output << "mfhi " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                    } else {
                        output << "mfhi $s4" << endl;
                        save(ircode->getObj(0), "$s4");
                    }
                    break;
                }
                case DIVOT: {
                    dealMulAndDiv(ircode, "div");
                    if (ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(0));
                        }
                        output << "mflo " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                    } else {
                        output << "mflo $s4" << endl;
                        save(ircode->getObj(0), "$s4");
                    }
                    break;
                }
                case MODOT: {
                    dealMulAndDiv(ircode, "div");
                    if (ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(0));
                        }
                        output << "mfhi " << tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId() << endl;
                    } else {
                        output << "mfhi $s4" << endl;
                        save(ircode->getObj(0), "$s4");
                    }
                    break;
                }
                case AssignOT: {
                    string name = "$s2";
                    if (ircode->getObj(1)->getName() == "RET")
                        // return value
                        output << "move $s2, $v0" << endl;
                    else {
                        if (ircode->getObj(1)->getVar()!= nullptr
                            and ircode->getObj(1)->getVar()->getIsTemp()) {
                            // a = t1
                            if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
                                //save(ircode->getObj(0), var2reg[ircode->getObj(1)->getVar()]);
                                loadTempVar(*this, ircode->getObj(1));
                            }
                            name = tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId();
                        } else {
                            load(ircode->getObj(1), "$s2");
                        }
                    }
                    if (name != "$s2" and ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                                  -1 : var2reg[ircode->getObj(0)->getVar()];
                        // check whether tn is in register
                        int index = moveTempVar(*this, var2reg[ircode->getObj(1)->getVar()], idx, output, ircode->getObj(0));
                        var2reg[ircode->getObj(0)->getVar()] = index;
                    } else {
                        save(ircode->getObj(0), name);
                    }
                    break;
                }
                case MainOT: {
                    output << endl << "j function_main" << endl;
                    break;
                }
                case NOTOT: {
                    if (ircode->getObj(2)->getVar() != nullptr
                        and ircode->getObj(2)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(2)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(2));
                        }
                        if (ircode->getObj(0)->getVar() != nullptr
                            and ircode->getObj(0)->getVar()->getIsTemp()) {
                            int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                                      -1 : var2reg[ircode->getObj(0)->getVar()];
                            // check whether tn is in register
                            int index = moveTempVar(*this, var2reg[ircode->getObj(1)->getVar()], idx, output, ircode->getObj(0));
                            var2reg[ircode->getObj(0)->getVar()] = index;
                            output << "seq " << tempRegs[index]->getId() << ", "
                            << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId()
                            << ", $zero" << endl;
                        } else {
                            output << "beq $s2, " << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId()
                            << ", $zero" << endl;
                            save(ircode->getObj(0), "$s2");
                        }
                    } else {
                        load(ircode->getObj(2), "$s2");
                        output << "seq $s2, $s2, $zero" << endl;
                        save(ircode->getObj(0), "$s2");
                    }
                    break;
                }
                case JumpOT: {
                    output << "j " << ircode->getObj(0)->getName() << endl;
                    break;
                }
                case LabelOT: {
                    output << ircode->getObj(0)->getName() << ":" << endl;
                    break;
                }
                case BLTOT:
                case BLEOT:
                case BGTOT:
                case BGEOT:
                case BEQOT:
                case BNEOT: {
                    string name1 = "$s2", name2 = "$s3";
                    if (ircode->getObj(0)->getVar()!= nullptr
                        and ircode->getObj(0)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(0));
                        }
                        name1 = tempRegs[var2reg[ircode->getObj(0)->getVar()]]->getId();
                    }
                    if (ircode->getObj(1)->getVar()!= nullptr
                        and ircode->getObj(1)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(1));
                        }
                        name2 = tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId();
                    }
                    if (name1 == "$s2") {
                        load(ircode->getObj(0), "$s2");
                    }
                    if (name2 == "$s3") {
                        load(ircode->getObj(1), "$s3");
                    }
                    output << operatorName[ircode->getOp()] << " " << name1 << ", " << name2 << ", " << ircode->getObj(2)->getName() << endl;
                    break;
                }
                case SLLOT:
                case SRLOT:
                case SRAOT: {
                    string name = "$s2";
                    if (ircode->getObj(1)->getVar() != nullptr
                        and ircode->getObj(1)->getVar()->getIsTemp()) {
                        if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(1));
                        }
                        name = tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId();
                    } else {
                        load(ircode->getObj(1), "$s2");
                    }
                    if (name != "$s2" and ircode->getObj(0)->getVar()->getIsTemp()) {
                        int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                                  -1 : var2reg[ircode->getObj(0)->getVar()];
                        // check whether tn is in register
                        int index = moveTempVar(*this, var2reg[ircode->getObj(0)->getVar()], idx, output, ircode->getObj(0));
                        var2reg[ircode->getObj(0)->getVar()] = index;
                        output << operatorName[ircode->getOp()] << " " << tempRegs[index]->getId() << ", "
                        << name << ", " << ircode->getObj(2)->getNum() << endl;
                    }   else {
                        output << operatorName[ircode->getOp()] << " $s2, "
                               << name << ", " << ircode->getObj(2)->getNum() << endl;
                        save(ircode->getObj(0), "$s2");
                    }
                    break;
                }
                case ANDOT:
                case OROT:
                case SEQOT:
                case SNEOT:
                case SLTOT:
                case SLEOT:
                case SGTOT:
                case SGEOT:
                case SRLVOT:
                case SLLVOT:
                case SRAVOT: {
                    int tmpCnt = 0, tmpIndex;
                    if (ircode->getObj(1)->getVar() != nullptr
                        and ircode->getObj(1)->getVar()->getIsTemp()) {
                        tmpCnt += 1;
                        if (var2reg.find(ircode->getObj(1)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(1));
                        }
                        tmpIndex = 1;
                    }
                    if (ircode->getObj(2)->getVar()!= nullptr
                        and ircode->getObj(2)->getVar()->getIsTemp()) {
                        tmpCnt += 1;
                        if (var2reg.find(ircode->getObj(2)->getVar()) == var2reg.end()) {
                            loadTempVar(*this, ircode->getObj(2));
                        }
                        tmpIndex = 2;
                    }
                    if (tmpCnt == 2) {
                        // a = t1 ble t2
                        if (ircode->getObj(0)->getVar()!= nullptr
                            and ircode->getObj(0)->getVar()->getIsTemp()) {
                            // tn = t1 ble  t2
                            int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                                      -1 : var2reg[ircode->getObj(0)->getVar()];
                            // check whether tn is in register
                            int index = moveTempVar(*this, var2reg[ircode->getObj(1)->getVar()], idx, output, ircode->getObj(0));
                            var2reg[ircode->getObj(0)->getVar()] = index;
                            output << operatorName[ircode->getOp()] << " " << tempRegs[index]->getId() << ", "
                                   << tempRegs[index]->getId() << ", "
                                   << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
                        } else {
                            output << operatorName[ircode->getOp()] << " $s2, " << tempRegs[var2reg[ircode->getObj(1)->getVar()]]->getId() << ", "
                                   << tempRegs[var2reg[ircode->getObj(2)->getVar()]]->getId() << endl;
                            save(ircode->getObj(0), "$s2");
                        }
                    } else if (tmpCnt == 1) {
                        // a = t1 + b
                        load(ircode->getObj(3 - tmpIndex), "$s2");
                        if (ircode->getObj(0)->getVar() != nullptr
                            and ircode->getObj(0)->getVar()->getIsTemp()) {
                            // tn = t1 + b
                            int idx = (var2reg.find(ircode->getObj(0)->getVar()) == var2reg.end()) ?
                                      -1 : var2reg[ircode->getObj(0)->getVar()];
                            // check whether tn is in register
                            int index = moveTempVar(*this, var2reg[ircode->getObj(tmpIndex)->getVar()], idx, output, ircode->getObj(0));
                            var2reg[ircode->getObj(0)->getVar()] = index;
                            if (tmpIndex == 1) {
                                output << operatorName[ircode->getOp()] << " " << tempRegs[index]->getId() << ", "
                                       << tempRegs[index]->getId() << ", "
                                       << "$s2" << endl;
                            } else {
                                output << operatorName[ircode->getOp()] << " " << tempRegs[index]->getId() << ", "
                                       << "$s2" << ", "
                                       << tempRegs[index]->getId() << endl;
                            }
                        } else {
                            //load(ir
                            output << operatorName[ircode->getOp()] << " $s2, " << tempRegs[var2reg[ircode->getObj(tmpIndex)->getVar()]]->getId() << ", "
                                   << "$s2" << endl;
                            save(ircode->getObj(0), "$s2");
                        }
                    } else {
                        // a = b + c
                        load(ircode->getObj(1), "$s2");
                        load(ircode->getObj(2), "$s3");
                        output << operatorName[ircode->getOp()] <<" $s2, $s2, $s3" << endl;
                        save(ircode->getObj(0), "$s2");
                    }
                    break;
                }
                case ParamOT:
                case ArrayOT: {
                    break;
                }
                default: {
                    cout << "error:unknown" << endl;
                    cout << operatorName[ircode->getOp()] << endl;
                    break;
                }
            }
        }
       // cout << "Finish a block" << endl;
    }
}
