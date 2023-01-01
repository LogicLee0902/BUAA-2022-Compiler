//
// Created by Leo on 2022/9/13.
//
#include "include/Lexer.h"
#include <iostream>

namespace lexer {

    vector<Identifier> identifierList;
    map<string, identifierType> identifierMap;

    using namespace std;

    bool isLetter(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    bool isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    bool isBlank(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n';
    }

    bool isSymbol(char ch) {
        return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '<' || ch == '>' || ch == '=' ||
               ch == '!' || ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' ||
               ch == '}' || ch == '&' || ch == '|';
    }

    Lexer::Lexer(const string& inputFile, const char *outputFile) {
        cout << "INPUT FILE " << inputFile << endl;
        this->input.open(inputFile, ios::in);
        this->output.open(outputFile, ios::out);
        this->line = 1;
        identifierMap["Ident"] = IDENFR;
        identifierMap["IntConst"] = INTCON;
        identifierMap["FormatString"] = STRCON;
        identifierMap["main"] = MAINTK;
        identifierMap["const"] = CONSTTK;
        identifierMap["int"] = INTTK;
        identifierMap["void"] = VOIDTK;
        identifierMap["break"] = BREAKTK;
        identifierMap["continue"] = CONTINUETK;
        identifierMap["if"] = IFTK;
        identifierMap["else"] = ELSETK;
        identifierMap["while"] = WHILETK;
        identifierMap["getint"] = GETINTTK;
        identifierMap["printf"] = PRINTFTK;
        identifierMap["return"] = RETURNTK;
        identifierMap["+"] = PLUS;
        identifierMap["-"] = MINU;
        identifierMap["*"] = MULT;
        identifierMap["/"] = DIV;
        identifierMap["%"] = MOD;
        identifierMap["<"] = LSS;
        identifierMap["<="] = LEQ;
        identifierMap[">"] = GRE;
        identifierMap[">="] = GEQ;
        identifierMap["=="] = EQL;
        identifierMap["!="] = NEQ;
        identifierMap["="] = ASSIGN;
        identifierMap[";"] = SEMICN;
        identifierMap[","] = COMMA;
        identifierMap["("] = LPARENT;
        identifierMap[")"] = RPARENT;
        identifierMap["["] = LBRACK;
        identifierMap["]"] = RBRACK;
        identifierMap["{"] = LBRACE;
        identifierMap["}"] = RBRACE;
        identifierMap["!"] = NOT;
        identifierMap["&&"] = AND;
        identifierMap["||"] = OR;
        identifierMap["bitand"] = BITAND;
    }

    int Lexer::readNext() {
        //output << "IN";
        char ch;
        string str;
        Identifier identifier;
        bool flag = true;
        while(flag) {
            //cout << ch;
            ch = (char)input.get();
            flag = false;
            while(isBlank(ch)) {
                if (ch == '\n') ++line;
                ch = (char)input.get();
            }
            if (ch == EOF) {
                    return -1;
            }
            else if(isLetter(ch) || ch == '_') {
                str += ch;
                while((ch = (char)input.get()) != EOF && (isLetter(ch) || isDigit(ch) || ch == '_')) {
                    str += ch;
                }
                input.putback(ch);
                if(identifierMap.find(str) != identifierMap.end()) {
                    // output << pairName[identifierMap[str]] << " " << str << endl;
                    identifier.name = str;
                    identifier.type = identifierMap[str];
                    identifier.line = line;
                } else {
                    // output << "IDENFR " << str << endl;
                    identifier.name = str;
                    identifier.type = IDENFR;
                    identifier.line = line;
                }
                str.clear();
            }
            else if(isDigit(ch)) {
                str += ch;
                while((ch = (char)input.get()) != EOF && isDigit(ch)) {
                    str += ch;
                }
                input.putback(ch);
                // output << "INTCON " << str << endl;
                identifier.name = str;
                identifier.type = INTCON;
                identifier.line = line;
                str.clear();
            }
            else if(isSymbol(ch)) {
                str += ch;
                if (ch == '/') {
                    ch = (char)input.get();
                    if (ch == '/') {
                        str.pop_back();
                        while ((ch = (char)input.get()) != EOF && ch != '\n');
                        line ++;
                        continue;
                    } else if (ch == '*') {
                        str.pop_back();
                        while ((ch = (char)input.get()) != EOF) {
                            if (ch == '\n') line ++;
                            if (ch == '*') {
                                char ch2 = (char)input.get();
                                ch = ch2;
                                if (ch == '/') {
                                    break;
                                } else {
                                    input.putback(ch);
                                }
                            }
                        }
                        continue;
                    } else {
                        input.putback(ch);
                    }
                }
                else if(ch == '<' || ch == '>' || ch == '!' || ch == '=') {
                    if((ch = (char)input.get()) != EOF && ch == '=') {
                        str += ch;
                    } else {
                        input.putback(ch);
                    }
                }
                else if(ch == '&' || ch == '|') {

                    if((ch = (char)input.get()) != EOF && (ch == '&' || ch == '|')) {
                        str += ch;
                    } else {
                        input.putback(ch);
                    }
                }
               // output << pairName[identifierMap[str]] << " " << str << endl;
                identifier.name = str;
                identifier.type= identifierMap[str];
                identifier.line= line;
                str.clear();
            }
            else if(ch == '"') {
                str += ch;
                while((ch = (char)input.get()) != EOF && ch != '"') {
                    str += ch;
                }
                str += ch;
                //output << "STRCON " << str << endl;
                identifier.name = str;
                identifier.type = STRCON;
                identifier.line = line;
                str.clear();
            }
        }
        if (!identifier.name.empty()) identifierList.push_back(identifier);
        return identifier.type;
    }

    void Lexer::analysis() {
        while(readNext() != -1);
    }

    Lexer::~Lexer() {
        input.close();
        output.close();
    }
}
