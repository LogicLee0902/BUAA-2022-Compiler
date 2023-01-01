//
// Created by Leo on 2022/10/11.
//

#ifndef COMPILER_ERRORPROCESSER_H
#define COMPILER_ERRORPROCESSER_H

#include "PRELOAD.h"
#include "AST.h"
#include "SymbolTable.h"
#include <iostream>
#include <stack>
#include <algorithm>
#include <memory>

namespace error {
    using namespace std;
    using namespace ast;

    extern SymbolTable symbolTable;
    extern vector <pair <int , string> >errors; // (line, type)

    class ErrorProcesser {
    private:
        fstream err;
        int currentDomain;
        string currentFunction;
        stack<identifierType>currentType;
        identifierType returnType;
        stack<bool> isLoop;
        shared_ptr<CompUnitAST> &compUnitAST;
    public:
        ErrorProcesser(const char *err, shared_ptr<CompUnitAST> &compUnitAST);
        ~ErrorProcesser();
        void analysis();
        void print();
        void processDecl(shared_ptr<DeclAST> &declAST);
        void processConstDecl(shared_ptr<ConstDeclAST> &constDeclAST);
        void processConstDef(shared_ptr<ConstDefAST> &constDefAST);
        void processVarDecl(shared_ptr<VarDeclAST> &varDeclAST);
        void processVarDef(shared_ptr<VarDefAST> &varDefAST);
        void processFuncDef(shared_ptr<FuncDefAST> &funcDefAST);
        void processFuncFParam(shared_ptr<FuncParamAST> &funcParamAST, vector<VarSymbol> &params);
        void processBlock(shared_ptr<BlockAST> &blockAST, bool isFunction);
        void processBlockItem(shared_ptr<BlockItemAST> &blockItem);
        void processStmt(shared_ptr<StmtAST> &stmtAST);
        int processExp(shared_ptr<ExpAST> &expAST);
        void processMainDef(shared_ptr<MainFuncDefAST> &mainFuncDefAST);
        int processLVal(shared_ptr<LValAST> &lValAST, bool &con);
        bool processPrint(string &formmat, int num);
        void processCond(shared_ptr<CondAST> &condAST);
        void processConstExp(shared_ptr<ConstExpAST> &constExpAST);
        void processConstInitVal(shared_ptr<ConstInitValAST> &constInitValAST);
        void processInitVal(shared_ptr<InitValAST> &initValAST);
        void processRelExp(shared_ptr<RelExpAST> &relExpAST);
        int processAddExp(shared_ptr<AddExpAST> &addExpAST);
        int processMulExp(shared_ptr<MulExpAST> &mulExpAST);
        int processUnaryExp(shared_ptr<UnaryExpAST> &unaryExpAST);
        int processPrimaryExp(shared_ptr<PrimaryExpAST> &primaryExpAST);
        void processEqExp(shared_ptr<EqExpAST> &eqExpAST);
        void processFuncRParams(shared_ptr<FuncRParamsAST> &funcRParamsAST, vector<int>& dimensions);
        void processLAndExp(shared_ptr<LAndExpAST> &lAndExpAST);
        void processLOrExp(shared_ptr<LOrExpAST> &lOrpExpAST);
    };

}


#endif //COMPILER_ERRORPROCESSER_H
