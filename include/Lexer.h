//
// Created by Leo on 2022/9/13.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include "PRELOAD.h"

namespace lexer {

    using namespace std;

    struct Identifier{
        string name;
        int line;
        enum identifierType type;
    };

    extern vector<Identifier> identifierList;
    extern map<string, identifierType> identifierMap;

    class Lexer {
    private:
        fstream input, output;
        int line;
    public:
        Lexer(const string& inputFile, const char* outputFile);
        ~Lexer();
        int readNext();
        void analysis();
    };

    bool isLetter(char ch);

    bool isSymbol(char ch);

    bool isDigit(char ch);

    bool isBlank(char ch);
}

#endif //COMPILER_LEXER_H
