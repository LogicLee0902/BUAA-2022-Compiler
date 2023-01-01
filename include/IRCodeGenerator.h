//
// Created by Leo on 2022/10/23.
//

#ifndef COMPILER_IRCODEGENERATOR_H
#define COMPILER_IRCODEGENERATOR_H

#include <utility>
#include <iostream>
#include <memory>
#include "AST.h"
#include "PRELOAD.h"

using namespace std;
using namespace ast;

namespace irCode {
    struct AttributeData {
        string label;
        string content;
    };

    class FuncIRCode;

    class VarIRCode;

    class IRCode;

    class Object;

    class IRCodeTable;

    extern vector<shared_ptr<IRCode> > IRCodeList;
    extern IRCodeTable irCodeTable;
    extern vector<AttributeData> attributeDataList;

    class IRCodeTable {
    private:
        vector<shared_ptr<VarIRCode> > vars;
        vector<shared_ptr<FuncIRCode> > funcs;
    public:
        vector<shared_ptr<VarIRCode> > &getVars() { return vars; }

        vector<shared_ptr<FuncIRCode> > &getFuncs() { return funcs;}
    };

    class FuncIRCode {
    private:
        string name;
        SymbolType returnType;
        vector<shared_ptr<VarIRCode> > exps;
        int offset;
    public:
        FuncIRCode(string name, SymbolType returnType, vector<shared_ptr<VarIRCode> > &exps) :
                name(std::move(name)), returnType(returnType), exps(exps) {}

        FuncIRCode(string name, SymbolType returnType) : name(std::move(name)), returnType(returnType){}

        void setOffset(int offset_t) { this->offset = offset_t; }

        string getName() const { return name; }

        SymbolType getReturnType() const { return returnType; }

        int getOffset() const { return offset; }
        vector<shared_ptr<VarIRCode> > &getExps() { return exps; }
    };

    class VarIRCode {
    private:
        string name;
        SymbolType type;
        int dimension;
        int domain;
        int originDomain;
        vector<int> spectrum;
        //single var
        shared_ptr<Object> value;
        //array
        vector<shared_ptr<Object> > values;
        int offset;
        int space;
        bool definedValue;
        bool isArrayPara;
        bool isConst;
        bool isUse;
        bool isTemp;

    public:
        VarIRCode(string name, int dimension, bool isConst, vector<int> &spectrum, SymbolType type, int domain);

        VarIRCode(string name, int dimension, bool isConst, shared_ptr<Object> &value, SymbolType type, int domain);

        VarIRCode(string name, int dimension, bool isConst, SymbolType type, int domain);

        void setValue(int _value);

        void setValue(shared_ptr<Object> _value);

        void setSpaceInfo(int _offset, int _space);

        int getDimension() const {return  dimension;}

        vector<shared_ptr<Object> >& getValues() {return values;}

        vector<int>& getSpectrum() {return spectrum;}

        string getName() const {return name;}

        shared_ptr<Object>& getValue() {return value;}

        bool getIsConst() const {return isConst;}

        void setDefinedValue(bool defined);

        void setOffset(int _offset);

        int getDomain() const {return domain;}

        int getOriginDomain() const {return originDomain;}

        int getOffset() const {return offset;}

        void setIsArrayPara(bool arrayPara);

        bool getIsArrayPara() const {return isArrayPara;}

        void setIsUse(bool use);

        bool getIsUse() const {return isUse;}

        void setIsTemp(bool temp);

        bool getIsTemp() const {return isTemp;}

        bool operator < (const VarIRCode &varIRCode) const {
          return this->offset < varIRCode.offset;
        };
    };

    class Object {
        // generic class for all kinds of objects
    private:
        /*
         * 0 for null
         * 1 for type:int, void
         * 2 for name of label
         * 3 for array: name[index] (index can be anything)
         * 4 for array, also but index is int32
         * 5 for num
         * 6 or long long num
         */
        int category;
        string name;
        int numericIndex;
        int num;
        long long longNum;
        shared_ptr<Object> index;
        SymbolType type;

        shared_ptr<VarIRCode> var;
        shared_ptr<FuncIRCode> func;

    public:
        Object() : category(0) {}

        explicit Object(SymbolType type) : category(1), type(type) {}

        explicit Object(string name) : category(2), name(std::move(name)) {}

        Object(string name, shared_ptr<VarIRCode> &var) : category(2), name(std::move(name)), var(var) {}

        Object(string name, shared_ptr<FuncIRCode> &func) : category(2), name(std::move(name)), func(func) {}

        Object(string name, shared_ptr<Object> index) : category(3), name(std::move(name)), index(std::move(index)) {}

        Object(string name, int numericIndex) : category(4), name(std::move(name)), numericIndex(numericIndex) {}

