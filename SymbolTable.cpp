//
// Created by Leo on 2022/10/16.
//

#include "include/SymbolTable.h"

using namespace std;

void SymbolTable::clearCurrentVarTable(int domain) {
    for (auto it = this->varTable.begin(); it != this->varTable.end();) {
        if (it->getDomain() == domain) {
            it = this->varTable.erase(it);
        } else {
            it++;
        }
    }
}

void SymbolTable::clearCurrentConstTable(int domain) {
    for (auto it = this->constTable.begin(); it != this->constTable.end();) {
        if (it->getDomain() == domain) {
            it = this->constTable.erase(it);
        } else {
            it++;
        }
    }
}