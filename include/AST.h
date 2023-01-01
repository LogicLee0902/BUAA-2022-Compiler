//
// Created by Leo on 2022/9/27.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <iostream>
#include <memory>
#include <utility>
#include "Lexer.h"

namespace ast {
    using namespace std;

    class AST {
    private:
    public:
        virtual ~AST() = default;
    };

    class MulExpAST;

    class BlockAST;

    class ConstDefAST;

    class BtypeAST : public AST {
    private:
        identifierType category;
        string type;
    public:
        BtypeAST(identifierType category, string type) : category(category), type(std::move(type)) {}

        identifierType getType() const { return category; }
    };

    class ConstDeclAST : public AST {
    private:
        //for type;
        shared_ptr<BtypeAST> btype;
        vector<shared_ptr<ConstDefAST>> constDefs;
    public:
        ConstDeclAST(shared_ptr<BtypeAST> btype, vector<shared_ptr<ConstDefAST>> constDefs) :
                btype(std::move(btype)), constDefs(std::move(constDefs)) {}

        vector<shared_ptr<ConstDefAST> > &getConstDefs() { return constDefs; }

        shared_ptr<BtypeAST> &getBtype() { return btype; }
    };

    class AddExpAST : public AST {
    private:
        vector<shared_ptr<MulExpAST> > mulExp;
        /*
         * 0 means +
         * 1 means -
         */
        vector<identifierType> type;
    public:
        AddExpAST(vector<shared_ptr<MulExpAST>> mulExp, vector<identifierType> type) :
                mulExp(std::move(mulExp)), type(std::move(type)) {}

        vector<shared_ptr<MulExpAST> > &getMulExp() { return mulExp; }

        vector<identifierType> &getAddOp() { return type; }
    };

    class ExpAST : public AST {
    private:
        shared_ptr<AddExpAST> addExp;
    public:
        explicit ExpAST(shared_ptr<AddExpAST> addExp) : addExp(std::move(addExp)) {}

        shared_ptr<AddExpAST> &getAddExp() { return addExp; }
    };

    class FuncRParamsAST : public AST {
    private:
        vector<shared_ptr<ExpAST> > exps;
    public:
        explicit FuncRParamsAST(vector<shared_ptr<ExpAST>> exps) : exps(std::move(exps)) {}

        vector<shared_ptr<ExpAST> > &getExps() { return exps; }
    };

    class PrimaryExpAST : public AST {
    private:
        shared_ptr<AST> exp;
        /*
         * 0 means Exp
         * 1 means LVal
         * 2 means Number
         */
        int type;
    public:
        explicit PrimaryExpAST(shared_ptr<AST> exp, int type) : exp(std::move(exp)), type(type) {}

        int getType() const { return type; }

        shared_ptr<AST> &getExp() { return exp; }
    };

    class UnaryOpAST : public AST {
    private:
        /*
         * one for +
         * one for -
         * one for !
         */
        identifierType type;
    public:
        explicit UnaryOpAST(identifierType type) : type(type) {}
        identifierType getType() const { return type;}
    };

    class UnaryExpAST : public AST {
    private:
        shared_ptr<PrimaryExpAST> primaryExp;
        string name;
        shared_ptr<FuncRParamsAST> funcRParams;
        shared_ptr<UnaryOpAST> unaryOp;
        shared_ptr<UnaryExpAST> unaryExp;
        /*
         * 0 means primaryExp
         * 1 means funcCall
         * 2 means unary
         */
        int type;
        int line;
    public:
        explicit UnaryExpAST(shared_ptr<PrimaryExpAST> primaryExp, int type = 0) : primaryExp(std::move(primaryExp)),
                                                                                   type(type) {}

        UnaryExpAST(string name, shared_ptr<FuncRParamsAST> funcRParams, int line, int type = 1) :
                name(std::move(name)), funcRParams(std::move(funcRParams)), line(line), type(type) {}

        UnaryExpAST(shared_ptr<UnaryOpAST> unaryOp, shared_ptr<UnaryExpAST> unaryExp, int type = 2) :
                unaryOp(std::move(unaryOp)), unaryExp(std::move(unaryExp)), type(type) {}

