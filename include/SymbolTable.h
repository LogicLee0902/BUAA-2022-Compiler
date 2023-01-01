//
// Created by Leo on 2022/10/10.
//

#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include <iostream>
#include <vector>
#include <utility>
#include "PRELOAD.h"

using namespace std;

class Symbol {
private:
    string name;
    identifierType type;
public:
    Symbol(string name, identifierType type) : name(std::move(name)), type(type) {}

    string getName() const { return name; }

    identifierType getType() const { return type; }
};

class VarSymbol : public Symbol {
private:
    int dimension; //  0 for scalar, 1 for array, 2 for matrix
    int domain;
public:
    VarSymbol(string name, identifierType type, int dimension, int domain) : Symbol(std::move(name), type),
                                                                                           dimension(dimension),
                                                                                           domain(domain) {}

    int getDimension() const { return dimension; }

    int getDomain() const { return domain; }
};

class ConstSymbol : public Symbol {
private:
    int domain;
    int dimension;
public:
    ConstSymbol(string name, identifierType type, int domain, int dimension) : Symbol(std::move(name),
                                                                                                    type),
                                                                                             domain(domain),
                                                                                             dimension(dimension) {}

    int getDomain() const { return domain; }

    int getDimension() const { return dimension; }
};

class FuncSymbol : public Symbol {
private:
    vector<VarSymbol> parameters;
public:
    FuncSymbol(string name, identifierType type, vector<VarSymbol> parameters) : Symbol(std::move(name),
                                                                                                      type),
                                                                                               parameters(std::move(
                                                                                                       parameters)) {}

    int getParameterNum() const { return (int) parameters.size(); }

    vector<VarSymbol> &getParameters() { return parameters; }
};

class SymbolTable {
private:
    vector<VarSymbol> varTable;
    vector<ConstSymbol> constTable;
    vector<FuncSymbol> funcTable;
public:
    vector<VarSymbol> &getVarTable() { return varTable; }

    vector<ConstSymbol> &getConstTable() { return constTable; }

    vector<FuncSymbol> &getFuncTable() { return funcTable; }

    void clearCurrentVarTable(int domain);

    void clearCurrentConstTable(int domain);
};


#endif //COMPILER_SYMBOLTABLE_H
