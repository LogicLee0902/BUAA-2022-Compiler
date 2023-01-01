//
// Created by Leo on 2022/10/23.
//
#include <iostream>
#include <stack>
#include <vector>
#include <algorithm>
#include <cmath>

#include <memory>
#include <utility>
#include "include/IRCodeGenerator.h"
#include "include/BigInteger.h"


namespace irCode {
    const int INF = 2147483647;
    using namespace std;
    using namespace ast;

    int varIndex, global_offset;
    stack<SymbolType> defineType;
    vector<shared_ptr<IRCode> > IRCodeList;
    IRCodeTable irCodeTable;
    vector<AttributeData> attributeDataList;
    bool isMain;
    stack<shared_ptr<Object> > stack_while_begin, stack_while_end;

    struct OutputInfo {
        string content;
        bool isOutput;
    };

    VarIRCode::VarIRCode(string name, int dimension, bool isConst, vector<int> &spectrum, SymbolType type, int domain) {
        this->name = std::move(name);
        this->dimension = dimension;
        this->isConst = isConst;
        this->type = type;
        this->spectrum = spectrum;
        this->originDomain = this->domain = domain;
        this->isTemp = false;
        int len = 1;
        for (int i: spectrum) {
            len *= i;
        }
        this->isArrayPara = false;
        this->values.reserve(len);
        this->isUse = false;
    }

    VarIRCode::VarIRCode(string name, int dimension, bool isConst, shared_ptr<Object> &value, SymbolType type,
                         int domain) {
        this->name = std::move(name);
        this->dimension = dimension;
        this->isConst = isConst;
        this->type = type;
        this->originDomain = this->domain = domain;
        this->value = value;
        this->isArrayPara = false;
        this->isUse = false;
        this->isTemp = false;
    }

    VarIRCode::VarIRCode(string name, int dimension, bool isConst, SymbolType type, int domain) :
            name(std::move(name)), dimension(dimension), isConst(isConst), type(type), domain(domain) {
        this->originDomain = domain;
        this->value = make_shared<Object>(0);
        isArrayPara = false;
        isUse = false;
        isTemp = false;
    }

    void VarIRCode::setValue(int _value) {
        this->value = make_shared<Object>(_value);
    }

    void VarIRCode::setValue(shared_ptr<Object> _value) {
        this->value = std::move(_value);
    }

    void VarIRCode::setSpaceInfo(int _offset, int _space) {
        this->offset = _offset;
        this->space = _space;
    }

    void VarIRCode::setDefinedValue(bool defined) {
        this->definedValue = defined;
    }

    void VarIRCode::setIsArrayPara(bool arrayPara) {
        this->isArrayPara = arrayPara;
    }

    void VarIRCode::setOffset(int _offset) {
        this->offset = _offset;
    }

    void VarIRCode::setIsUse(bool use) {
        this->isUse = use;
    }

    void VarIRCode::setIsTemp(bool temp) {
        this->isTemp = temp;
    }

    void Object::setVar(shared_ptr<irCode::VarIRCode> &_var) {
        this->var = _var;
    }

    void Object::setNum(int number) {
        this->num = number;
    }

    void Object::setCategory(int _category) {
        this->category = _category;
    }

    string IRCodeGenerator::generateNewLabel(SymbolType type) {
        int count = 0;
        static int labelWhile = 0;
        static int labelIf = 0;
        if (type == WHILEST) {
            count = labelWhile++;
        } else if (type == IFST) {
            count = labelIf++;
        }
        string label = symbolName[type] + to_string(count);
        return label;
    }

    shared_ptr<Object> IRCodeGenerator::generateTempVarT() {
        static int count = 0;
        ++count;
        string name = "tmpVar_" + to_string(count);
        shared_ptr<Object> object = make_shared<Object>(name);
        return object;
    }

    IRCodeGenerator::IRCodeGenerator(shared_ptr<CompUnitAST> &compUnitAST, const string& filename) : compUnitAST(compUnitAST) {
        varIndex = 0;
        global_offset = 0;
        isMain = false;
        currentDomain = 0;
        IR_FILE = filename + "_20377241.txt";
        output.open(IR_FILE, ios::out);

    }

    string Object::Print() {
        switch (category) {
            case 0:
                return "";
            case 1:
                return symbolName[type];
            case 2:
                return name;
            case 3:
                return name + "[" + index->Print() + "]";
            case 4:
                return name + "[" + to_string(numericIndex) + "]";
            case 5:
                return to_string(num);
            case 6:
                return to_string(longNum);
            default:
                break;
        }
        return "";
    }

    void IRCode::printObject(int num, std::fstream &output) {
        for (int i = 0; i < num; ++i) {
            if (obj[i] != nullptr) {
                output << obj[i]->Print() << " ";
            }
        }
        output << "\n";
    }

    void IRCode::Print(fstream &output) {
        //cout << operatorName[op] << endl;
        switch (op) {
            case VarOT: {
                output << operatorName[op] << " ";
                printObject(3, output);
                break;
            }
            case ArrayOT: {
                output << operatorName[op] << " ";
                printObject(2, output);
                break;
            }
            case ConstOT: {
                output << operatorName[op] << " ";
                printObject(3, output);
                break;
            }
            case PLUSOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " + " << obj[2]->Print() << "\n";
                break;
            }
            case MINUSOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " - " << obj[2]->Print() << "\n";
                break;
            }
            case MULTOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " * " << obj[2]->Print() << "\n";
                break;
            }
            case BITANDOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " & " << obj[2]->Print() << "\n";
                break;
            }
            case MULTHOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " * " << obj[2]->Print() << " (high part)\n";
                break;
            }
            case DIVOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " / " << obj[2]->Print() << "\n";
                break;
            }
            case MODOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " % " << obj[2]->Print() << "\n";
                break;
            }
            case AssignOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << "\n";
                break;
            }

            case CallOT:
            case PushOT: {
                output << operatorName[op] << " " << obj[0]->Print() << "\n";
                break;
            }

            case FuncOT: {
                output << obj[0]->Print() << " " << obj[1]->Print() << "\n";
                break;
            }
            case NOTOT: {
                output << obj[0]->Print() << " = !" << obj[2]->Print() << "\n";
                break;
            }
            case ParamOT: {
                cout << operatorName[op] << " " << obj[0]->Print() << " " << obj[1]->Print() << endl;
                break;
            }
            case ReturnOT: {
                output << operatorName[op] << " ";
                printObject(3, output);
                break;
            }

            case PrintOT:
            case GetIntOT:
            case PrintIntOT: {
                output << operatorName[op] << " " << obj[0]->Print() << "\n";
                break;
            }

            case ExitOT:
            case MainOT: {
                output << operatorName[op] << "\n";
                break;
            }

            case JumpOT: {
                output << operatorName[op] << " " << obj[0]->Print() << "\n";
                break;
            }
            case LabelOT: {
                output << obj[0]->Print() << ":\n";
                break;
            }
            case BLTOT:
            case BLEOT:
            case BGTOT:
            case BGEOT:
            case BEQOT:
            case BNEOT:
            case ANDOT:
            case OROT:
            case SEQOT:
            case SNEOT:
            case SLTOT:
            case SLEOT:
            case SGTOT:
            case SGEOT:
            case SLLOT:
            case SRLOT:
            case SRLVOT:
            case SLLVOT:
            case SRAOT:
            case SRAVOT: {
                output << obj[0]->Print() << " = " << obj[1]->Print() << " " << operatorName[op] << " "
                       << obj[2]->Print() << "\n";
                break;
            }
            default: {
                cout << operatorName[op] << "can not identidy" << endl;
                break;
            }
        }
    }

    void IRCodeGenerator::analyzeConstInitVal(shared_ptr<ast::ConstInitValAST> &constInitVal,
                                              shared_ptr<irCode::Object> array[3]) {
        int dimension = array[1]->getVar()->getDimension();
        if (!dimension) {
            // const int var = value;
            array[2] = analyzeConstExp(constInitVal->getConstExp());
            array[1]->getVar()->setValue(array[2]);
#ifndef  OPTIMIZE
            shared_ptr<IRCode> ircode = make_shared<IRCode>(VarOT, array);
            IRCodeList.push_back(ircode);
#endif
        } else if (dimension == 1) {
            // const int array[2] = {1, 2};
            int size = (int) constInitVal->getConstInitVals().size();
            for (int i = 0; i < size; i++) {
                shared_ptr<Object> objects[3];
                objects[0] = make_shared<Object>(array[1]->getName(), i, array[1]->getVar());
                objects[1] = analyzeConstExp(constInitVal->getConstInitVals()[i]->getConstExp());
                objects[0]->getVar()->getValues().push_back(objects[1]);
                shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                IRCodeList.push_back(ircode);
            }
            // global array initialization: make the element zero
#ifdef INIT
            if (currentDomain == 0) {
                for (int i = size; i < array[1]->getNumericIndex(); i++) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), i, array[1]->getVar());
                    objects[1] = make_shared<Object>(0);
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
            }