        int getType() const { return type; }

        string getName() const { return name; }

        shared_ptr<FuncRParamsAST> &getFuncRParams() { return funcRParams; }

        shared_ptr<PrimaryExpAST> &getPrimaryExp() { return primaryExp; }

        shared_ptr<UnaryExpAST> &getUnaryExp() { return unaryExp; }

        shared_ptr<UnaryOpAST> &getUnaryOp() { return unaryOp; }

        int getLine() const { return line; }
    };

    class MulExpAST : public AST {
    private:
        vector<shared_ptr<UnaryExpAST> > unaryExp;
        /*
         * one for *
         * one for /
         * one for %
        */
        vector<identifierType> type;
    public:
        MulExpAST(vector<shared_ptr<UnaryExpAST>> unaryExp, vector<identifierType> type) :
                unaryExp(std::move(unaryExp)), type(std::move(type)) {}

        vector<shared_ptr<UnaryExpAST> > &getUnaryExp() { return unaryExp; }

        vector<identifierType>& getMulOp() { return type; }
    };

    class RelExpAST : public AST {
    private:
        /*
         * one for <
         * one for >
         * one for <=
         * one for >=
         */
        vector<identifierType> type;
        vector<shared_ptr<AddExpAST>> addExp;
    public:
        RelExpAST(vector<identifierType> type, vector<shared_ptr<AddExpAST>> addExp) :
                type(std::move(type)), addExp(std::move(addExp)) {}

        vector<shared_ptr<AddExpAST> > &getAddExp() { return addExp; }
        vector<identifierType> &getRelOp() { return type; }
    };

    class EqExpAST : public AST {
    private:
        /*
         * one for ==
         * one for !=
         */
        vector<identifierType> type;
        vector<shared_ptr<RelExpAST>> relExp;
    public:
        EqExpAST(vector<identifierType> type, vector<shared_ptr<RelExpAST>> relExp) :
                type(std::move(type)), relExp(std::move(relExp)) {}

        vector<shared_ptr<RelExpAST> > &getRelExp() { return relExp; }
        vector<identifierType> &getEqOp() { return type; }
    };

    class ConstExpAST : public AST {
    private:
        shared_ptr<AddExpAST> addExp;
    public:
        explicit ConstExpAST(shared_ptr<AddExpAST> addExp) : addExp(std::move(addExp)) {}

        shared_ptr<AddExpAST> &getAddExp() { return addExp; }
    };

    class LValAST : public AST {
    private:
        string name;
        int line;
        vector<shared_ptr<ExpAST>> constExps;
    public:
        explicit LValAST(string name, int line, vector<shared_ptr<ExpAST>> constExps = vector<shared_ptr<ExpAST>>())
                : name(std::move(name)), line(line), constExps(std::move(constExps)) {}

        string getName() const { return name; }

        int getLine() const { return line; }

        vector<shared_ptr<ExpAST> > &getConstExps() { return constExps; }
    };

    class ConstInitValAST : public AST {
    private:
        shared_ptr<ConstExpAST> constExp;
        vector<shared_ptr<ConstInitValAST> > constInitVals;
    public:
        explicit ConstInitValAST(shared_ptr<ConstExpAST> constExp) : constExp(std::move(constExp)) {}

        explicit ConstInitValAST(vector<shared_ptr<ConstInitValAST>> constInitVals) : constInitVals(
                std::move(constInitVals)) {}

        shared_ptr<ConstExpAST> &getConstExp() { return constExp; }

        vector<shared_ptr<ConstInitValAST> > &getConstInitVals() { return constInitVals; }
    };

    class ConstDefAST : public AST {
    private:
        string name;
        vector<shared_ptr<ConstExpAST>> constExps;
        shared_ptr<ConstInitValAST> constInitVal;
        int line;
    public:
        ConstDefAST(string name, vector<shared_ptr<ConstExpAST>> constExps, shared_ptr<ConstInitValAST> constInitVal,
                    int line) :
                name(std::move(name)), constExps(std::move(constExps)), constInitVal(std::move(constInitVal)),
                line(line) {}

