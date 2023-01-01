#include "include/Lexer.h"
#include "include/Parser.h"
#include "include/ErrorProcesser.h"
#include "include/IRCodeGenerator.h"
#include "include/MipsGenerator.h"
#include "include/Optimization.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace lexer;
using namespace parser;
using namespace error;
using namespace irCode;

string INPUT_FILE = "testfile";
const char *OUTPUT_FILE = "output.txt";
const char *ERROR_FILE = "error.txt";
string MIPS_FILE = "mips.txt";

//#define DEBUG
//#define ERROR

int main() {
//    scanf("%s", INPUT_FILE);
//    std::system("chcp 65001");
//    cin >> INPUT_FILE;
//    MIPS_FILE = INPUT_FILE + "_20377241_.txt";
    // cout << INPUT_FILE << endl;
    Lexer lexer(INPUT_FILE+".txt", OUTPUT_FILE);
    lexer.analysis();
#ifdef DEBUG
    for (auto & i : identifierList) {
        cout << i.line << " " << pairName[i.type] << " " << i.name << endl;
    }
#endif
    Parser parser(OUTPUT_FILE);
    parser.analysis();
    cout << "Analysis finished." << endl;
    shared_ptr<ast::CompUnitAST> ast = parser.getGlobalAST();
    ErrorProcesser errorProcesser(ERROR_FILE, ast);
#ifdef ERROR
    errorProcesser.analysis();
    errorProcesser.print();
#endif
    IRCodeGenerator irCodeGenerator(ast, INPUT_FILE);
    irCodeGenerator.analyze();
    cout << "generate IR Code Over" << endl;
#ifdef OPTIMIZE
    Optimization optimization;
    GenerateBlock generateBlock = optimization.run();
#endif
    MipsGenerator mipsGenerator(MIPS_FILE);
#ifndef OPTIMIZEREG
    mipsGenerator.run();
#else
    mipsGenerator.work(generateBlock);
#endif

    return 0;


}