#endif
        } else if (dimension == 2) {
            int first_dimension = array[1]->getVar()->getSpectrum()[0];
            int second_dimension = array[1]->getVar()->getSpectrum()[1];
            int i;
            for (i = 0; i < constInitVal->getConstInitVals().size(); ++i) {
                int j;
                for (j = 0; j < constInitVal->getConstInitVals()[i]->getConstInitVals().size(); ++j) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), i * second_dimension + j, array[1]->getVar());
                    objects[1] = analyzeConstExp(
                            constInitVal->getConstInitVals()[i]->getConstInitVals()[j]->getConstExp());
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
#ifdef INIT
                if (currentDomain == 0) {
                    // fill the rest of the array with zero
                    for (; j < second_dimension; ++j) {
                        shared_ptr<Object> objects[3];
                        objects[0] = make_shared<Object>(array[1]->getName(), i * second_dimension + j,
                                                         array[1]->getVar());
                        objects[1] = make_shared<Object>(0);
                        objects[0]->getVar()->getValues().push_back(objects[1]);
                        shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                        IRCodeList.push_back(ircode);
                    }
                }
#endif
            }
#ifdef INIT
            if (currentDomain == 0) {
                for (int k = i * second_dimension; k < first_dimension * second_dimension; ++k) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), k, array[1]->getVar());
                    objects[1] = make_shared<Object>(0);
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
            }
#endif
        }
    }

    void IRCodeGenerator::analyzeConstDef(shared_ptr<ast::ConstDefAST> &constDef) {
        string name = constDef->getName();
        int dimension = (int) constDef->getConstExps().size();
        vector<shared_ptr<ConstExpAST> > &constExps = constDef->getConstExps();
        shared_ptr<Object> before;
        shared_ptr<Object> after;
        vector<int> exps;
        int space = 1;

        for (int i = 0; i < dimension; ++i) {
            before = after;
            // calculate the dimension
            after = analyzeConstExp(constExps[i]);
            space *= after->getNum();
            exps.push_back(after->getNum());
        }
        shared_ptr<VarIRCode> var;
        if (dimension == 0) {
            var = make_shared<VarIRCode>(name, dimension, true, defineType.top(), currentDomain);
        } else {
            var = make_shared<VarIRCode>(name, dimension, true, exps, defineType.top(), currentDomain);
        }

        varIndex += space;
        if (currentDomain == 0) {
            global_offset += space;
        }
        var->setSpaceInfo(varIndex, space);
        // insert the variable into the symbol table
        irCodeTable.getVars().push_back(var);
        if (constDef->getConstInitVal()->getConstInitVals().empty()) {
            // const int a;
            shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, var)};
            analyzeConstInitVal(constDef->getConstInitVal(), objects);
        } else {
            shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()),
                                             make_shared<Object>(name, space, var)};
#ifndef OPTIMIZE
            shared_ptr<IRCode> ircode = make_shared<IRCode>(ArrayOT, objects);
            IRCodeList.push_back(ircode);