        string getName() const { return name; }

        vector<shared_ptr<ConstExpAST> > &getConstExps() { return constExps; }

        shared_ptr<ConstInitValAST> &getConstInitVal() { return constInitVal; }

        int getLine() const { return line; }
    };

    class InitValAST : public AST {
    private:
        shared_ptr<ExpAST> exp;
        vector<shared_ptr<InitValAST>> initVals;
    public:
        explicit InitValAST(shared_ptr<ExpAST> exp) : exp(std::move(exp)) {}

        explicit InitValAST(vector<shared_ptr<InitValAST>> initVals) : initVals(std::move(std::move(initVals))) {}

        shared_ptr<ExpAST> &getExp() { return exp; }

        vector<shared_ptr<InitValAST> > &getInitVals() { return initVals; }
    };

    class NumberAST : public AST {
    private:
        long long num;
        string origin;
    public:
        NumberAST(long long num, string origin) : num(num), origin(std::move(origin)) {}
        long long getNum() const { return num; }
    };

    class VarDefAST : public AST {
    private:
        string name;
        vector<shared_ptr<ConstExpAST> > constExps;
        shared_ptr<InitValAST> InitVal;
        int line;
        bool getInt;
    public:
        VarDefAST(string name, vector<shared_ptr<ConstExpAST>> constExps, int line, bool getInt,
                  shared_ptr<InitValAST> InitVal = nullptr) :
                name(std::move(name)), constExps(std::move(constExps)), line(line), getInt(getInt), InitVal(std::move(InitVal)) {}

        string getName() { return name; }

        vector<shared_ptr<ConstExpAST> > &getConstExps() { return constExps; }

        int getLine() const { return line; }

        shared_ptr<InitValAST> &getInitVal() { return InitVal; }

        bool getGetInt() { return getInt; }
    };

    class VarDeclAST : public AST {
    private:
        shared_ptr<BtypeAST> btype;
        vector<shared_ptr<VarDefAST>> varDef;
    public:
        VarDeclAST(shared_ptr<BtypeAST> btype, vector<shared_ptr<VarDefAST>> varDef) :
                btype(std::move(btype)), varDef(std::move(varDef)) {};

        shared_ptr<BtypeAST> &getBType() { return btype; }

        vector<shared_ptr<VarDefAST>> &getVarDefs() { return varDef; }
    };

    class LAndExpAST : public AST {
    private:
        vector<shared_ptr<EqExpAST>> eqExp;
    public:
        explicit LAndExpAST(vector<shared_ptr<EqExpAST>> eqExp) : eqExp(std::move(eqExp)) {}

        vector<shared_ptr<EqExpAST>> &getEqExp() { return eqExp; }
    };

    class LOrExpAST : public AST {
    private:
        vector<shared_ptr<LAndExpAST>> lAndExp;
    public:
        explicit LOrExpAST(vector<shared_ptr<LAndExpAST>> lAndExp) : lAndExp(std::move(lAndExp)) {}

        vector<shared_ptr<LAndExpAST>> &getLAndExp() { return lAndExp; }
    };

    class CondAST : public AST {
    private:
        shared_ptr<LOrExpAST> lOrExp;
    public:
        explicit CondAST(shared_ptr<LOrExpAST> lOrExp) : lOrExp(std::move(lOrExp)) {}

        shared_ptr<LOrExpAST> &getLOrExp() { return lOrExp; }
    };

    class FuncTypeAST : public AST {
    private:
        identifierType category;
        string name;
    public:
        FuncTypeAST(identifierType category, string name) : category(category), name(std::move(name)) {}

        string getName() const { return name; }

        identifierType getCategory() const { return category; }
    };