        Object(string name, shared_ptr<Object> &obj, shared_ptr<VarIRCode> &var) {
            this->var = var;
            if (obj->category == 2 or obj->category == 3 or obj->category == 4) {
                this->category = 3;
                this->index = obj;
            } else if (obj->category == 5) {
                this->category = 4;
                this->numericIndex = obj->num;
            }
            this->name = std::move(name);
        }

        Object(string name, int numericIndex, shared_ptr<VarIRCode> &var) : category(4), name(std::move(name)),
                                                                            numericIndex(numericIndex), var(var) {}

        explicit Object(int num) : category(5), num(num) {}

        explicit Object(long long longnum, string _) : category(6), longNum(longnum) {}

        Object(Object &t) {
            this->category = t.category;
            this->name = t.name;
            this->numericIndex = t.numericIndex;
            this->num = t.num;
            this->index = t.index;
            this->type = t.type;
            this->var = t.var;
            this->func = t.func;
        }

        string Print();

        shared_ptr<FuncIRCode> &getFunc() { return func; }

        shared_ptr<VarIRCode> &getVar() { return var; }

        string getName() const { return name; }

        int getNumericIndex() const {return numericIndex; }

        int getNum() const {return num; }

        int getCategory() const {return category; }

        long long getLongNum() const {return longNum; }

        void setNum(int number);

        void setCategory(int _category);

        void setVar(shared_ptr<VarIRCode> &_var);

        shared_ptr<Object> &getIndex() { return index; }
    };

    class IRCode {
    private:
        OperatorType op;
        shared_ptr<Object> obj[3];

    public:
        IRCode(OperatorType op, shared_ptr<Object> obj[3]) {
            this->op = op;
            for (int i = 0; i < 3; i++) {
                this->obj[i] = obj[i];
            }
        }

        void printObject(int num, fstream& output);

        OperatorType getOp() const {return op;}

        shared_ptr<Object>& getObj(int i) {return obj[i];}

        void Print(fstream &output);
    };

    class IRCodeGenerator {
    private:
        shared_ptr<CompUnitAST> &compUnitAST;
        int currentDomain;
        //const char *IR_FILE = "ir.txt";
        string IR_FILE;
        fstream output;
    public:
        IRCodeGenerator(shared_ptr<CompUnitAST> &compUnitAST, const string &filename);

        string generateNewLabel(SymbolType type);

        static shared_ptr<Object> generateTempVarT();

        void analyze();

        void analyzeDecl(shared_ptr<DeclAST> &decl);

        void analyzeConstDef(shared_ptr<ConstDefAST>& constDef);

        void analyzeConstDecl(shared_ptr<ConstDeclAST>& constDecl);

        void analyzeVarDef(shared_ptr<VarDefAST> &varDef);

        void analyzeVarDecl(shared_ptr<VarDeclAST> &varDecl);

        void analyzeFuncDef(shared_ptr<FuncDefAST> &funcDef);

        void analyzeFuncFParam(shared_ptr<FuncParamAST> &funcFParam, vector<shared_ptr<VarIRCode> > &exps);

        void analyzeBlock(shared_ptr<BlockAST> &block, bool isFunction);

        void analyzeBlockItem(shared_ptr<BlockItemAST> &blockItem);

        void analyzeStmt(shared_ptr<StmtAST> &stmt);

        shared_ptr<Object> analyzeLVal(shared_ptr<LValAST> &lval);

        void analyzeMainFuncDef(shared_ptr<MainFuncDefAST> &mainFuncDef);

        void analyzePrint(string &format, vector<shared_ptr<ExpAST> > &exps);

        void analyzeInitVal(shared_ptr<InitValAST> &initVal, shared_ptr<Object> array[3]);

        void analyzeConstInitVal(shared_ptr<ConstInitValAST> &constInitVal, shared_ptr<Object> array[3]);

        shared_ptr<Object> analyzeExp(shared_ptr<ExpAST> &exp);

        shared_ptr<Object> analyzeConstExp(shared_ptr<ConstExpAST> &constExp);

        shared_ptr<Object> analyzeAddExp(shared_ptr<AddExpAST> &addExp);

        shared_ptr<Object> analyzeMulExp(shared_ptr<MulExpAST> &mulExp);

        shared_ptr<Object> analyzeUnaryExp(shared_ptr<UnaryExpAST> &unaryExp);

        shared_ptr<Object> analyzePrimaryExp(shared_ptr<PrimaryExpAST> &primaryExp);

        shared_ptr<Object> analyzeRelExp(shared_ptr<RelExpAST> &relExp, string &label, string &else_, bool isEq);

        shared_ptr<Object> analyzeEqExp(shared_ptr<EqExpAST> &eqExp, string &label, string &else_);

        void analyzeLAndExp(shared_ptr<LAndExpAST> &lAndExp, string &label, string &else_);

        void analyzeLOrExp(shared_ptr<LOrExpAST> &lOrExp, string &label, string &else_);

        void analyzeCond(shared_ptr<CondAST> &cond, string &label, string &else_);
    };
}

#endif //COMPILER_IRCODEGENERATOR_H