#endif
            analyzeConstInitVal(constDef->getConstInitVal(), objects);
        }
    }

    void IRCodeGenerator::analyzeConstDecl(shared_ptr<ConstDeclAST> &constDecl) {
        if (constDecl->getBtype()->getType() == INTTK) {
            defineType.push(INTST);
        }
        for (auto &i: constDecl->getConstDefs()) {
            analyzeConstDef(i);
        }
        defineType.pop();
    }

    shared_ptr<Object> IRCodeGenerator::analyzeLVal(shared_ptr<ast::LValAST> &lval) {
        string name = lval->getName();
        vector<shared_ptr<ExpAST> > &exps = lval->getConstExps();
        vector<shared_ptr<Object>> objectExps;
        shared_ptr<VarIRCode> var;

        for (int i = (int) irCodeTable.getVars().size() - 1; i >= 0; --i) {
            if (irCodeTable.getVars()[i]->getName() == name) {
                var = irCodeTable.getVars()[i];
                break;
            }
        }

        for (auto &exp: exps) {
            objectExps.push_back(analyzeExp(exp));
        }

        if (var->getDimension() == 0) {
            shared_ptr<Object> obj = make_shared<Object>(name, var);
            obj->setNum(var->getValue()->getNum());
            if (var->getIsConst()) {
                obj->setCategory(5);
            }
            return obj;
        } else if (var->getDimension() != exps.size()) {
            // only happening in passing the argument, it means passing the whole array
            // exps.size(): the true dimension
            if (exps.empty()) {
                shared_ptr<Object> obj = make_shared<Object>(name, var);
                //obj->setNum(var->getValue()->getNum());
                return obj;
            } else if (exps.size() == 1) {
                shared_ptr<Object> obj = make_shared<Object>(name, objectExps[0], var);
                return obj;
            }
        } else if (var->getDimension() == 1) {
            shared_ptr<Object> obj = make_shared<Object>(name, objectExps[0], var);
            if (objectExps[0]->getCategory() == 5 and var->getIsConst()) {
                obj->setNum(var->getValues()[objectExps[0]->getNum()]->getNum());
                obj->setCategory(5);
            }
            return obj;
        } else if (var->getDimension() == 2) {
            int firstDimension = var->getSpectrum()[0],
                    secondDimension = var->getSpectrum()[1];
            if (objectExps[0]->getCategory() == 5 and objectExps[1]->getCategory() == 5) {
                // locate
                int index = objectExps[0]->getNum() * secondDimension + objectExps[1]->getNum();
                shared_ptr<Object> object = make_shared<Object>(name, index, var);
                if (var->getIsConst()) {
                    object->setNum(var->getValues()[index]->getNum());
                    object->setCategory(5);
                }
                return object;
            }
            // need to use Object Index
            shared_ptr<Object> obj1D[3] = {generateTempVarT(), objectExps[0], make_shared<Object>(secondDimension)};
            shared_ptr<VarIRCode> var1D = make_shared<VarIRCode>(obj1D[0]->getName(), 0, false, INTST, currentDomain);
            varIndex += 1;
            if (currentDomain == 0) {
                global_offset += 1;
            }
            var1D->setIsTemp(true);
            var1D->setSpaceInfo(varIndex, 1);
            obj1D[0]->setVar(var1D);
            shared_ptr<IRCode> ircode1D = make_shared<IRCode>(MULTOT, obj1D);
            IRCodeList.push_back(ircode1D);

            shared_ptr<Object> obj2D[3] = {generateTempVarT(), obj1D[0], objectExps[1]};
            shared_ptr<VarIRCode> var2D = make_shared<VarIRCode>(obj2D[0]->getName(), 0, false, INTST, currentDomain);
            varIndex += 1;
            if (currentDomain == 0) {
                global_offset += 1;
            }
            var2D->setIsTemp(true);
            var2D->setSpaceInfo(varIndex, 1);
            obj2D[0]->setVar(var2D);
            shared_ptr<IRCode> ircode2D = make_shared<IRCode>(PLUSOT, obj2D);
            IRCodeList.push_back(ircode2D);
            return make_shared<Object>(name, obj2D[0], var);
        }
        cout << "Erroe LVal" << endl;
        return nullptr;
    }

    shared_ptr<Object> IRCodeGenerator::analyzePrimaryExp(shared_ptr<ast::PrimaryExpAST> &primaryExp) {
        switch (primaryExp->getType()) {
            case 0: {
                shared_ptr<ExpAST> exp = dynamic_pointer_cast<ExpAST>(primaryExp->getExp());
                return analyzeExp(exp);
            }
            case 1: {
                shared_ptr<LValAST> lval = dynamic_pointer_cast<LValAST>(primaryExp->getExp());
                return analyzeLVal(lval);
            }
            case 2: {
                shared_ptr<NumberAST> number = dynamic_pointer_cast<NumberAST>(primaryExp->getExp());
                shared_ptr<Object> obj = make_shared<Object>(number->getNum());
                return obj;
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeExp(shared_ptr<ast::ExpAST> &exp) {
        shared_ptr<Object> obj = analyzeAddExp(exp->getAddExp());
        return obj;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeUnaryExp(shared_ptr<ast::UnaryExpAST> &unaryExp) {
        switch (unaryExp->getType()) {
            case 0: {
                return analyzePrimaryExp(unaryExp->getPrimaryExp());
            }
            case 1: {
                // funcCall
                string name = unaryExp->getName();
                shared_ptr<FuncRParamsAST> &funcRParams = unaryExp->getFuncRParams();
                if (funcRParams != nullptr) {
                    for (auto &i: funcRParams->getExps()) {
                        shared_ptr<Object> obj = analyzeExp(i);
                        shared_ptr<Object> objects[3];
                        objects[0] = obj;
                        // push the parameter
                        shared_ptr<IRCode> ircode = make_shared<IRCode>(PushOT, objects);
                        IRCodeList.push_back(ircode);
                    }
                }

                shared_ptr<FuncIRCode> func;
                for (auto &i: irCodeTable.getFuncs()) {
                    if (i->getName() == name) {
                        func = i;
                        break;
                    }
                }
                shared_ptr<Object> objects[3] = {make_shared<Object>(name, func)};
                shared_ptr<IRCode> ircode = make_shared<IRCode>(CallOT, objects);
                IRCodeList.push_back(ircode);

                if (func->getReturnType() != VOIDST) {
                    shared_ptr<Object> returnValue[3];
                    returnValue[0] = generateTempVarT();
                    shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, false, INTST,
                                                                       currentDomain);
                    varIndex += 1;
                    if (currentDomain == 0) {
                        global_offset += 1;
                    }
                    var->setIsTemp(true);
                    var->setSpaceInfo(varIndex, 1);
                    returnValue[0]->setVar(var);
                    returnValue[1] = make_shared<Object>("RET");
                    IRCodeList.push_back(make_shared<IRCode>(AssignOT, returnValue));
                    return returnValue[0];
                } else {
                    // no return value
                    return make_shared<Object>(0);
                }
            }
            case 2: {
                shared_ptr<UnaryOpAST> &unaryOp = unaryExp->getUnaryOp();
                shared_ptr<Object> obj = analyzeUnaryExp(unaryExp->getUnaryExp());

                shared_ptr<Object> objects[3];
                objects[0] = generateTempVarT();
                objects[1] = make_shared<Object>(0);
                objects[2] = obj;

                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, false, INTST,
                                                                   currentDomain);
                varIndex += 1;
                if (currentDomain == 0) {
                    global_offset += 1;
                }
                shared_ptr<IRCode> ircode;
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                objects[0]->setVar(var);
                if (unaryOp->getType() == PLUS) {
                    objects[0]->setNum(objects[2]->getNum());
                    ircode = make_shared<IRCode>(PLUSOT, objects);
                } else if (unaryOp->getType() == MINU) {
                    objects[0]->setNum(-objects[2]->getNum());
                    ircode = make_shared<IRCode>(MINUSOT, objects);
                } else if (unaryOp->getType() == NOT) {
                    if (obj->getCategory() == 5) {
                        objects[0]->setNum(obj->getNum() == 0 ? 1 : 0);
                    }
                    ircode = make_shared<IRCode>(NOTOT, objects);
                }
                if (obj->getCategory() == 5)
                    objects[0]->setCategory(5);
#ifndef OPTIMIZE
                IRCodeList.push_back(make_shared<IRCode>(NOTOT, objects));
#endif
#ifdef OPTIMIZE
                if (obj->getCategory() != 5)
                    IRCodeList.push_back(ircode);
#endif
                return objects[0];
            }
            default:
                break;
        }
        return nullptr;
    }

    bool isPowerOfTwo(int n) {
        return n && (!(n & (n - 1)));
    }

    bool judge(int n, int &a, int &b) {
        for (int i = 1; i <= (int) sqrt(n); ++i) {
            if (n % i == 0) {
                if (isPowerOfTwo(i) and isPowerOfTwo(n / i - 1)) {
                    a = (int) log2(i);
                    b = (int) log2(n / i - 1);
                    return true;
                } else if (isPowerOfTwo(n / i) and isPowerOfTwo(i - 1)) {
                    a = (int) log2(n / i);
                    b = (int) log2(i - 1);
                    return true;
                }
            }
        }
        return false;
    }

    long long getMagicNumber(const BigInteger &divImm, int &leftBit, int &shPosition) {
        leftBit = 0;
        BigInteger twoBase(2);
        while (divImm > (twoBase ^ leftBit)) {
            leftBit++;
        }
        shPosition = leftBit;
//        cout << "leftBit: " << leftBit << endl;
        long long lowValue = ((twoBase ^ (leftBit + 32)) / divImm).toLongLong();
        long long highValue = (((twoBase ^ (leftBit + 32)) + (twoBase ^ (leftBit + 1))) / divImm).toLongLong();
//        cout << lowValue << " " << highValue << endl;
        while ((highValue >> 1) > (lowValue >> 1) and shPosition > 0) {
            highValue >>= 1;
            lowValue >>= 1;
            shPosition--;
        }
        return highValue;
    }

    int BitCount(int num) {
        int total = 0;
        for (int j = 0; j < 32; j++) {
            total += (num) & 1;
            num >>= 1;
        }
        return total;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeMulExp(shared_ptr<ast::MulExpAST> &mulExp) {
        vector<shared_ptr<UnaryExpAST> > &unaryExps = mulExp->getUnaryExp();
        vector<identifierType> &types = mulExp->getMulOp();

        shared_ptr<Object> before, after;

        int num = 1;
        bool isConst = true;
        for (int i = 0; i < unaryExps.size(); ++i) {
            before = after;
            after = analyzeUnaryExp(unaryExps[i]);
//            cout << unaryExps[i]->getType() << endl;
//            cout << "after done!" << endl;
            if (i == 0) {
                num *= after->getNum();
            } else {
                if (types[i - 1] == MULT) {
                    num *= after->getNum();
                } else if (types[i - 1] == DIV and after->getNum() != 0) {
                    num /= after->getNum();
                } else if (types[i - 1] == MOD and after->getNum() != 0) {
                    num %= after->getNum();
                } else if (types[i] == BITAND) {
                    num &= after->getNum();
                }

            }
            if (after->getCategory() != 5) {
                isConst = false;
            }

            if (i != 0) {
                shared_ptr<Object> objects[3];
                objects[0] = generateTempVarT();
                objects[1] = before;
                objects[2] = after;
                after = objects[0];
                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, isConst, INTST,
                                                                   currentDomain);
                varIndex += 1;
                if (currentDomain == 0) {
                    global_offset += 1;
                }
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                objects[0]->setVar(var);
                if (isConst) {
                    objects[0]->setCategory(5);
                    objects[0]->setNum(num);
                }
                shared_ptr<IRCode> ircode, ircode2, ircode3, ircodeNeg;
                shared_ptr<Object> objectsNeg[3];
                //bool flag = false;
                // mult & div optimization
                int index = -1, cor_index, exp1, exp2;
                if (types[i - 1] == MULT) {
#ifdef  OPTIMIZE
                    if (objects[1]->getCategory() != 5 and objects[2]->getCategory() == 5) {
                        //getResult(ircode, ircode2, objects, objects[2]);
                        index = 2;
                        cor_index = 1;
                    } else if (objects[1]->getCategory() == 5 and objects[2]->getCategory() != 5) {
                        //getResult(ircode, ircode2, objects, objects[1]);
                        index = 1;
                        cor_index = 2;
                    }
                    if (index != -1) {
                        shared_ptr<Object> objects1[3];
                        objects1[0] = objects[0];
                        objects1[1] = objects[cor_index];
                        objects1[2] = objects[index];
                        if (isPowerOfTwo(objects1[2]->getNum())) {
                            int exponent = (int) log2(objects1[2]->getNum());
                            objects1[2] = make_shared<Object>(exponent);
                            ircode = make_shared<IRCode>(SLLOT, objects1);
                        } else if (isPowerOfTwo(objects1[2]->getNum() + 1)) {
                            int exponent = (int) log2(objects1[2]->getNum() + 1);
                            objects1[2] = make_shared<Object>(exponent);
                            ircode2 = make_shared<IRCode>(SLLOT, objects1);
                            IRCodeList.push_back(ircode2);
                            shared_ptr<Object> objects2[3];
                            objects2[0] = objects1[0];
                            objects2[1] = objects1[0];
                            objects2[2] = objects1[1];
                            ircode = make_shared<IRCode>(MINUSOT, objects2);
                        } else if (isPowerOfTwo(objects1[2]->getNum() - 1)) {
                            int exponent = (int) log2(objects1[2]->getNum() - 1);
                            objects1[2] = make_shared<Object>(exponent);
                            ircode2 = make_shared<IRCode>(SLLOT, objects1);
                            shared_ptr<Object> objects2[3];
                            objects2[0] = objects1[0];
                            objects2[1] = objects1[0];
                            objects2[2] = objects1[1];
                            ircode = make_shared<IRCode>(PLUSOT, objects2);
                            IRCodeList.push_back(ircode2);
                        } else if (judge(objects1[2]->getNum(), exp1, exp2)) {
                            //flag = true;
                            exp2 += exp1;
                            objects1[2] = make_shared<Object>(exp1);
                            ircode2 = make_shared<IRCode>(SLLOT, objects1);
                            shared_ptr<Object> objects3[3], objects2[3];
                            objects2[0] = generateTempVarT();
                            objects2[1] = objects1[1];
                            objects2[2] = make_shared<Object>(exp2);
                            ircode3 = make_shared<IRCode>(SLLVOT, objects2);
                            IRCodeList.push_back(ircode2);
                            shared_ptr<VarIRCode> var_temp = make_shared<VarIRCode>(objects2[0]->getName(), 0, false,
                                                                                    INTST,
                                                                                    currentDomain);
                            varIndex += 1;
                            if (currentDomain == 0) {
                                global_offset += 1;
                            }
                            var_temp->setIsTemp(true);
                            var_temp->setSpaceInfo(varIndex, 1);
                            objects2[0]->setVar(var_temp);
                            objects3[0] = objects1[0];
                            objects3[1] = objects1[0];
                            objects3[2] = objects2[0];
                            ircode = make_shared<IRCode>(PLUSOT, objects3);
                            IRCodeList.push_back(ircode3);
                        } else {
                            ircode = make_shared<IRCode>(MULTOT, objects);
                        }
                    } else {
                        ircode = make_shared<IRCode>(MULTOT, objects);
                    }
#else
                    ircode = make_shared<IRCode>(MULTOT, objects);
#endif
                } else if (types[i - 1] == DIV) {
#ifdef OPTIMIZE_DIVIDE
                    if(objects[1]->getCategory() != 5 and objects[2]->getCategory() == 5) {
                    int dividend = objects[2]->getNum();
                    int result, shPosition, leftBit;
                    shared_ptr<IRCode> ircodeMagic, ircode1;
                    long long magicNumber = getMagicNumber(BigInteger(abs(objects[2]->getNum())), leftBit, shPosition);
                    cout << "magicNumber: " << magicNumber << endl;
                    cout << "shPosition: " << shPosition << endl;
                    if (abs(objects[2]->getNum()) == 1) {
                        if (objects[2]->getNum() == 1) {
                           ircode = make_shared<IRCode>(AssignOT, objects);
                        } else {
                            shared_ptr<Object> objects1[3];
                            objects1[0] = objects[0];
                            objects1[2] = objects[1];
                            objects1[1] = make_shared<Object>(0);
                            ircode = make_shared<IRCode>(MINUSOT, objects1);
                        }
                    } /*else if (BitCount(abs(ircode->getObj(2)->getNum())) == 1) {
                        /* TODO: optimize the division by immediate number */
                    //}
                    else {
                        // objects1 for positive or negative
                        shared_ptr<Object> objectsMagic[3], objects1[3];
                        objectsMagic[0] = objects[0];
                        objectsMagic[1] = make_shared<Object>(magicNumber, "LONG");
                        objectsMagic[2] = objects[1];
                        ircodeMagic = make_shared<IRCode>(MULTHOT, std::move(objectsMagic));
                        IRCodeList.push_back(std::move(ircodeMagic));
                        if (magicNumber >= INF) {
                            shared_ptr<Object> objects2[3];
                            objects2[0] = generateTempVarT();
                            objects2[1] = objects[1];
                            objects2[2] = make_shared<Object>(1LL << 32, "LONG");
                            shared_ptr<VarIRCode> var_temp = make_shared<VarIRCode>(objects2[0]->getName(), 0, false,
                                                                                    INTST,
                                                                                    currentDomain);
                            varIndex += 1;
                            if (currentDomain == 0) {
                                global_offset += 1;
                            }
                            var_temp->setIsTemp(true);
                            var_temp->setSpaceInfo(varIndex, 1);
                            objects2[0]->setVar(var_temp);
                            ircode1 = make_shared<IRCode>(MULTOT, objects2);
                            IRCodeList.push_back(std::move(ircode1));
                            objectsMagic[0] = objects[0];
                            objectsMagic[1] = objects[0];
                            objectsMagic[2] = objects2[0];
                            ircodeMagic = make_shared<IRCode>(MINUSOT, std::move(objectsMagic));
                            IRCodeList.push_back(std::move(ircodeMagic));
                        }
                        // >> 32
                        /* objectsMagic[0] = objects[0];
                         objectsMagic[1] = objects[0];
                         objectsMagic[2] = make_shared<Object>(32);
                         ircodeMagic = make_shared<IRCode>(SRAOT, std::move(objectsMagic));
                         IRCodeList.push_back(std::move(ircodeMagic));*/
                        // & 0xfffffff
                        /*objectsMagic[0] = objects[0];
                        objectsMagic[1] = objects[0];
                         objectsMagic[2] = make_shared<Object>(0xffffffff);
                        ircodeMagic = make_shared<IRCode>(ANDOT, std::move(objectsMagic));

                        IRCodeList.push_back(std::move(ircodeMagic));*/
                        if (magicNumber >= INF) {
                            objectsMagic[0] = objects[0];
                            objectsMagic[1] = objects[0];
                            objectsMagic[2] = before;
                            ircodeMagic = make_shared<IRCode>(PLUSOT, std::move(objectsMagic));
                            IRCodeList.push_back(std::move(ircodeMagic));
                        }
                        // >> shPosition
                        objectsMagic[0] = objects[0];
                        objectsMagic[2] = make_shared<Object>(shPosition);
                        objectsMagic[1] = objects[0];
                        ircodeMagic = make_shared<IRCode>(SRAOT, objectsMagic);
                        IRCodeList.push_back(ircodeMagic);
                        // get sign bit
                        objects1[0] = generateTempVarT();
                        objects1[1] = before;
                        objects1[2] = make_shared<Object>(31);
                        shared_ptr<VarIRCode> var_temp = make_shared<VarIRCode>(objects1[0]->getName(), 0, false,
                                                                                INTST,
                                                                                currentDomain);
                        varIndex += 1;
                        if (currentDomain == 0) {
                            global_offset += 1;
                        }
                        var_temp->setIsTemp(true);
                        var_temp->setSpaceInfo(varIndex, 1);
                        objects1[0]->setVar(var_temp);
                        ircode1 = make_shared<IRCode>(SRLOT, objects1);
                        IRCodeList.push_back(ircode1);
                        objects[1] = objects[0];
                        objects[2] = objects1[0];
                        ircode = make_shared<IRCode>(PLUSOT, objects);
                        if (dividend < 0) {
                            cout << "deal neg" << endl;
                            objectsNeg[0] = objects[0];
                            objectsNeg[1] = make_shared<Object>(0);
                            objectsNeg[2] = objects[0];
                            ircodeNeg = make_shared<IRCode>(MINUSOT, objectsNeg);
                        }
                    }
                } else {
                    ircode = make_shared<IRCode>(DIVOT, objects);
                }
#else
                    ircode = make_shared<IRCode>(DIVOT, objects);
#endif
                } else if (types[i - 1] == MOD) {
#ifdef OPTIMIZE_MOD
                    if(objects[1]->getCategory() != 5 and objects[2]->getCategory() == 5) {

                    int result, shPosition, leftBit;
                    shared_ptr<IRCode> ircodeMagic, ircode1, ircode2;
                    long long magicNumber = getMagicNumber(BigInteger(abs(objects[2]->getNum())), leftBit, shPosition);
                    cout << "magicNumber: " << magicNumber << endl;
                    cout << "shPosition: " << shPosition << endl;
                        // objects1 for positive or negative
                        shared_ptr<Object> objectsMagic[3], objects1[3], objects2[3];
                        objectsMagic[0] = objects[0];
                        objectsMagic[1] = make_shared<Object>(magicNumber, "LONG");
                        objectsMagic[2] = objects[1];
                        ircodeMagic = make_shared<IRCode>(MULTHOT, std::move(objectsMagic));
                        IRCodeList.push_back(std::move(ircodeMagic));
                        if (magicNumber >= INF) {
                            shared_ptr<Object> objects2[3];
                            objects2[0] = generateTempVarT();
                            objects2[1] = objects[1];
                            objects2[2] = make_shared<Object>(1LL << 32, "LONG");
                            shared_ptr<VarIRCode> var_temp = make_shared<VarIRCode>(objects2[0]->getName(), 0, false,
                                                                                    INTST,
                                                                                    currentDomain);
                            varIndex += 1;
                            if (currentDomain == 0) {
                                global_offset += 1;
                            }
                            var_temp->setIsTemp(true);
                            var_temp->setSpaceInfo(varIndex, 1);
                            objects2[0]->setVar(var_temp);
                            ircode1 = make_shared<IRCode>(MULTOT, objects2);
                            IRCodeList.push_back(std::move(ircode1));
                            objectsMagic[0] = objects[0];
                            objectsMagic[1] = objects[0];
                            objectsMagic[2] = objects2[0];
                            ircodeMagic = make_shared<IRCode>(MINUSOT, std::move(objectsMagic));
                            IRCodeList.push_back(std::move(ircodeMagic));
                        }
                        if (magicNumber >= INF) {
                            objectsMagic[0] = objects[0];
                            objectsMagic[1] = objects[0];
                            objectsMagic[2] = before;
                            ircodeMagic = make_shared<IRCode>(PLUSOT, std::move(objectsMagic));
                            IRCodeList.push_back(std::move(ircodeMagic));
                        }
                        // >> shPosition
                        objectsMagic[0] = objects[0];
                        objectsMagic[2] = make_shared<Object>(shPosition);
                        objectsMagic[1] = objects[0];
                        ircodeMagic = make_shared<IRCode>(SRAOT, objectsMagic);
                        IRCodeList.push_back(ircodeMagic);
                        // get sign bit
                        objects1[0] = generateTempVarT();
                        objects1[1] = before;
                        objects1[2] = make_shared<Object>(31);
                        shared_ptr<VarIRCode> var_temp = make_shared<VarIRCode>(objects1[0]->getName(), 0, false,
                                                                                    INTST,
                                                                                    currentDomain);
                        varIndex += 1;
                        if (currentDomain == 0) {
                            global_offset += 1;
                        }
                        var_temp->setIsTemp(true);
                        var_temp->setSpaceInfo(varIndex, 1);
                        objects1[0]->setVar(var_temp);
                        ircode1 = make_shared<IRCode>(SRLOT, objects1);
                        IRCodeList.push_back(ircode1);
                        objects2[0] = objects[0];
                        objects2[1] = objects[0];
                        objects2[2] = objects1[0];
                        ircode2 = make_shared<IRCode>(PLUSOT, std::move(objects2));
                        IRCodeList.push_back(std::move(ircode2));
                        if (objects[2]->getNum() < 0) {
                            objects2[0] = objects[0];
                            objects2[1] = make_shared<Object>(0);
                            objects2[2] = objects[0];
                            ircode2 = make_shared<IRCode>(MINUSOT, objects2);
                            IRCodeList.push_back(std::move(ircode2));
                        }
                        shared_ptr<Object> objectsMod[3];
                        shared_ptr<IRCode> irCodeMod;
                        objectsMod[0] = generateTempVarT();
                        objectsMod[1] = make_shared<Object>(objects[2]->getNum());
                        objectsMod[2] = objects[0];
                        shared_ptr<VarIRCode> var_temp2 = make_shared<VarIRCode>(objectsMod[0]->getName(), 0, false,
                                                                                    INTST,
                                                                                    currentDomain);
                        varIndex += 1;
                        if (currentDomain == 0) {
                            global_offset += 1;
                        }
                        var_temp2->setIsTemp(true);
                        var_temp2->setSpaceInfo(varIndex, 1);
                        objectsMod[0]->setVar(var_temp2);
                        irCodeMod = make_shared<IRCode>(MULTOT, objectsMod);
                        IRCodeList.push_back(irCodeMod);
                        objects[1] = before;
                        objects[2] = objectsMod[0];
                        ircode= make_shared<IRCode>(MINUSOT, objects);
                } else {
                    ircode = make_shared<IRCode>(MODOT, objects);
                }
#else
                    ircode = make_shared<IRCode>(MODOT, objects);
#endif
                } else if (types[i-1] == BITAND) {
                    ircode = make_shared<IRCode>(BITANDOT, objects);
                }
#ifndef OPTIMIZE
                IRCodeList.push_back(ircode);
#endif
#ifdef OPTIMIZE
                if (!isConst) {
                    IRCodeList.push_back(ircode);
                    if (ircodeNeg != nullptr) {
                        cout << "ircodeNeg" << endl;
                        IRCodeList.push_back(ircodeNeg);
                    }
                }
#endif
            }
        }
        after->setNum(num);
        return after;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeAddExp(shared_ptr<ast::AddExpAST> &addExp) {
        vector<shared_ptr<MulExpAST> > &mulExps = addExp->getMulExp();
        vector<identifierType> &types = addExp->getAddOp();

        shared_ptr<Object> before, after;

        int num = 0;
        bool isConst = true;
        for (int i = 0; i < mulExps.size(); ++i) {
            before = after;
            after = analyzeMulExp(mulExps[i]);
            if (i == 0) {
                num += after->getNum();
            } else {
                num = (types[i - 1] == PLUS) ? num + after->getNum() : num - after->getNum();
            }
            if (after->getCategory() != 5) {
                isConst = false;
            }

            if (i != 0) {
                shared_ptr<Object> objects[3];
                objects[0] = generateTempVarT();
                objects[1] = before;
                objects[2] = after;
                after = objects[0];
                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, isConst, INTST,
                                                                   currentDomain);
                varIndex += 1;
                if (currentDomain == 0) {
                    global_offset += 1;
                }
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                objects[0]->setVar(var);
                if (isConst) {
                    objects[0]->setCategory(5);
                    objects[0]->setNum(num);
                }
                shared_ptr<IRCode> ircode;
                if (types[i - 1] == PLUS) {
                    ircode = make_shared<IRCode>(PLUSOT, objects);
                } else if (types[i - 1] == MINU) {
                    ircode = make_shared<IRCode>(MINUSOT, objects);
                }
#ifndef OPTIMIZE
                IRCodeList.push_back(ircode);
#else
                if (!isConst) {
                    IRCodeList.push_back(ircode);
                }
#endif
            }
        }
        after->setNum(num);
        return after;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeConstExp(shared_ptr<ast::ConstExpAST> &constExp) {
        shared_ptr<Object> obj = analyzeAddExp(constExp->getAddExp());
        obj->setCategory(5);
        return obj;
    }

    void IRCodeGenerator::analyzeInitVal(shared_ptr<ast::InitValAST> &initVal, shared_ptr<irCode::Object> array[3]) {
        int dimension = array[1]->getVar()->getDimension();
        if (dimension == 0) {
            // int a = 1;
            array[2] = analyzeExp(initVal->getExp());
            array[1]->getVar()->setValue(array[2]);
            IRCodeList.push_back(make_shared<IRCode>(VarOT, array));
        } else if (dimension == 1) {
            int i;
            for (i = 0; i < initVal->getInitVals().size(); ++i) {
                shared_ptr<Object> objects[3];
                objects[0] = make_shared<Object>(array[1]->getName(), i, array[1]->getVar());
                objects[1] = analyzeExp(initVal->getInitVals()[i]->getExp());
                objects[0]->getVar()->getValues().push_back(objects[1]);
                shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                IRCodeList.push_back(ircode);
            }
#ifdef INIT
            if (currentDomain == 0) {
                for (; i < array[1]->getNumericIndex(); ++i) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), i, array[1]->getVar());
                    objects[1] = analyzeExp(initVal->getInitVals()[i]->getExp());
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
            }
#endif
        } else if (dimension == 2) {
            int first_dimension = array[1]->getVar()->getSpectrum()[0];
            int second_dimension = array[1]->getVar()->getSpectrum()[1];
            int i;
            for (i = 0; i < initVal->getInitVals().size(); ++i) {
                int j;
                for (j = 0; j < initVal->getInitVals()[i]->getInitVals().size(); ++j) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), i * second_dimension + j, array[1]->getVar());
                    objects[1] = analyzeExp(
                            initVal->getInitVals()[i]->getInitVals()[j]->getExp());
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
#ifdef INIT
                if (currentDomain == 0) {
                    // fill the rest of the array with zero
                    for (; j < second_dimension; ++j) {
                        shared_ptr<Object> objects[3];
                        objects[0] = make_shared<Object>(array[1]->getName(), i * second_dimension + j,
                                                         array[1]->getVar());
                        objects[1] = make_shared<Object>(0);
                        objects[0]->getVar()->getValues().push_back(objects[1]);
                        shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                        IRCodeList.push_back(ircode);
                    }
                }
#endif
            }
#ifdef INIT
            if (currentDomain == 0) {
                for (int k = i * second_dimension; k < first_dimension * second_dimension; ++k) {
                    shared_ptr<Object> objects[3];
                    objects[0] = make_shared<Object>(array[1]->getName(), k, array[1]->getVar());
                    objects[1] = make_shared<Object>(0);
                    objects[0]->getVar()->getValues().push_back(objects[1]);
                    shared_ptr<IRCode> ircode = make_shared<IRCode>(AssignOT, objects);
                    IRCodeList.push_back(ircode);
                }
            }
#endif
        }
    }

    void IRCodeGenerator::analyzeVarDef(shared_ptr<ast::VarDefAST> &varDef) {
        string name = varDef->getName();
        int dimension = (int) varDef->getConstExps().size();
        vector<shared_ptr<ConstExpAST> > &constExps = varDef->getConstExps();

        vector<int> exps; // for array
        shared_ptr<Object> before;
        shared_ptr<Object> after;

        shared_ptr<Object> objectsVardef[3];

        int space = 1;
        for (auto &constExp: constExps) {
            before = after;
            after = analyzeConstExp(constExp);
            space *= after->getNum();
            exps.push_back(after->getNum());
        }

        shared_ptr<VarIRCode> var;
        if (dimension == 0) {
            var = make_shared<VarIRCode>(name, 0, false, defineType.top(), currentDomain);
        } else {
            var = make_shared<VarIRCode>(name, dimension, false, exps, defineType.top(), currentDomain);
        }
        varIndex += space;
        var->setSpaceInfo(varIndex, space);
        irCodeTable.getVars().push_back(var);

        if (varDef->getInitVal() != nullptr) {
            if (varDef->getInitVal()->getInitVals().empty()) {
                // int a = 1;
                shared_ptr<Object> array[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, var)};
                analyzeInitVal(varDef->getInitVal(), array);
            } else {
                // int a[2] = {1, 2};
                shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()),
                                                 make_shared<Object>(name, space, var)};
                shared_ptr<IRCode> ircode = make_shared<IRCode>(ArrayOT, objects);
                IRCodeList.push_back(ircode);
                analyzeInitVal(varDef->getInitVal(), objects);
            }
        } else {
            // do not have the initial value
            var->setDefinedValue(true);
            if (dimension == 0) {
                objectsVardef[0] = make_shared<Object>(defineType.top());
                objectsVardef[1] = make_shared<Object>(name, var);
                if (currentDomain == 0) {
                    // global var initialization
                    objectsVardef[2] = make_shared<Object>(0);
                    objectsVardef[1]->getVar()->setValue(0);
                }
                IRCodeList.push_back(make_shared<IRCode>(VarOT, objectsVardef));
            } else {
                shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()),
                                                 make_shared<Object>(name, space, var)};
                shared_ptr<IRCode> ircode = make_shared<IRCode>(ArrayOT, objects);
                IRCodeList.push_back(ircode);