    class FuncParamAST : public AST {
    private:
        shared_ptr<BtypeAST> btype;
        string name;
        vector<shared_ptr<ConstExpAST> > constExps;
        int line;
    public:
        FuncParamAST(shared_ptr<BtypeAST> btype, string name, int line,
                     vector<shared_ptr<ConstExpAST>> constExps = vector<shared_ptr<ConstExpAST>>()) :
                btype(std::move(btype)), name(std::move(name)), line(line), constExps(std::move(constExps)) {}

        string getName() const { return name; }

        shared_ptr<BtypeAST> &getBType() { return btype; }

        vector<shared_ptr<ConstExpAST> > &getConstExps() { return constExps; }

        int getLine() const { return line; }
    };

    class FuncFParamsAST : public AST {
    private:
        vector<shared_ptr<FuncParamAST>> funcParams;
    public:
        explicit FuncFParamsAST(vector<shared_ptr<FuncParamAST>> funcParams) : funcParams(std::move(funcParams)) {}
        vector<shared_ptr<FuncParamAST>> &getFuncParams() { return funcParams; }
    };

    class DeclAST : public AST {
    private:
        shared_ptr<AST> decl;
        /*
         * 0 means const
         * 1 means var
         */
        int type;
    public:
        DeclAST(shared_ptr<AST> decl, int type) : decl(std::move(decl)), type(type){}

        int getType() const { return type; }

        shared_ptr<AST> &getDecl() { return decl; }
    };

    class StmtAST : public AST {
    private:
        shared_ptr<LValAST> lVal;
        shared_ptr<ExpAST> exp;
        shared_ptr<ExpAST> singleExp;
        shared_ptr<BlockAST> block;
        shared_ptr<CondAST> cond_if;
        shared_ptr<StmtAST> stmt_if;
        shared_ptr<StmtAST> stmt_else;
        shared_ptr<CondAST> cond_while;
        shared_ptr<StmtAST> stmt_while;
        identifierType category;
        shared_ptr<ExpAST> expReturn;
        shared_ptr<LValAST> lvalGetInt;
        string format;
        vector<shared_ptr<ExpAST>> expsPrint;
        /*
         * 0: lVal = exp;
         * 1: singleExp; [Exp]
         * 2: block
         * 3: if (cond) stmt else stmt
         * 4: while (cond) stmt
         * 5 break | continue
         * 6: return exp
         * 7: getint(exp)
         * 8: printf(format, exps)
         */
        int type;
        int line;
    public:
        explicit StmtAST(shared_ptr<LValAST> lVal, shared_ptr<ExpAST> exp, int line, int type = 0) :
                lVal(std::move(lVal)), exp(std::move(exp)), line(line), type(type) {}

        explicit StmtAST(shared_ptr<ExpAST> singleExp, int type = 1) : singleExp(std::move(singleExp)), type(type) {}

        explicit StmtAST(shared_ptr<BlockAST> block, int type = 2) : block(std::move(block)), type(type) {}

        StmtAST(shared_ptr<CondAST> cond_if, shared_ptr<StmtAST> stmt_if, shared_ptr<StmtAST> stmt_else, int type = 3) :
                cond_if(std::move(cond_if)), stmt_if(std::move(stmt_if)), stmt_else(std::move(stmt_else)), type(type) {}

        StmtAST(shared_ptr<CondAST> cond_while, shared_ptr<StmtAST> stmt_while, int type = 4) :
                cond_while(std::move(cond_while)), stmt_while(std::move(stmt_while)), type(type) {}

        explicit StmtAST(const identifierType category, int line, int type = 5) : category(category), line(line),
                                                                                  type(type) {}

        explicit StmtAST(shared_ptr<ExpAST> expReturn, string s, int line, int type = 6) : expReturn(
                std::move(expReturn)), line(line), type(type) {}

        explicit StmtAST(shared_ptr<LValAST> lvalGetInt, int line, int type = 7) : lvalGetInt(std::move(lvalGetInt)),
                                                                                   line(line), type(type) {}

        StmtAST(string format, vector<shared_ptr<ExpAST>> expsPrint, int line, int type = 8) : format(
                std::move(format)), expsPrint(std::move(expsPrint)), line(line), type(type) {}
        int getType() const { return type; }
        int getLine() const { return line; }

