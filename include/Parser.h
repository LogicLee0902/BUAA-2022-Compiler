//
// Created by Leo on 2022/9/27.
//

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "AST.h"
#include "ErrorProcesser.h"

namespace parser{
    using namespace std;
    using namespace ast;
    class Parser{
    private:
        fstream output;
        int index; // index of identifierList
        string currentIdentifier;
        identifierType currentType;
        int prefetchIndex; // index of identifierList prefetched
        identifierType prefetchType;
        shared_ptr<CompUnitAST> globalAST;
    public:
        explicit Parser(const char* outputFile);
        ~Parser();
        void analysis();
        int prefetch();
        int readNext();
        void parseProgram();
        shared_ptr<CompUnitAST>& getGlobalAST();
        shared_ptr<CompUnitAST> parseCompUnit();
        shared_ptr<DeclAST> parseDecl();
        shared_ptr<FuncDefAST> parseFuncDef();
        shared_ptr<MainFuncDefAST> parseMainFuncDef();
        shared_ptr<ConstDeclAST> parseConstDecl();
        shared_ptr<VarDeclAST> parseVarDecl();
        shared_ptr<ConstDefAST> parseConstDef();
        shared_ptr<ConstInitValAST> parseConstInitVal();
        shared_ptr<ConstExpAST> parseConstExp();
        shared_ptr<BtypeAST> parseBtype();
        shared_ptr<VarDefAST> parseVarDef();
        shared_ptr<InitValAST> parseInitVal();
        shared_ptr<FuncTypeAST> parseFuncType();
        shared_ptr<FuncFParamsAST> parseFuncFParams();
        shared_ptr<BlockAST> parseBlock();
        shared_ptr<BlockItemAST> parseBlockItem();
        shared_ptr<FuncParamAST> parseFuncParam();
        shared_ptr<StmtAST> parseStmt();
        shared_ptr<ExpAST> parseExp();
        shared_ptr<CondAST> parseCond();
        shared_ptr<LValAST> parseLVal();
        shared_ptr<PrimaryExpAST> parsePrimaryExp();
        shared_ptr<NumberAST> parseNumber();
        shared_ptr<UnaryExpAST> parseUnaryExp();
        shared_ptr<UnaryOpAST> parseUnaryOp();
        shared_ptr<FuncRParamsAST> parseFuncRParams();
        shared_ptr<MulExpAST> parseMulExp();
        shared_ptr<AddExpAST> parseAddExp();
        shared_ptr<RelExpAST> parseRelExp();
        shared_ptr<EqExpAST> parseEqExp();
        shared_ptr<LAndExpAST> parseLAndExp();
        shared_ptr<LOrExpAST> parseLOrExp();
        void restore();
        int getLine() const;
        int getPreLine() const;
        void setToBackUp(int num);
        friend void dealMulExp(vector<shared_ptr<UnaryExpAST> > &t, Parser& p);
        friend void dealAddExp(vector<shared_ptr<MulExpAST> > &t, Parser& p);
    };
}

#endif //COMPILER_PARSER_H