#ifdef INIT
                if (currentDomain == 0) {
                    // global array initialization
                    for (int i = 0; i < space; ++i) {
                        shared_ptr<Object> objs[3];
                        objs[0] = make_shared<Object>(name, i, var);
                        objs[1] = make_shared<Object>(0);
                        objs[0]->getVar()->getValues().push_back(objs[1]);
                        shared_ptr<IRCode> tmp = make_shared<IRCode>(AssignOT, objs);
                        IRCodeList.push_back(tmp);
                    }
                }
#endif
            }
        }
        if (varDef->getGetInt()) {
            shared_ptr<Object> getint[3] = {generateTempVarT()};
            shared_ptr<VarIRCode> var = make_shared<VarIRCode>(getint[0]->getName(), 0, false, INTST,
                                                               currentDomain);
            varIndex += 1;
            var->setIsTemp(true);
            var->setSpaceInfo(varIndex, 1);
            getint[0]->setVar(var);
            shared_ptr<IRCode> irCode = make_shared<IRCode>(GetIntOT, getint);
            IRCodeList.push_back(irCode);
            var->setDefinedValue(false);
            shared_ptr<Object> objects[3] = {objectsVardef[1], getint[0]};
            shared_ptr<IRCode> ir_getint = make_shared<IRCode>(AssignOT, objects);
            IRCodeList.push_back(ir_getint);
        }
    }

    void IRCodeGenerator::analyzeVarDecl(shared_ptr<ast::VarDeclAST> &varDecl) {
        if (varDecl->getBType()->getType() == INTTK) {
            defineType.push(INTST);
        }
        for (auto &i: varDecl->getVarDefs()) {
            analyzeVarDef(i);
        }
        defineType.pop();
    }

    void IRCodeGenerator::analyzeDecl(shared_ptr<ast::DeclAST> &decl) {
        if (decl->getType() == 0) {
            shared_ptr<ConstDeclAST> constDecl = dynamic_pointer_cast<ConstDeclAST>(decl->getDecl());
            analyzeConstDecl(constDecl);
        } else {
            shared_ptr<VarDeclAST> varDecl = dynamic_pointer_cast<VarDeclAST>(decl->getDecl());
            analyzeVarDecl(varDecl);
        }
    }

    void IRCodeGenerator::analyzeFuncFParam(shared_ptr<ast::FuncParamAST> &funcFParam,
                                            vector<shared_ptr<irCode::VarIRCode> > &exps) {
        string name = funcFParam->getName();
        int dimension = (int) funcFParam->getConstExps().size();
        if (funcFParam->getBType()->getType() == INTTK) {
            defineType.push(INTST);
        }
        vector<shared_ptr<ConstExpAST> > &constExps = funcFParam->getConstExps();
        if (dimension == 0) {
            shared_ptr<VarIRCode> var = make_shared<VarIRCode>(name, 0, false, defineType.top(), currentDomain);
            irCodeTable.getVars().push_back(var);
            exps.push_back(var);
            varIndex += 1;
            var->setSpaceInfo(varIndex, 1);
            shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, var)};
            IRCodeList.push_back(make_shared<IRCode>(ParamOT, objects));
        } else if (dimension == 1) {
            // para int a[]
            vector<int> dimensionExps = {0};
            shared_ptr<VarIRCode> var = make_shared<VarIRCode>(name, dimension, false, dimensionExps, defineType.top(),
                                                               currentDomain);
            var->setIsArrayPara(true);
            irCodeTable.getVars().push_back(var);
            exps.push_back(var);
            varIndex += 1;
            var->setSpaceInfo(varIndex, 1);
            shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, var)};
            shared_ptr<IRCode> ircode = make_shared<IRCode>(ParamOT, objects);
            IRCodeList.push_back(ircode);
        } else if (dimension == 2) {
            // para int a[2][3]
            shared_ptr<Object> object = analyzeConstExp(constExps[1]);
            vector<int> dimensionExps = {0, (int) object->getNum()};
            shared_ptr<VarIRCode> var = make_shared<VarIRCode>(name, dimension, false, dimensionExps, defineType.top(),
                                                               currentDomain);
            var->setIsArrayPara(true);
            irCodeTable.getVars().push_back(var);
            exps.push_back(var);
            varIndex += 1;
            var->setSpaceInfo(varIndex, 1);
            shared_ptr<Object> objects[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, var)};
            shared_ptr<IRCode> ircode = make_shared<IRCode>(ParamOT, objects);
            IRCodeList.push_back(ircode);
        }
        defineType.pop();
    }

    shared_ptr<Object> IRCodeGenerator::analyzeRelExp(shared_ptr<ast::RelExpAST> &relExp, std::string &label,
                                                      std::string &else_, bool isEq) {
        vector<identifierType> &types = relExp->getRelOp();
        vector<shared_ptr<AddExpAST> > &addExps = relExp->getAddExp();
        if (types.empty()) {
            return analyzeAddExp(addExps[0]);
        } else if (!isEq) {
            shared_ptr<Object> exp = analyzeAddExp(addExps[0]);
            shared_ptr<Object> next;
            for (int i = 0; i < types.size(); ++i) {
                identifierType type = types[i];
                next = analyzeAddExp(addExps[i + 1]);
                if (i == types.size() - 1) {
                    if (exp->getCategory() == 5 and next->getCategory() == 5) {
                        bool ans;
                        if ((exp->getNum() > next->getNum() and (type == GRE or type == GEQ)) or
                            (exp->getNum() < next->getNum() and (type == LSS or type == LEQ)) or
                            (exp->getNum() == next->getNum() and (type == LEQ or type == GEQ)))
                            ans = true;
                        else ans = false;
                        return make_shared<Object>(ans ? 1 : 0);
                    }
                    // do not get the value completely
                    shared_ptr<Object> objects[3] = {exp, next, make_shared<Object>(label + else_)};
                    shared_ptr<IRCode> irCode;
                    if (type == GRE) {
                        irCode = make_shared<IRCode>(BLEOT, objects);
                    } else if (type == GEQ) {
                        irCode = make_shared<IRCode>(BLTOT, objects);
                    } else if (type == LSS) {
                        irCode = make_shared<IRCode>(BGEOT, objects);
                    } else if (type == LEQ) {
                        irCode = make_shared<IRCode>(BGTOT, objects);
                    }
                    IRCodeList.push_back(irCode);
                    return make_shared<Object>(); // category = 0
                } else {
                    if (exp->getCategory() == 5 and next->getCategory() == 5) {
                        bool ans;
                        if ((exp->getNum() > next->getNum() and (type == GRE or type == GEQ)) or
                            (exp->getNum() < next->getNum() and (type == LSS or type == LEQ)) or
                            (exp->getNum() == next->getNum() and (type == LEQ or type == GEQ)))
                            ans = true;
                        else ans = false;
                        exp = make_shared<Object>(ans ? 1 : 0);
                        continue;
                    }
                    shared_ptr<Object> objects[3] = {generateTempVarT(), exp, next};
                    shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, false, INTST,
                                                                       currentDomain);
                    varIndex += 1;
                    var->setIsTemp(true);
                    var->setSpaceInfo(varIndex, 1);
                    objects[0]->setVar(var);
                    shared_ptr<IRCode> irCode;
                    if (type == GRE) {
                        irCode = make_shared<IRCode>(SGTOT, objects);
                    } else if (type == GEQ) {
                        irCode = make_shared<IRCode>(SGEOT, objects);
                    } else if (type == LSS) {
                        irCode = make_shared<IRCode>(SLTOT, objects);
                    } else if (type == LEQ) {
                        irCode = make_shared<IRCode>(SLEOT, objects);
                    }
                    IRCodeList.push_back(irCode);
                    exp = objects[0];
                }
            }
        } else {
            // in the case of a>b == c>d, need to transform comparison to 0 or 1 (bool value)
            shared_ptr<Object> exp = analyzeAddExp(addExps[0]);
            shared_ptr<Object> next;
            for (int i = 0; i < types.size(); i++) {
                identifierType type = types[i];
                next = analyzeAddExp(addExps[i + 1]);
                if (exp->getCategory() == 5 and next->getCategory() == 5) {
                    bool ans;
                    if ((exp->getNum() > next->getNum() and (type == GRE or type == GEQ)) or
                        (exp->getNum() < next->getNum() and (type == LSS or type == LEQ)) or
                        (exp->getNum() == next->getNum() and (type == LEQ or type == GEQ)))
                        ans = true;
                    else ans = false;
                    exp = make_shared<Object>(ans ? 1 : 0);
                }
                shared_ptr<Object> objects[3] = {generateTempVarT(), exp, next};
                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, false, INTST,
                                                                   currentDomain);
                varIndex += 1;
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                objects[0]->setVar(var);
                shared_ptr<IRCode> irCode;
                if (type == GRE) {
                    irCode = make_shared<IRCode>(SGTOT, objects);
                } else if (type == GEQ) {
                    irCode = make_shared<IRCode>(SGEOT, objects);
                } else if (type == LSS) {
                    irCode = make_shared<IRCode>(SLTOT, objects);
                } else if (type == LEQ) {
                    irCode = make_shared<IRCode>(SLEOT, objects);
                }
                IRCodeList.push_back(irCode);
                exp = objects[0];
                if (i == types.size() - 1) {
                    return exp;
                }
            }
        }
        return nullptr;
    }

    shared_ptr<Object> IRCodeGenerator::analyzeEqExp(shared_ptr<ast::EqExpAST> &eqExp, std::string &label,
                                                     std::string &else_) {
        vector<identifierType> &types = eqExp->getEqOp();
        if (types.empty()) {
            shared_ptr<Object> out = analyzeRelExp(eqExp->getRelExp()[0], label, else_, false);
            if (out->getVar() != nullptr) {
                // no print before, e.g. if(!a)
                shared_ptr<Object> objects[3] = {out, make_shared<Object>(0), make_shared<Object>(label + else_)};
                shared_ptr<IRCode> irCode = make_shared<IRCode>(BEQOT, objects);
                IRCodeList.push_back(irCode);
            } else if (out->getCategory() == 5) {
                if (out->getNum() == 0) {
                    shared_ptr<Object> objects[3] = {make_shared<Object>(label + else_)};
                    IRCodeList.push_back(make_shared<IRCode>(JumpOT, objects));
                }
            }
            return out;
        } else {
            shared_ptr<Object> exp = analyzeRelExp(eqExp->getRelExp()[0], label, else_, true);
            shared_ptr<Object> next;
            for (int i = 0; i < types.size(); i++) {
                int type = types[i];
                next = analyzeRelExp(eqExp->getRelExp()[i + 1], label, else_, true);
                if (i == types.size() - 1) {
                    if (exp->getCategory() == 5 and next->getCategory() == 5) {
                        bool ans;
                        shared_ptr<Object> objects[3] = {exp, next, make_shared<Object>(label + else_)};
                        shared_ptr<IRCode> irCode;
                        if ((exp->getNum() == next->getNum() and type == EQL) or
                            (exp->getNum() != next->getNum() and type == NEQ))
                            ans = true;
                        else ans = false;
                        if (type == NEQ) {
                            irCode = make_shared<IRCode>(BEQOT, objects);
                        } else if (type == EQL) {
                            irCode = make_shared<IRCode>(BNEOT, objects);
                        }
                        IRCodeList.push_back(irCode);
                        return make_shared<Object>(ans ? 1 : 0);
                    }
                    shared_ptr<Object> objects[3] = {exp, next, make_shared<Object>(label + else_)};
                    shared_ptr<IRCode> irCode;
                    if (type == NEQ) {
                        irCode = make_shared<IRCode>(BEQOT, objects);
                    } else if (type == EQL) {
                        irCode = make_shared<IRCode>(BNEOT, objects);
                    }
                    IRCodeList.push_back(irCode);
                    return make_shared<Object>();
                } else {
                    if (exp->getCategory() == 5 and next->getCategory() == 5) {
                        bool ans;
                        if ((exp->getNum() == next->getNum() and type == EQL) or
                            (exp->getNum() != next->getNum() and type == NEQ))
                            ans = true;
                        else ans = false;
                        exp = make_shared<Object>(ans ? 1 : 0);
                        continue;
                    }
                }
                shared_ptr<Object> objects[3] = {generateTempVarT(), exp, next};
                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(objects[0]->getName(), 0, false, INTST,
                                                                   currentDomain);
                varIndex += 1;
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                objects[0]->setVar(var);
                shared_ptr<IRCode> irCode;
                if (type == NEQ) {
                    irCode = make_shared<IRCode>(SNEOT, objects);
                } else if (type == EQL) {
                    irCode = make_shared<IRCode>(SEQOT, objects);
                }
                IRCodeList.push_back(irCode);
                exp = objects[0];
            }
        }
        return nullptr;
    }

    void IRCodeGenerator::analyzeLAndExp(shared_ptr<ast::LAndExpAST> &lAndExp, std::string &label, std::string &else_) {
        vector<shared_ptr<ast::EqExpAST> > &eqExps = lAndExp->getEqExp();
        for (auto &eqExp: eqExps) {
            shared_ptr<Object> result = analyzeEqExp(eqExp, label, else_);
            if (result->getCategory() == 5) {
                if (result->getNum() == 0)
                    break;
            }
        }
    }

    void IRCodeGenerator::analyzeLOrExp(shared_ptr<ast::LOrExpAST> &lOrExp, std::string &label, std::string &else_) {
        vector<shared_ptr<LAndExpAST> > &lAndExps = lOrExp->getLAndExp();
        if (lAndExps.size() == 1) {
            analyzeLAndExp(lAndExps[0], label, else_);
            return;
        }
        for (int i = 0; i < lAndExps.size(); ++i) {
            if (i != lAndExps.size() - 1) {
                string tmp = "else_" + to_string(i);
                analyzeLAndExp(lAndExps[i], label, tmp);
                shared_ptr<Object> objects[3] = {make_shared<Object>(label + "begin")};
                shared_ptr<IRCode> irCode = make_shared<IRCode>(JumpOT, objects);
                IRCodeList.push_back(irCode);
                objects[0] = make_shared<Object>(label + tmp);
                IRCodeList.push_back(make_shared<IRCode>(LabelOT, objects));
            } else {
                analyzeLAndExp(lAndExps[i], label, else_);
            }
        }
    }

    void IRCodeGenerator::analyzeCond(shared_ptr<ast::CondAST> &cond, std::string &label, std::string &else_) {
        analyzeLOrExp(cond->getLOrExp(), label, else_);
    }

    void IRCodeGenerator::analyzeStmt(shared_ptr<ast::StmtAST> &stmt) {
        switch (stmt->getType()) {
            case 0: {
                // lval
                shared_ptr<Object> lval = analyzeLVal(stmt->getLVal());
                cout << "lval over : " << lval->getName() << endl;
                shared_ptr<Object> exp = analyzeExp(stmt->getExp());
                cout << "exp over : " << exp->getName() << endl;
                lval->getVar()->setValue(exp);
                shared_ptr<Object> objects[3] = {lval, exp};
                shared_ptr<IRCode> irCode = make_shared<IRCode>(AssignOT, objects);
                IRCodeList.push_back(irCode);
                break;
            }
            case 1: {
                //[exp]
                if (stmt->getExpSingle() != nullptr) {
                    analyzeExp(stmt->getExpSingle());
                }
                cout << "ExpSingle OVer" << endl;
                break;
            }
            case 2: {
                // bock;
                ++currentDomain;
                analyzeBlock(stmt->getBlock(), false);
                --currentDomain;
                break;
            }
            case 3: {
                shared_ptr<CondAST> cond_if = stmt->getCondIfExps();
                shared_ptr<StmtAST> stmt_if = stmt->getStmtIf();
                shared_ptr<StmtAST> stmt_else = stmt->getStmtElse();
                string elseFlag = stmt_else == nullptr ? "end" : "else_";
                string label = generateNewLabel(IFST);
                analyzeCond(cond_if, label, elseFlag);
                shared_ptr<Object> objects[3] = {make_shared<Object>(label + "begin")};
                shared_ptr<IRCode> irCode = make_shared<IRCode>(LabelOT, objects);
                IRCodeList.push_back(irCode);
                analyzeStmt(stmt_if);
                if (stmt_else != nullptr) {
                    objects[0] = make_shared<Object>(label + "end");
                    irCode = make_shared<IRCode>(JumpOT, objects);
                    IRCodeList.push_back(irCode);
                    objects[0] = make_shared<Object>(label + "else_");
                    irCode = make_shared<IRCode>(LabelOT, objects);
                    IRCodeList.push_back(irCode);
                    analyzeStmt(stmt_else);
                }
                objects[0] = make_shared<Object>(label + "end");
                irCode = make_shared<IRCode>(LabelOT, objects);
                IRCodeList.push_back(irCode);
                break;
            }
            case 4: {
                shared_ptr<CondAST> cond_while = stmt->getCondWhile();
                shared_ptr<StmtAST> stmt_while = stmt->getStmtWhile();
                string label = generateNewLabel(WHILEST);
                string endFlag = "end";
                stack_while_begin.push(make_shared<Object>(label + "head"));
                stack_while_end.push(make_shared<Object>(label + "end"));
                // label
                shared_ptr<Object> objects[3] = {make_shared<Object>(label + "head")};
                shared_ptr<IRCode> irCode = make_shared<IRCode>(LabelOT, objects);
                IRCodeList.push_back(irCode);
                analyzeCond(cond_while, label, endFlag);
                objects[0] = make_shared<Object>(label + "begin");
                irCode = make_shared<IRCode>(LabelOT, objects);
                IRCodeList.push_back(irCode);
                analyzeStmt(stmt_while);
                objects[0] = make_shared<Object>(label + "head");
                irCode = make_shared<IRCode>(JumpOT, objects);
                IRCodeList.push_back(irCode);
                objects[0] = make_shared<Object>(label + "end");
                irCode = make_shared<IRCode>(LabelOT, objects);
                IRCodeList.push_back(irCode);

                stack_while_end.pop();
                stack_while_begin.pop();
                break;
            }
            case 5: {
                identifierType category = stmt->getCategory();
                shared_ptr<Object> objects[3];
                objects[0] = category == BREAKTK ? stack_while_end.top() : stack_while_begin.top();
                shared_ptr<IRCode> irCode = make_shared<IRCode>(JumpOT, objects);
                IRCodeList.push_back(irCode);
                break;
            }
            case 6: {
                if (isMain) {
                    shared_ptr<Object> decls[3];
                    shared_ptr<IRCode> irCode = make_shared<IRCode>(ExitOT, decls);
                    IRCodeList.push_back(irCode);
                } else {
                    if (stmt->getExpReturn() != nullptr) {
                        shared_ptr<Object> objects[3] = {analyzeExp(stmt->getExpReturn())};
                        shared_ptr<IRCode> irCode = make_shared<IRCode>(ReturnOT, objects);
                        IRCodeList.push_back(irCode);
                    } else {
                        shared_ptr<Object> objects[3];
                        shared_ptr<IRCode> irCode = make_shared<IRCode>(ReturnOT, objects);
                        IRCodeList.push_back(irCode);
                    }
                }
                break;
            }
            case 7: {
                // lval = getinit
                shared_ptr<Object> getint[3] = {generateTempVarT()};
                shared_ptr<VarIRCode> var = make_shared<VarIRCode>(getint[0]->getName(), 0, false, INTST,
                                                                   currentDomain);
                varIndex += 1;
                var->setIsTemp(true);
                var->setSpaceInfo(varIndex, 1);
                getint[0]->setVar(var);
                shared_ptr<IRCode> irCode = make_shared<IRCode>(GetIntOT, getint);
                IRCodeList.push_back(irCode);
                shared_ptr<Object> lval = analyzeLVal(stmt->getLValGetinit());
                lval->getVar()->setDefinedValue(false);
                shared_ptr<Object> objects[3] = {lval, getint[0]};
                shared_ptr<IRCode> ir_getint = make_shared<IRCode>(AssignOT, objects);
                IRCodeList.push_back(ir_getint);
                break;
            }
            case 8: {
                // print
                analyzePrint(stmt->getFormat(), stmt->getExpPrint());
                break;
            }
            default: {
                break;
            }
        }
    }

    void IRCodeGenerator::analyzePrint(std::string &format, vector<shared_ptr<ast::ExpAST>> &exps) {
        cout << "in print" << std::endl;
        static int printLabel = -1;
        vector<shared_ptr<Object> > print;
        for (auto &exp: exps) {
            shared_ptr<Object> object = analyzeExp(exp);
            print.push_back(object);
        }
        vector<OutputInfo> divided;
        // deal the format string
        int pos = 1;
        for (int i = 1; i < format.size() - 2; ++i) {
            if (format[i] == '%') {
                if (format[i + 1] == 'd') {
                    divided.push_back(OutputInfo{format.substr(pos, i - pos), true});
                    pos = i + 2;
                }
            } else if (format[i] == '\\') {
                if (format[i + 1] == 'n') {
                    divided.push_back(OutputInfo{format.substr(pos, i - pos), false});
                    divided.emplace_back(OutputInfo{"\\n", false});
                    pos = i + 2;
                }
            }
        }
        divided.push_back(OutputInfo{format.substr(pos, format.size() - 1 - pos), true});
        int index = 0;
        for (int i = 0; i < divided.size(); i++) {
            if (!divided[i].content.empty()) {
                if (divided[i].content == "\\n") {
                    shared_ptr<Object> objects[3] = {make_shared<Object>("endl")};
                    shared_ptr<IRCode> irCode = make_shared<IRCode>(PrintOT, objects);
                    IRCodeList.push_back(irCode);
                } else {
                    AttributeData data;
                    data.content = divided[i].content;
                    data.label = "str_label_" + to_string(++printLabel);
                    attributeDataList.push_back(data);
                    shared_ptr<Object> objects[3] = {make_shared<Object>(data.label)};
                    IRCodeList.push_back(make_shared<IRCode>(PrintOT, objects));
                }
            }
            if (i != divided.size() - 1 and divided[i].isOutput) {
                shared_ptr<Object> objects[3] = {print[index++]};
                IRCodeList.push_back(make_shared<IRCode>(PrintIntOT, objects));
            }
        }
    }

    void IRCodeGenerator::analyzeBlockItem(shared_ptr<ast::BlockItemAST> &blockItem) {
        if (blockItem->getType() == 0) {
            shared_ptr<ast::DeclAST> decl = dynamic_pointer_cast<DeclAST>(blockItem->getBlockItem());
            analyzeDecl(decl);
        } else {
            shared_ptr<ast::StmtAST> stmt = dynamic_pointer_cast<StmtAST>(blockItem->getBlockItem());
            analyzeStmt(stmt);
        }
    }

    void IRCodeGenerator::analyzeBlock(shared_ptr<ast::BlockAST> &block, bool isFunc) {
        for (auto &blockItem: block->getBlockItems()) {
            //cout << "--------------------------------" << " BlockItem "
            //     << find(block->getBlockItems().begin(), block->getBlockItems().end(), blockItem) -
             //       block->getBlockItems().begin() << " --------------------------------" << endl;
            analyzeBlockItem(blockItem);
            // cout << "Done!" << endl;
        }
        vector<shared_ptr<BlockItemAST> > &blockItems = block->getBlockItems();
        // table for the block
        for (auto i = irCodeTable.getVars().begin(); i != irCodeTable.getVars().end(); ++i) {
            if ((*i)->getDomain() == currentDomain or (*i)->getDomain() == -1) {
                irCodeTable.getVars().erase(i);
                --i;
            }
        }

        if (blockItems.empty()) {
            if (isFunc) {
                shared_ptr<Object> decls[3];
                shared_ptr<IRCode> irCode = make_shared<IRCode>(ReturnOT, decls);
                IRCodeList.push_back(irCode);
            }
            return;
        }
        shared_ptr<BlockItemAST> endItem = blockItems.back();
        if (endItem->getType() == 0) {
            if (isFunc) {
                shared_ptr<Object> decls[3];
                shared_ptr<IRCode> irCode = make_shared<IRCode>(ReturnOT, decls);
                IRCodeList.push_back(irCode);
            }
        } else {
            shared_ptr<StmtAST> stmt = dynamic_pointer_cast<StmtAST>(endItem->getBlockItem());
            if (stmt->getType() != 6 and isFunc) {
                // no return
                if (isFunc) {
                    shared_ptr<Object> decls[3];
                    shared_ptr<IRCode> irCode = make_shared<IRCode>(ReturnOT, decls);
                    IRCodeList.push_back(irCode);
                }
            }
        }
    }

    void IRCodeGenerator::analyzeFuncDef(shared_ptr<ast::FuncDefAST> &funcDef) {
        string name = funcDef->getName();
        if (funcDef->getFuncType()->getCategory() == INTTK) {
            defineType.push(INTST);
        } else {
            defineType.push(VOIDST);
        }
        ++currentDomain;
        shared_ptr<FuncFParamsAST> &funcParams = funcDef->getParams();
        shared_ptr<FuncIRCode> func;
        if (!irCodeTable.getFuncs().empty()) {
            int size = (int) irCodeTable.getFuncs().size();
            irCodeTable.getFuncs()[size - 1]->setOffset(varIndex);
        }
        varIndex = 0;
        if (funcParams != nullptr) {
            vector<shared_ptr<VarIRCode> > exps;
            for (auto &i: funcParams->getFuncParams()) {
                analyzeFuncFParam(i, exps);
            }
            cout << "Index " << varIndex << endl;
            func = make_shared<FuncIRCode>(name, defineType.top(), exps);
            irCodeTable.getFuncs().push_back(func);
        } else {
            func = make_shared<FuncIRCode>(name, defineType.top());
            irCodeTable.getFuncs().push_back(func);
        }
        shared_ptr<Object> decls[3] = {make_shared<Object>(defineType.top()), make_shared<Object>(name, func)};
        IRCodeList.push_back(make_shared<IRCode>(FuncOT, decls));

        analyzeBlock(funcDef->getBlock(), true);
        cout << "Block Index " << varIndex << endl;
        --currentDomain;
        defineType.pop();
    }

    void IRCodeGenerator::analyzeMainFuncDef(shared_ptr<ast::MainFuncDefAST> &mainFuncDef) {
        cout << "IN" << endl;
        defineType.push(INTST);
        ++currentDomain;
        if (!irCodeTable.getFuncs().empty()) {
            irCodeTable.getFuncs()[irCodeTable.getFuncs().size() - 1]->setOffset(varIndex);
        }
        shared_ptr<FuncIRCode> func = make_shared<FuncIRCode>("main", defineType.top());
        varIndex = 0;
        irCodeTable.getFuncs().push_back(func);
        shared_ptr<Object> decls[3] = {make_shared<Object>(defineType.top()), make_shared<Object>("main", func)};
        shared_ptr<IRCode> irCode = make_shared<IRCode>(FuncOT, decls);
        IRCodeList.push_back(irCode);
        analyzeBlock(mainFuncDef->getBlock(), false);
        --currentDomain;
        defineType.pop();
    }

    void IRCodeGenerator::analyze() {
        vector<shared_ptr<DeclAST> > &decls = compUnitAST->getDecls();
        for (auto &decl: decls) {
            analyzeDecl(decl);
        }
        cout << "decl over " << endl;
        shared_ptr<Object> objects[3];
        shared_ptr<IRCode> irCode = make_shared<IRCode>(MainOT, objects);
        IRCodeList.push_back(irCode);
        vector<shared_ptr<FuncDefAST> > &funcDefs = compUnitAST->getFuncDefs();
        for (auto &funcDef: funcDefs) {
            cout << "Begin to analyze funcDef: " << funcDef->getName() << endl;
            analyzeFuncDef(funcDef);
        }
        cout << "func over " << endl;
        isMain = true;
        if (compUnitAST->getMainFuncDef() != nullptr) {
            analyzeMainFuncDef(compUnitAST->getMainFuncDef());
            irCodeTable.getFuncs()[irCodeTable.getFuncs().size() - 1]->setOffset(varIndex);
        }
        cout << "main over " << endl;
        isMain = false;
#ifdef IRCODE
        for (auto &i: IRCodeList) {
            //cout << i->getObj(0)->getName() << " " << i->getObj(1)->getCategory() << endl;
            i->Print(output);
        }
        output.close();
#endif
    }


}