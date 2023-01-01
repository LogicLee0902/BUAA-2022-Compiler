//
// Created by Leo on 2022/11/16.
//

#include "include/Optimization.h"
#include <iostream>
#include <vector>
#include "include/BasicBlock.h"

using namespace std;
using namespace irCode;

GenerateBlock Optimization::run() {
#ifdef OPTIMIZE
    cout << "Optimization start" << endl;
//    analyzeFunction();
//    analyzeLoop();
    GenerateBlock generator;
    generator.run();
    cout << "Optimization finished" << endl;
    return std::move(generator);
#endif
}

void Optimization::analyzeFunction() {
    for (auto &i: IRCodeList) {
        OperatorType op = i->getOp();
        if (op == FuncOT) {
            string name = i->getObj(1)->getName();
            if (name.find("main") != string::npos) {
                return;
            }
            continue;
        }
        if (op == VarOT) continue;
        for (int j = 0; j < 3; ++j) {
            shared_ptr<Object> object = i->getObj(j);
            if (object != nullptr and object->getVar() != nullptr) {
                if (object->getVar()->getDomain() == 0) {
                    object->getVar()->setIsUse(true);
                }
            }
        }
    }
}

void Optimization::analyzeLoop() {
    vector<int> while_end;
    vector<bool> while_value;
    bool isCond = false;
    for (int i = (int)IRCodeList.size() - 1; i >= 0; i--) {
        OperatorType op = IRCodeList[i]->getOp();
        switch (op) {
            case LabelOT: {
                shared_ptr<Object> object = IRCodeList[i]->getObj(0);
                string name = object->getName();
                if (name.find("while") != string::npos and name.find("end") != string::npos) {
                    while_end.push_back(i + 1);
                    // unused by default
                    while_value.push_back(false);
                }
                if (name.find("while") != string::npos and name.find("begin") != string::npos) {
                    isCond = true;
                }
                if (name.find("while") != string::npos and name.find("head") != string::npos) {
                    isCond = false;
                    int end = while_end.back();
                    while_end.pop_back();
                    bool value = while_value.back();
                    while_value.pop_back();

                    if (while_end.empty()) {
                        if (!value) {
                            // unused
                            IRCodeList.erase(IRCodeList.begin() + i, IRCodeList.begin() + end);
                        }
                    } else {
                        if (!value) {
                            // unused
                            IRCodeList.erase(IRCodeList.begin() + i, IRCodeList.begin() + end);
                            for (int &j: while_end) {
                                j -= (end - i);
                            }
                        }
                    }
                }
                break;
            }
            case CallOT:
            case GetIntOT: {
                for (auto &&j: while_value) {
                    j = true;
                }
                break;
            }
            case PrintIntOT:
            case ReturnOT:
            case PushOT:
            case PrintOT: {
                shared_ptr<Object> object = IRCodeList[i]->getObj(0);
                if (object != nullptr and object->getVar() != nullptr) {
                    if (object->getCategory() == 3 and object->getIndex() != nullptr
                        and object->getIndex()->getVar() != nullptr) {
                        object->getIndex()->getVar()->setIsUse(true);
                    }
                    object->getVar()->setIsUse(true);
                }
                for (auto &&j: while_value) {
                    j = true;
                }
                break;
            }
            case VarOT: {
                shared_ptr<Object> object = IRCodeList[i]->getObj(2);
                if (object != nullptr and object->getVar() != nullptr) {
                    if (object->getCategory() == 3 and object->getIndex() != nullptr
                        and object->getIndex()->getVar() != nullptr) {
                        object->getIndex()->getVar()->setIsUse(true);
                    }
                    object->getVar()->setIsUse(true);
                }
                break;
            }
            case AssignOT: {
                // object0 = object1
                shared_ptr<Object> object0 = IRCodeList[i]->getObj(0);
                shared_ptr<Object> object1 = IRCodeList[i]->getObj(1);
                if (!object0->getVar()->getIsArrayPara() and !object0->getVar()->getIsUse()) {
                    if (while_value.empty()) {
                        IRCodeList.erase(IRCodeList.begin() + i);
                    }
                } else {
                    if (object0->getIndex() != nullptr and object0->getIndex()->getVar() != nullptr) {
                        object0->getIndex()->getVar()->setIsUse(true);
                    }
                    if (object1 != nullptr and object1->getVar() != nullptr) {
                        if (object1->getCategory() == 3 and object1->getIndex() != nullptr
                            and object1->getIndex()->getVar() != nullptr) {
                            object1->getIndex()->getVar()->setIsUse(true);
                        }
                        object1->getVar()->setIsUse(true);
                    }
                    for (auto &&j: while_value) {
                        j = true;
                    }
                }
                break;
            }
            case NOTOT:
            case MULTOT:
            case DIVOT:
            case PLUSOT:
            case MINUSOT:
            case MODOT:
            case ANDOT:
            case OROT:
            case SLLOT:
            case SRLOT:
            case SRLVOT:
            case SLLVOT: {
                // object0 = object1 op object2
                shared_ptr<Object> object0 = IRCodeList[i]->getObj(0);
                shared_ptr<Object> object1 = IRCodeList[i]->getObj(1);
                shared_ptr<Object> object2 = IRCodeList[i]->getObj(2);
                if (!object0->getVar()->getIsArrayPara() and !object0->getVar()->getIsUse()) {
                    if (while_value.empty()) {
                        IRCodeList.erase(IRCodeList.begin() + i);
                    }
                } else {
                    if (object0->getIndex() != nullptr and object0->getIndex()->getVar() != nullptr) {
                        object0->getIndex()->getVar()->setIsUse(true);
                    }
                    if (object1 != nullptr and object1->getVar() != nullptr) {
                        if (object1->getCategory() == 3 and object1->getIndex() != nullptr
                            and object1->getIndex()->getVar() != nullptr) {
                            object1->getIndex()->getVar()->setIsUse(true);
                        }
                        object1->getVar()->setIsUse(true);
                    }
                    if (object2 != nullptr and object2->getVar() != nullptr) {
                        if (object2->getCategory() == 3 and object2->getIndex() != nullptr
                            and object2->getIndex()->getVar() != nullptr) {
                            object2->getIndex()->getVar()->setIsUse(true);
                        }
                        object2->getVar()->setIsUse(true);
                    }
                    if (!isCond) {
                        for (auto &&j: while_value) {
                            j = true;
                        }
                    }
                }
                break;
            }
            case BGEOT:
            case BGTOT:
            case BLEOT:
            case BLTOT:
            case BNEOT:
            case BEQOT: {
                // if object1 op object2 goto label
                shared_ptr<Object> object0 = IRCodeList[i]->getObj(0);
                shared_ptr<Object> object1 = IRCodeList[i]->getObj(1);
                if (isCond and while_value.back()) {
                    // loop is working
                    if (object0 != nullptr and object0->getVar() != nullptr) {
                        if (object0->getCategory() == 3 and object0->getIndex() != nullptr
                            and object0->getIndex()->getVar() != nullptr) {
                            object0->getIndex()->getVar()->setIsUse(true);
                        }
                        object0->getVar()->setIsUse(true);
                    }
                    if (object1 != nullptr and object1->getVar() != nullptr) {
                        if (object1->getCategory() == 3 and object1->getIndex() != nullptr
                            and object1->getIndex()->getVar() != nullptr) {
                            object1->getIndex()->getVar()->setIsUse(true);
                        }
                        object1->getVar()->setIsUse(true);
                    }
                }
                break;
            }
            case SLTOT:
            case SLEOT:
            case SGEOT:
            case SGTOT:
            case SEQOT:
            case SNEOT: {
                // object0[0/1] = object1 op object2
                shared_ptr<Object> object0 = IRCodeList[i]->getObj(0);
                shared_ptr<Object> object1 = IRCodeList[i]->getObj(1);
                shared_ptr<Object> object2 = IRCodeList[i]->getObj(2);
                if (object0 != nullptr and object0->getVar() != nullptr) {
                    if (object0->getCategory() == 3 and object0->getIndex() != nullptr
                        and object0->getIndex()->getVar() != nullptr) {
                        object0->getIndex()->getVar()->setIsUse(true);
                    }
                    object0->getVar()->setIsUse(true);
                }
                if (object1 != nullptr and object1->getVar() != nullptr) {
                    if (object1->getCategory() == 3 and object1->getIndex() != nullptr
                        and object1->getIndex()->getVar() != nullptr) {
                        object1->getIndex()->getVar()->setIsUse(true);
                    }
                    object1->getVar()->setIsUse(true);
                }
                if (object2 != nullptr and object2->getVar() != nullptr) {
                    if (object2->getCategory() == 3 and object2->getIndex() != nullptr
                        and object2->getIndex()->getVar() != nullptr) {
                        object2->getIndex()->getVar()->setIsUse(true);
                    }
                    object2->getVar()->setIsUse(true);
                }
                break;
            }
            default:
                break;
        }

    }

}