//
// Created by Leo on 2022/9/25.
//

#ifndef COMPILER_PRELOAD_H
#define COMPILER_PRELOAD_H

#include <iostream>

#define LEXICAL
#define SYNTAX
#define OPTIMIZE
#define OPTIMIZE_DIVIDE
#define OPTIMIZE_MOD
#define IRCODE
//#define OPTIMIZEREG


using namespace std;


// for identifier
static const int identifierNum = 40;

enum identifierType {
    IDENFR, INTCON, STRCON, MAINTK, CONSTTK, INTTK, BREAKTK, CONTINUETK, IFTK, ELSETK,
    NOT, AND, OR, WHILETK, GETINTTK, PRINTFTK, RETURNTK, PLUS, MINU, VOIDTK,
    MULT, DIV, MOD, LSS, LEQ, GRE, GEQ, EQL, NEQ,
    ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE, BITAND, END=-1
};

static const char *pairName[identifierNum] = {
        "IDENFR", "INTCON", "STRCON", "MAINTK", "CONSTTK", "INTTK", "BREAKTK", "CONTINUETK", "IFTK", "ELSETK",
        "NOT", "AND", "OR", "WHILETK", "GETINTTK", "PRINTFTK", "RETURNTK", "PLUS", "MINU", "VOIDTK",
        "MULT", "DIV", "MOD", "LSS", "LEQ", "GRE", "GEQ", "EQL", "NEQ",
        "ASSIGN", "SEMICN", "COMMA", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE", "BITAND"
};

enum OperatorType {
    ArrayOT, VarOT, ConstOT, PLUSOT, MINUSOT, MULTOT, MULTHOT, DIVOT, MODOT,
    AssignOT, PushOT, CallOT,
    GetIntOT, PrintOT, FuncOT, ReturnOT, MainOT, ExitOT, ParamOT, PrintIntOT,
    BLTOT, BLEOT, BGTOT, BGEOT, BEQOT, BNEOT, ANDOT, OROT, NOTOT,
    // >=, >, <=, <, !=, ==
    JumpOT, LabelOT,
    SEQOT, SNEOT, SLTOT, SLEOT, SGTOT, SGEOT, SLLOT, SRLOT, SRLVOT, SLLVOT, SRAOT, SRAVOT,
    //&
    BITANDOT
};

static const char *operatorName[] = {
        "arr", "var", "Const", "Plus", "Minu", "Mult", "MultH", "Div", "Mod",
        "Assign", "Push", "Call",
        "GetInt", "Print", "Func", "Ret", "Main", "Exit", "Param", "PrintInt",
        "blt", "ble", "bgt", "bge", "beq", "bne", "and", "or", "Not",
        "Jump", "Label",
        "seq", "sne", "slt", "sle", "sgt", "sge", "sll", "srl", "srlv", "sllv", "sra", "srav",
        "and"
};

enum SymbolType {
    INTST, VOIDST, IFST, WHILEST
};

static const char *symbolName[] = {
        "int", "void", "if_", "while_"
};

#endif //COMPILER_PRELOAD_H