        shared_ptr<LValAST>& getLVal() {return lVal;}
        shared_ptr<ExpAST>& getExp() {return exp;}
        shared_ptr<ExpAST>& getExpSingle() {return singleExp;}
        shared_ptr<BlockAST>& getBlock() {return block;}
        shared_ptr<CondAST> & getCondIfExps() { return cond_if;}
        shared_ptr<StmtAST>& getStmtIf() {return stmt_if;}
        shared_ptr<StmtAST>& getStmtElse() {return stmt_else;}
        shared_ptr<CondAST>& getCondWhile() {return cond_while;}
        shared_ptr<StmtAST>& getStmtWhile() {return stmt_while;}
        shared_ptr<ExpAST>& getExpReturn() {return expReturn;}
        shared_ptr<LValAST>& getLValGetinit() {return lvalGetInt;}
        string& getFormat() {return format;}
        vector<shared_ptr<ExpAST> >& getExpPrint() {return expsPrint;}
        identifierType getCategory() const {return category;}
    };

    class BlockItemAST : public AST {
    private:
        shared_ptr<AST> blockItem;
        /*
         * 0: decl
         * 1: stmt
         */
        int type;
    public:
        explicit BlockItemAST(shared_ptr<AST> blockItem, int type) : blockItem(std::move(blockItem)), type(type) {}
        int getType() const { return type; }
        shared_ptr<AST> &getBlockItem() { return blockItem; }
    };

    class BlockAST : public AST {
    private:
        vector<shared_ptr<BlockItemAST> > blockItems;
        int line;
    public:
        explicit BlockAST(vector<shared_ptr<BlockItemAST>> blockItems, int line) :
                blockItems(std::move(blockItems)), line(line) {}
        vector<shared_ptr<BlockItemAST> >& getBlockItems() { return blockItems; }
        int getLine() const { return line; }
    };

    class MainFuncDefAST : public AST {
    private:
        shared_ptr<BlockAST> block;
        int line;
    public:
        explicit MainFuncDefAST(shared_ptr<BlockAST> block, int line) : block(std::move(block)), line(line) {}
        shared_ptr<BlockAST> &getBlock() { return block; }
        int getLine() const { return line; }
    };

    class FuncDefAST : public AST {
    private:
        shared_ptr<FuncTypeAST> funcType;
        string name;
        shared_ptr<FuncFParamsAST> funcParams;
        shared_ptr<BlockAST> block;
        int line;
    public:
        FuncDefAST(shared_ptr<FuncTypeAST> funcType, string name, shared_ptr<FuncFParamsAST> funcParams,
                   shared_ptr<BlockAST> block, int line) :
                funcType(std::move(funcType)), name(std::move(name)), funcParams(std::move(funcParams)),
                block(std::move(block)), line(line) {}
        int getLine() const { return line; }
        string& getName() { return name; }
        shared_ptr<FuncTypeAST>& getFuncType() { return funcType; }
        shared_ptr<FuncFParamsAST>& getParams() { return funcParams; }
        shared_ptr<BlockAST>& getBlock() { return block; }
    };

    class CompUnitAST : public AST {
    private:
        vector<shared_ptr<DeclAST> > decls;
        vector<shared_ptr<FuncDefAST> > funcDefs;
        shared_ptr<MainFuncDefAST> mainfunc;
    public:
        CompUnitAST(vector<shared_ptr<DeclAST>> decls, vector<shared_ptr<FuncDefAST>> funcDefs,
                    shared_ptr<MainFuncDefAST> mainfunc) :
                decls(std::move(decls)), funcDefs(std::move(funcDefs)), mainfunc(std::move(mainfunc)) {}

        vector<shared_ptr<DeclAST> > &getDecls() { return decls; }

        vector<shared_ptr<FuncDefAST> > &getFuncDefs() { return funcDefs; }

        shared_ptr<MainFuncDefAST> &getMainFuncDef() { return mainfunc; }
    };
}

#endif //COMPILER_AST_H

