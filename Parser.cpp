//
// Created by Leo on 2022/9/30.
//

#include <iostream>
#include <utility>
#include <memory>
#include "include/PRELOAD.h"
#include "include/Parser.h"

namespace parser {
    using namespace std;
    using namespace lexer;
    using namespace error;

    Parser::Parser(const char *filename) {
        this->output.open(filename, ios::out);
        index = 0;
    }

    Parser::~Parser() {
        this->output.close();
    }

    void Parser::restore() {
        prefetchIndex = index - 1;
        if (prefetchIndex < identifierList.size()) {
            prefetchType = identifierList[prefetchIndex++].type;
        } else {
            prefetchType = END;
        }
    }

    int Parser::getLine() const{
        if (index > 0) {
            return identifierList[index - 1].line;
        } else {
            return 1;
        }
    }

    int Parser::getPreLine() const {
        if (index > 1) {
            return identifierList[index - 2].line;
        } else {
            return 1;
        }
    }

    void Parser::setToBackUp(int num) {
        this->index = num - 1;
        if (this->index < identifierList.size()) {
            currentIdentifier = identifierList[this->index].name;
            currentType = identifierList[this->index++].type;
        } else {
            currentType = END;
        }
    }

    int Parser::readNext() {
        /*
         * Step 1: Get the next token
         * Step 2: Print the related information
         * Step 3:prefetch so need to the extra judgement
         */
        if (index < identifierList.size()) {
            currentIdentifier = identifierList[index].name;
            prefetchIndex = index + 1;
#ifdef LEXICAL
            if (index > 0) {
                output << pairName[identifierList[index - 1].type] << " " << identifierList[index - 1].name << endl;
            }
#endif //LEXICAL
            currentType = identifierList[index++].type;
            return currentType;
        } else { // no need for prefetchIndex
#ifdef LEXICAL
            if (index > 0) {
                output << pairName[identifierList[index - 1].type] << " " << identifierList[index - 1].name << endl;
            }
#endif //LEXICAL
        }
        currentType = END;
        return END;
    }

    int Parser::prefetch() {
        if (prefetchIndex < identifierList.size()) {
            prefetchType = identifierList[prefetchIndex++].type;
            return prefetchType;
        } else
            return END;
    }

    void Parser::parseProgram() {
        if ((globalAST = parseCompUnit())) {
#ifdef SYNTAX
            output << "<CompUnit>" << endl;
#endif //SYNTAX
        }
    }

    void Parser::analysis() {
        readNext();
        parseProgram();
    }

    shared_ptr<CompUnitAST>& Parser::getGlobalAST() {
        return globalAST;
    }

    shared_ptr<PrimaryExpAST> Parser::parsePrimaryExp() { //NOLINT
        switch (currentType) {
            case INTCON: {
                shared_ptr<NumberAST> number = parseNumber();
#ifdef SYNTAX
                output << "<Number>" << endl;
#endif //SYNTAX
                return make_shared<PrimaryExpAST>(std::move(number), 2);
            }
            case IDENFR: {
                shared_ptr<LValAST> lVal = parseLVal();
#ifdef SYNTAX
                output << "<LVal>" << endl;
#endif //SYNTAX
                return make_shared<PrimaryExpAST>(std::move(lVal), 1);
            }
            case LPARENT: {
                readNext();
                shared_ptr<ExpAST> exp = parseExp();
#ifdef SYNTAX
                output << "<Exp>" << endl;
#endif //SYNTAX
                if (currentType == RPARENT) {
                    readNext();
                } else {
                    errors.emplace_back(getPreLine(), "j");
                }
                    return make_shared<PrimaryExpAST>(std::move(exp), 0);
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<CondAST> Parser::parseCond() {
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<LOrExpAST> lOrExp = parseLOrExp();
#ifdef SYNTAX
                output << "<LOrExp>" << endl;
#endif //SYNTAX
                return make_shared<CondAST>(std::move(lOrExp));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<LOrExpAST> Parser::parseLOrExp() {
        vector<shared_ptr<LAndExpAST> > landExpList;
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<LAndExpAST> landExp = parseLAndExp();
#ifdef SYNTAX
                output << "<LAndExp>" << endl;
#endif //SYNTAX
                landExpList.push_back(landExp);
                while (currentType == OR) {
#ifdef SYNTAX
                    output << "<LOrExp>" << endl;
#endif //SYNTAX
                    readNext();
                    landExp = parseLAndExp();
                    landExpList.push_back(landExp);
#ifdef SYNTAX
                    output << "<LAndExp>" << endl;
#endif //SYNTAX
                }
                return make_shared<LOrExpAST>(std::move(landExpList));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<LAndExpAST> Parser::parseLAndExp() {
        vector<shared_ptr<EqExpAST> > eqExpList;
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<EqExpAST> eqExp = parseEqExp();
#ifdef SYNTAX
                output << "<EqExp>" << endl;
#endif //SYNTAX
                eqExpList.push_back(eqExp);
                while (currentType == AND) {
#ifdef SYNTAX
                    output << "<LAndExp>" << endl;
#endif //SYNTAX
                    readNext();
                    eqExp = parseEqExp();
                    eqExpList.push_back(eqExp);
#ifdef SYNTAX
                    output << "<EqExp>" << endl;
#endif //SYNTAX
                }
                return make_shared<LAndExpAST>(std::move(eqExpList));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<ExpAST> Parser::parseExp() {
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<AddExpAST> addExp = parseAddExp();
#ifdef SYNTAX
                output << "<AddExp>" << endl;
#endif //SYNTAX
                return make_shared<ExpAST>(std::move(addExp));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<ConstExpAST> Parser::parseConstExp() {
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON: {
                shared_ptr<AddExpAST> addExp = parseAddExp();
#ifdef SYNTAX
                output << "<AddExp>" << endl;
#endif //SYNTAX
                return make_shared<ConstExpAST>(std::move(addExp));
            }
            default:
                break;
        }
        return nullptr;
    }


    shared_ptr<EqExpAST> Parser::parseEqExp() {
        vector<shared_ptr<RelExpAST> > relExpList;
        vector<identifierType> symbolList;
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<RelExpAST> relExp = parseRelExp();
#ifdef SYNTAX
                output << "<RelExp>" << endl;
#endif //SYNTAX
                relExpList.push_back(relExp);
                while (currentType == EQL || currentType == NEQ) {
#ifdef SYNTAX
                    output << "<EqExp>" << endl;
#endif //SYNTAX
                    identifierType symbol = currentType;
                    symbolList.push_back(symbol);
                    readNext();
                    relExp = parseRelExp();
                    relExpList.push_back(relExp);
#ifdef SYNTAX
                    output << "<RelExp>" << endl;
#endif //SYNTAX
                }
                return make_shared<EqExpAST>(std::move(symbolList), std::move(relExpList));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<RelExpAST> Parser::parseRelExp() {
        vector<shared_ptr<AddExpAST> > addExpList;
        vector<identifierType> symbolList;
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                shared_ptr<AddExpAST> addExp = parseAddExp();
#ifdef SYNTAX
                output << "<AddExp>" << endl;
#endif //SYNTAX
                addExpList.push_back(addExp);
                while (currentType == LSS || currentType == LEQ || currentType == GRE || currentType == GEQ) {
#ifdef SYNTAX
                    output << "<RelExp>" << endl;
#endif //SYNTAX
                    identifierType symbol = currentType;
                    symbolList.push_back(symbol);
                    readNext();
                    addExp = parseAddExp();
                    addExpList.push_back(addExp);
#ifdef SYNTAX
                    output << "<AddExp>" << endl;
#endif //SYNTAX
                }
                return make_shared<RelExpAST>(std::move(symbolList), std::move(addExpList));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<StmtAST> Parser::parseStmt() {
        // LVal = Exp
        shared_ptr<LValAST> lVal;
        shared_ptr<ExpAST> exp;

        // Exp
        shared_ptr<ExpAST> expSingle;

        // block
        shared_ptr<BlockAST> block;

        // if (Cond) Stmt [else Stmt]
        shared_ptr<CondAST> condIf;
        shared_ptr<StmtAST> stmtIf;
        shared_ptr<StmtAST> stmtElse;

        // while
        shared_ptr<CondAST> condWhile;
        shared_ptr<StmtAST> stmtWhile;

        // break | continue
        identifierType category;

        // return [Exp]
        shared_ptr<ExpAST> expReturn;

        // getint
        shared_ptr<LValAST> lValGetint;

        // print
        vector<shared_ptr<ExpAST> > expPrint;
        string format;

        int line;

        switch (currentType) {
            case PRINTFTK: {
                line = getLine();
                readNext();
                if (currentType == LPARENT) {
                    readNext();
                    if (currentType == STRCON) {
                        format = currentIdentifier;
                        for(int i = 0; i < format.size(); i++) {
                          if (i == 0 || i == format.size() - 1) {
                              continue;
                          }
                          if (format[i] == '%') {
                              if (format[i + 1] == 'd') {
                                  ++ i;
                                  continue;
                              } else {
                                  errors.emplace_back(getLine(), "a");
                                  break;
                              }
                          }
                          if (format[i] == '\\') {
                              if (format[i + 1] == 'n') {
                                  ++ i;
                                  continue;
                              } else {
                                  errors.emplace_back(getLine(), "a");
                                  break;
                              }
                          }
                          if (format[i] == 32 || format[i] == 33
                          || (format[i] >= 40 && format[i] <= 120)) {
                              continue;
                          } else {
                              errors.emplace_back(getLine(),"a");
                          }
                        }
                        readNext();
                        while (currentType == COMMA) {
                            readNext();
                            shared_ptr<ExpAST> exp_print = parseExp();
#ifdef SYNTAX
                            output << "<Exp>" << endl;
#endif //SYNTAX
                            expPrint.push_back(exp_print);
                        }

                        if (currentType == RPARENT) {
                            readNext();
                            if (currentType == SEMICN) {
                                readNext();
                                //output << "<Stmt>" << endl;
                                return make_shared<StmtAST>(std::move(format), std::move(expPrint), line, 8);
                            } else {
                                errors.emplace_back(getPreLine(), "i");
                            }
                        } else {
                            errors.emplace_back(getPreLine(), "j");
                        }
                    }
                }
                break;
            }
            case BREAKTK:
            case CONTINUETK: {
                line = getLine();
                category = currentType;
                readNext(); // usually semicolon
                if (currentType != SEMICN) {
                    errors.emplace_back(getPreLine(), "i");
                } else {
                    readNext();
                }
                return make_shared<StmtAST>(category, line, 5);
            }
            case RETURNTK: {
                line = getLine();
                readNext();
                if (currentType == SEMICN) {
                    readNext();
                    return make_shared<StmtAST>(std::move(expReturn), "", line, 6);
                } else {
                    expReturn = parseExp();
#ifdef SYNTAX
                    output << "<Exp>" << endl;
#endif //SYNTAX
                    if (currentType != SEMICN) {
                        errors.emplace_back(getPreLine(), "i");
                    } else {
                        readNext();
                    }
                    return make_shared<StmtAST>(std::move(expReturn), "",line, 6);
                }
            }
            case WHILETK: {
                readNext();
                if (currentType == LPARENT) {
                    readNext();
                    condWhile = parseCond();
#ifdef SYNTAX
                    output << "<Cond>" << endl; // end with )
#endif //SYNTAX
                    if (currentType == RPARENT) {
                        readNext();
                    } else {
                        errors.emplace_back(getPreLine(), "j");
                    }
                        stmtWhile = parseStmt();
#ifdef SYNTAX
                        output << "<Stmt>" << endl;
#endif //SYNTAX
                        return make_shared<StmtAST>(std::move(condWhile), std::move(stmtWhile));
                }
                break;
            }
            case IFTK: {
                readNext();
                if (currentType == LPARENT) {
                    readNext();
                    condIf = parseCond();
#ifdef SYNTAX
                    output << "<Cond>" << endl; // end with )
#endif //SYNTAX
                    if (currentType == RPARENT) {
                        readNext();
                    } else {
                        errors.emplace_back(getPreLine(), "j");
                    }
                        stmtIf = parseStmt();
#ifdef SYNTAX
                        output << "<Stmt>" << endl;
#endif //SYNTAX
                        if (currentType == ELSETK) {
                            readNext();
                            stmtElse = parseStmt();
#ifdef SYNTAX
                            output << "<Stmt>" << endl;
#endif //SYNTAX
                        }
                        return make_shared<StmtAST>(std::move(condIf), std::move(stmtIf), std::move(stmtElse), 3);
                }
                break;
            }
            case LBRACE: {
                block = parseBlock();
#ifdef SYNTAX
                output << "<Block>" << endl;
#endif //SYNTAX
                return make_shared<StmtAST>(std::move(block));
            }
            case SEMICN: {
                // null sentence
                readNext();
                return make_shared<StmtAST>(std::move(expSingle));
            }
            case PLUS:
            case MINU:
            case LPARENT:
            case INTCON: {
                // [exp]
                expSingle = parseExp();
#ifdef SYNTAX
                output << "<Exp>" << endl;
#endif //SYNTAX
                if (currentType != SEMICN) {
                    errors.emplace_back(getPreLine(), "i");
                } else {
                    readNext();
                }
                return make_shared<StmtAST>(std::move(expSingle));
            }
            case IDENFR: {
                // has multiple choices, need to prefetch
                line = getLine();
                prefetch();
                if (prefetchType == LPARENT) {
                    // function call
                    expSingle = parseExp();
#ifdef SYNTAX
                    output << "<Exp>" << endl;
#endif //SYNTAX
                    if (currentType != SEMICN) {
                        errors.emplace_back(getPreLine(), "i");
                    } else {
                        readNext();
                    }
                    return make_shared<StmtAST>(std::move(expSingle));
                }
                bool flag = false;
                int idx = this->index;
                restore();
                // just pre-scan, no need to restore
                parseLVal();
                if (currentType == ASSIGN) {
                    flag = true;
                }
                restore();
                setToBackUp(idx);
                if (flag) {
                    // assignment
                    prefetch();
                    if (prefetchType != GETINTTK) {
                        // LVal = Exp
                        lVal = parseLVal();
#ifdef SYNTAX
                        output << "<LVal>" << endl;
#endif //SYNTAX
                        if (currentType == ASSIGN) {
                            readNext();
                            exp = parseExp();
#ifdef SYNTAX
                            output << "<Exp>" << endl;
#endif //SYNTAX
                            if (currentType != SEMICN) {
                                errors.emplace_back(getPreLine(), "i");
                            } else {
                                readNext();
                            }
                            return make_shared<StmtAST>(std::move(lVal), std::move(exp), line);
                        }
                    } else {
                        // getint
                        lValGetint = parseLVal();
#ifdef SYNTAX
                        output << "<LVal>" << endl;
#endif //SYNTAX
                        // getint();
                        for (int i = 0; i < 3; ++i) readNext();
                        if (currentType != RPARENT) {
                            errors.emplace_back(getPreLine(), "j");
                        } else {
                            readNext();
                        }
                        if (currentType != SEMICN) {
                            errors.emplace_back(getPreLine(), "i");
                        } else {
                            readNext();
                        }
                        return make_shared<StmtAST>(std::move(lValGetint), line);
                    }
                } else {
                    expSingle = parseExp();
#ifdef SYNTAX
                    output << "<Exp>" << endl;
#endif //SYNTAX
                    if (currentType != SEMICN) {
                        errors.emplace_back(getPreLine(), "i");
                    } else {
                        readNext();
                    }
                    return make_shared<StmtAST>(std::move(expSingle));
                }
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<BlockItemAST> Parser::parseBlockItem() {
        switch (currentType) {
            case CONSTTK:
            case INTTK: {
                shared_ptr<DeclAST> decl = parseDecl();
                return make_shared<BlockItemAST>(std::move(decl), 0);
            }
            case IDENFR:
            case SEMICN:
            case LPARENT:
            case PLUS:
            case MINU:
            case INTCON:
            case LBRACE:
            case PRINTFTK:
            case WHILETK:
            case IFTK:
            case RETURNTK:
            case BREAKTK:
            case CONTINUETK: {
                shared_ptr<StmtAST> stmt = parseStmt();
#ifdef SYNTAX
                output << "<Stmt>" << endl;
#endif //SYNTAX
                return make_shared<BlockItemAST>(std::move(stmt), 1);
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<BlockAST> Parser::parseBlock() {
        readNext();
        int line;
        vector<shared_ptr<BlockItemAST> > blockItemList;
        if (currentType == RBRACE) {
            line = getLine();
            readNext();
            return make_shared<BlockAST>(std::move(blockItemList), line);
        }
        while (true) {
            switch (currentType) {
                case RBRACE:
                    break;
                case CONSTTK:
                case INTTK:
                case IDENFR:
                case SEMICN:
                case LPARENT:
                case PLUS:
                case MINU:
                case INTCON:
                case LBRACE:
                case PRINTFTK:
                case WHILETK:
                case IFTK:
                case RETURNTK:
                case BREAKTK:
                case CONTINUETK: {
                    shared_ptr<BlockItemAST> blockItem = parseBlockItem();
                    blockItemList.push_back(blockItem);
                    break;
                }
                default:
                    cout << "Unknown " << pairName[currentType] << " in parseBlock" << endl;
                    break;
            }
            if (currentType == RBRACE) {
                break;
            }
            // cout << "In Block" << endl;
        }
        line = getLine();
        readNext();
        return make_shared<BlockAST>(std::move(blockItemList), line);
    }

    shared_ptr<MainFuncDefAST> Parser::parseMainFuncDef() {
//        cout << currentIdentifier << endl;
        if (currentType == INTTK) {
//            cout << currentIdentifier << endl;
            readNext();
            int line = getLine();
            if (currentType == MAINTK) {
                readNext();
//                cout << currentIdentifier << endl;
                if (currentType == LPARENT) {
                    readNext();
//                    cout << currentIdentifier << endl;
                    if (currentType == RPARENT) {
                        readNext();
//                        cout << currentIdentifier << endl;
                    } else {
                        errors.emplace_back(getPreLine(), "j");
                    }
                        if (currentType == LBRACE) {
                            shared_ptr<BlockAST> block = parseBlock();
#ifdef SYNTAX
                            output << "<Block>" << endl;
#endif //SYNTAX
                            return make_shared<MainFuncDefAST>(std::move(block), line);
                        }
                }
            }
        }
        cout << "wrong in MainFuncDefAST" << endl;
        return nullptr;
    }

    shared_ptr<LValAST> Parser::parseLVal() {
        // currentType = IDENFR
        vector<shared_ptr<ExpAST> > expList;
        string identifier = currentIdentifier;
        int line = getLine();
        readNext();
        while (currentType == LBRACK) {
            readNext();
            switch (currentType) {
                case IDENFR:
                case PLUS:
                case MINU:
                case INTCON:
                case LPARENT: {
                    shared_ptr<ExpAST> exp = parseExp();
#ifdef SYNTAX
                    output << "<Exp>" << endl;
#endif //SYNTAX
                    expList.push_back(exp);
                    // currentType = RBRACK
                    if (currentType != RBRACK) {
                        errors.emplace_back(getPreLine(), "k");
                    } else {
                        readNext();
                    }
                    break;
                }
                default:
                    break;
            }
        }
        return make_shared<LValAST>(identifier, line, std::move(expList));
    }

    shared_ptr<ConstInitValAST> Parser::parseConstInitVal() {
        shared_ptr<ConstExpAST> constExp;
        vector<shared_ptr<ConstInitValAST> > constInitValList;
        switch (currentType) {
            case IDENFR:
            case PLUS:
            case MINU:
            case INTCON:
            case LPARENT: {
                constExp = parseConstExp();
#ifdef SYNTAX
                output << "<ConstExp>" << endl;
#endif //SYNTAX
                return make_shared<ConstInitValAST>(std::move(constExp));
            }
            case LBRACE: {
                readNext();
                switch (currentType) {
                    case IDENFR:
                    case PLUS:
                    case MINU:
                    case INTCON:
                    case LPARENT:
                    case LBRACE: {
                        shared_ptr<ConstInitValAST> constInitVal = parseConstInitVal();
#ifdef SYNTAX
                        output << "<ConstInitVal>" << endl;
#endif //SYNTAX
                        constInitValList.push_back(constInitVal);
                        while (currentType == COMMA) {
                            readNext();
                            constInitVal = parseConstInitVal();
#ifdef SYNTAX
                            output << "<ConstInitVal>" << endl;
#endif //SYNTAX
                            constInitValList.push_back(constInitVal);
                        }
                        readNext();
                        return make_shared<ConstInitValAST>(std::move(constInitValList));
                    }
                    case RBRACE:
                        readNext();
                        return make_shared<ConstInitValAST>(std::move(constInitValList));
                    default:
                        break;
                }
                return make_shared<ConstInitValAST>(std::move(constInitValList));
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<NumberAST> Parser::parseNumber() {
        if (currentType == INTCON) {
            long long value = stoll(currentIdentifier);
            readNext();
            return make_shared<NumberAST>(value, currentIdentifier);
        }
        return nullptr;
    }

    shared_ptr<FuncTypeAST> Parser::parseFuncType() {
        identifierType type = currentType;
        if (type == VOIDTK || type == INTTK) {
            readNext();
            return make_shared<FuncTypeAST>(type, currentIdentifier);
        }
        return nullptr;
    }

    shared_ptr<FuncParamAST> Parser::parseFuncParam() {
        // currentType == INTTK
        shared_ptr<BtypeAST> btype;
        string name;
        vector<shared_ptr<ConstExpAST> > constExpList;
        btype = parseBtype();
        // currentType == IDENFR
        name = currentIdentifier;
        int line = getLine();
        readNext();
        if (currentType == LBRACK) {
            readNext();
            // currentType == RBRACK
            if (currentType != RBRACK) {
                errors.emplace_back(getPreLine(),  "k");
                // cout << " error of missing ] : " << getPreLine() << endl;
            } else {
                readNext();
            }
            constExpList.push_back(nullptr);
            while (currentType == LBRACK) {
                readNext();
                shared_ptr<ConstExpAST> constExp = parseConstExp();
#ifdef SYNTAX
                output << "<ConstExp>" << endl;
#endif //SYNTAX
                // currentType == RBRACK
                constExpList.push_back(constExp);
                if (currentType != RBRACK) {
                    errors.emplace_back(getPreLine(),  "k");
                    // cout << " error of missing ] : " << getPreLine() << endl;
                } else {
                    readNext();
                }
            }
        }
        return make_shared<FuncParamAST>(std::move(btype), name, line, std::move(constExpList));
    }

    shared_ptr<FuncFParamsAST> Parser::parseFuncFParams() {
        vector<shared_ptr<FuncParamAST> > funcParamList;
        switch (currentType) {
            case INTTK: {
                shared_ptr<FuncParamAST> funcParam = parseFuncParam();
#ifdef SYNTAX
                output << "<FuncFParam>" << endl;
#endif //SYNTAX
                funcParamList.push_back(funcParam);
                while (currentType == COMMA) {
                    readNext();
                    funcParam = parseFuncParam();
#ifdef SYNTAX
                    output << "<FuncFParam>" << endl;
#endif //SYNTAX
                    funcParamList.push_back(funcParam);
                }
                return make_shared<FuncFParamsAST>(std::move(funcParamList));
            }
            case RPARENT:
                return make_shared<FuncFParamsAST>(std::move(funcParamList));
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<FuncDefAST> Parser::parseFuncDef() {
        shared_ptr<FuncTypeAST> funcType;
        string name;
        shared_ptr<FuncFParamsAST> funcFParams = nullptr;
        shared_ptr<BlockAST> block;
        // currentType == INTTK || VOIDTK
        funcType = parseFuncType();
#ifdef SYNTAX
        output << "<FuncType>" << endl;
#endif //SYNTAX
        // currentType == IDENFR
        name = currentIdentifier;
        int line = getLine();
        readNext();
        // currentType == LPARENT
        readNext();
        if (currentType == INTTK) {
            funcFParams = parseFuncFParams();
#ifdef SYNTAX
            output << "<FuncFParams>" << endl;
#endif //SYNTAX
        }
        // currentType == RPARENT
        if (currentType != RPARENT) {
            errors.emplace_back(getPreLine(),  "j");
        } else {
            readNext();
        }
        // currentType == LBRACE
        block = parseBlock();
#ifdef SYNTAX
        output << "<Block>" << endl;
#endif //SYNTAX
        return make_shared<FuncDefAST>(std::move(funcType), name, std::move(funcFParams), std::move(block), line);
    }

    shared_ptr<FuncRParamsAST> Parser::parseFuncRParams() {
        vector<shared_ptr<ExpAST> > expList;
        while (true) {
            switch (currentType) {
                case IDENFR:
                case PLUS:
                case MINU:
                case INTCON:
                case LPARENT: {
                    shared_ptr<ExpAST> exp = parseExp();
#ifdef SYNTAX
                    output << "<Exp>" << endl;
#endif //SYNTAX
                    expList.push_back(exp);
                    break;
                }
                default:
                    break;
            }
            if (currentType != COMMA) {
                break;
            }
            readNext();
            // cout << "In FuncRParamsAST" << endl;
        }
        return make_shared<FuncRParamsAST>(std::move(expList));
    }

    shared_ptr<UnaryOpAST> Parser::parseUnaryOp() {
        // currentType = PLUS or MINU or Not (only in Cond)
        identifierType type = currentType;
        readNext();
        return make_shared<UnaryOpAST>(type);
    }

    shared_ptr<UnaryExpAST> Parser::parseUnaryExp() {
        int line;
        switch (currentType) {
            case PLUS:
            case MINU:
            case NOT: {
                shared_ptr<UnaryOpAST> unaryOp = parseUnaryOp();
#ifdef SYNTAX
                output << "<UnaryOp>" << endl;
#endif //SYNTAX
                shared_ptr<UnaryExpAST> unaryExp = parseUnaryExp();
#ifdef SYNTAX
                output << "<UnaryExp>" << endl;
#endif //SYNTAX
                return make_shared<UnaryExpAST>(std::move(unaryOp), std::move(unaryExp));
            }
            case INTCON:
            case LPARENT: {
                shared_ptr<PrimaryExpAST> primaryExp = parsePrimaryExp();
#ifdef SYNTAX
                output << "<PrimaryExp>" << endl;
#endif //SYNTAX
                return make_shared<UnaryExpAST>(std::move(primaryExp));
            }
            case IDENFR: {
                // multiple circumstances
                restore();
                prefetch();
                switch (prefetchType) {
                    case LPARENT: {
                        // funcall
                        string name = currentIdentifier;
                        line = getLine();
                        readNext(); // get (
                        readNext();
                        switch (currentType) {
                            case PLUS:
                            case MINU:
                            case IDENFR:
                            case INTCON:
                            case LPARENT: {
                                shared_ptr<FuncRParamsAST> funcRParams = parseFuncRParams();
#ifdef SYNTAX
                                output << "<FuncRParams>" << endl;
#endif //SYNTAX
                                // currentType = RPARENT
                                if (currentType != RPARENT) {
                                    errors.emplace_back(getPreLine(),  "j");
                                } else {
                                    readNext();
                                }
                                return make_shared<UnaryExpAST>(name, std::move(funcRParams), line);
                            }
                            case RPARENT: {
                                readNext();
                                return make_shared<UnaryExpAST>(name, nullptr, line);
                            }
                            default: {
                                if (currentType != RPARENT) {
                                    errors.emplace_back(getPreLine(), "j");
                                }
                                return make_shared<UnaryExpAST>(name, nullptr, line);
                            }
                        }
                        break;
                    }
                    case LBRACK: {
                        // primaryExp
                        shared_ptr<PrimaryExpAST> primaryExp = parsePrimaryExp();
#ifdef SYNTAX
                        output << "<PrimaryExp>" << endl;
#endif //SYNTAX
                        return make_shared<UnaryExpAST>(std::move(primaryExp));
                    }
                    default: {
                        // primaryExp
                        shared_ptr<PrimaryExpAST> primaryExp  = parsePrimaryExp();
#ifdef SYNTAX
                        output << "<PrimaryExp>" << endl;
#endif //SYNTAX
                        return make_shared<UnaryExpAST>(std::move(primaryExp));
                    }
                }
                break;
            }
            default:
                break;
        }
        return nullptr;
    }

    void dealMulExp(vector<shared_ptr<UnaryExpAST> > &t, Parser &p) {
        switch (p.currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case INTCON:
            case NOT:
            case LPARENT: {
                shared_ptr<UnaryExpAST> unaryExp = p.parseUnaryExp();
#ifdef SYNTAX
                p.output << "<UnaryExp>" << endl;
#endif //SYNTAX
                t.push_back(unaryExp);
                break;
            }
            default:
                break;
        }
    }

    shared_ptr<MulExpAST> Parser::parseMulExp() {
        vector<identifierType> symbolList;
        vector<shared_ptr<UnaryExpAST> > unaryExpList;
        dealMulExp(unaryExpList, *this);
        while (currentType == MULT || currentType == DIV || currentType == MOD || currentType == BITAND) {
#ifdef SYNTAX
            output << "<MulExp>" << endl;
#endif //SYNTAX
            identifierType type = currentType;
            readNext();
            symbolList.push_back(type);
            dealMulExp(unaryExpList, *this);
        }
        return make_shared<MulExpAST>(std::move(unaryExpList), std::move(symbolList));
    }

    void dealAddExp(vector<shared_ptr<MulExpAST> > &t, Parser &p) {
        switch (p.currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case INTCON:
            case NOT:
            case LPARENT: {
                shared_ptr<MulExpAST> mulExp = p.parseMulExp();
#ifdef SYNTAX
                p.output << "<MulExp>" << endl;
#endif //SYNTAX
                t.push_back(mulExp);
                break;
            }
            default:
                break;
        }
    }

    shared_ptr<AddExpAST> Parser::parseAddExp() {
        vector<shared_ptr<MulExpAST> > mulExpList;
        vector<identifierType> symbolList;
        dealAddExp(mulExpList, *this);
        while (currentType == PLUS || currentType == MINU) {
#ifdef SYNTAX
            output << "<AddExp>" << endl;
#endif //SYNTAX
            identifierType type = currentType;
            readNext();
            symbolList.push_back(type);
            dealAddExp(mulExpList, *this);
        }
        return make_shared<AddExpAST>(std::move(mulExpList), std::move(symbolList));
    }

    shared_ptr<ConstDefAST> Parser::parseConstDef() {
        // currentType = IDENFR
        int line;
        string name = currentIdentifier;
        line = getLine();
        shared_ptr<ConstInitValAST> constInitVal;
        vector<shared_ptr<ConstExpAST> > constExpList;
        readNext();
        while (true) {
            if (currentType == ASSIGN) {
                readNext();
                constInitVal = parseConstInitVal();
#ifdef SYNTAX
                output << "<ConstInitVal>" << endl;
#endif //SYNTAX
                break;
            } else if (currentType == LBRACK) {
                readNext();
                shared_ptr<ConstExpAST> constExp = parseConstExp();
#ifdef SYNTAX
                output << "<ConstExp>" << endl;
#endif //SYNTAX
                constExpList.push_back(constExp);
                // currentType = RBRACK
                if (currentType != RBRACK) {
                    errors.emplace_back(getPreLine(), "k");
                } else {
                    readNext();
                }
            } else {
                break;
            }
            // cout << "In ConstDef" << endl;
        }
        return make_shared<ConstDefAST>(std::move(name), std::move(constExpList), std::move(constInitVal), line);
    }

    shared_ptr<BtypeAST> Parser::parseBtype() {
        switch (currentType) {
            case INTTK: {
                identifierType type = currentType;
                readNext();
                return make_shared<BtypeAST>(type, currentIdentifier);
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<ConstDeclAST> Parser::parseConstDecl() {
        // currentType = CONSTTK
        //string symbol = currentIdentifier;
        shared_ptr<BtypeAST> btype;
        vector<shared_ptr<ConstDefAST> > constDefList;
        readNext();
        // currentType = INTTK
        btype = parseBtype();
        // currentType = IDENFR
        shared_ptr<ConstDefAST> constDef = parseConstDef();
#ifdef SYNTAX
        output << "<ConstDef>" << endl;
#endif //SYNTAX
        constDefList.push_back(constDef);
        while (currentType == COMMA) {
            readNext();
            // currentType = IDENFR
            constDef = parseConstDef();
#ifdef SYNTAX
            output << "<ConstDef>" << endl;
#endif //SYNTAX
            constDefList.push_back(constDef);
        }
        // currentType = SEMICN
        if (currentType != SEMICN) {
            errors.emplace_back(getPreLine(), "i");
        } else {
            readNext();
        }
        return make_shared<ConstDeclAST>(std::move(btype), std::move(constDefList));
    }

    shared_ptr<InitValAST> Parser::parseInitVal() {
        shared_ptr<ExpAST> exp;
        vector<shared_ptr<InitValAST> > initValList;
        switch (currentType) {
            case PLUS:
            case MINU:
            case IDENFR:
            case LPARENT:
            case INTCON:
            case NOT: {
                exp = parseExp();
#ifdef SYNTAX
                output << "<Exp>" << endl;
#endif //SYNTAX
                return make_shared<InitValAST>(std::move(exp));
            }
            case LBRACE: {
                readNext();
                switch (currentType) {
                    case PLUS:
                    case MINU:
                    case IDENFR:
                    case LPARENT:
                    case INTCON:
                    case LBRACE: {
                        shared_ptr<InitValAST> initVal = parseInitVal();
#ifdef SYNTAX
                        output << "<InitVal>" << endl;
#endif //SYNTAX
                        initValList.push_back(initVal);
                        while (currentType == COMMA) {
                            readNext();
                            initVal = parseInitVal();
#ifdef SYNTAX
                            output << "<InitVal>" << endl;
#endif //SYNTAX
                            initValList.push_back(initVal);
                        }
                        // currentType == RBRACE;
                        readNext();
                        return make_shared<InitValAST>(std::move(initValList));
                    }
                    case RBRACE: {
                        readNext();
                        return make_shared<InitValAST>(std::move(initValList));
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
        cout << "wrong in initVal" << endl;
        return nullptr;
    }

    shared_ptr<VarDefAST> Parser::parseVarDef() {
        int line;
        string name;
        vector<shared_ptr<ConstExpAST> > constExpList;
        //currentType == IDENFR
        name = currentIdentifier;
        line = getLine();
        readNext();
        while (currentType == LBRACK) {
            readNext();
            shared_ptr<ConstExpAST> constExp = parseConstExp();
#ifdef SYNTAX
            output << "<ConstExp>" << endl;
#endif //SYNTAX
            constExpList.push_back(constExp);
            // currentType = RBRACK
            if (currentType != RBRACK) {
                errors.emplace_back(getPreLine(), "k");
            } else {
                readNext();
            }
        }
        if (currentType == ASSIGN) {
            readNext();
            if (currentType != GETINTTK) {
                shared_ptr<InitValAST> initVal = parseInitVal();
#ifdef SYNTAX
                output << "<InitVal>" << endl;
#endif //SYNTAX
                return make_shared<VarDefAST>(std::move(name), std::move(constExpList), line, false, std::move(initVal));
            }
            else {
                // getint();
                for (int i = 0; i < 3; ++i) readNext();
                return make_shared<VarDefAST>(std::move(name), std::move(constExpList), line, true);
            }
        } else {
            return make_shared<VarDefAST>(std::move(name), std::move(constExpList), line, false);
        }
    }

    shared_ptr<VarDeclAST> Parser::parseVarDecl() {
        shared_ptr<BtypeAST> btype;
        vector<shared_ptr<VarDefAST> > varDefList;
        // currentType = INTTK
        btype = parseBtype();
        // currentType = IDENFR
        shared_ptr<VarDefAST> varDef = parseVarDef();
#ifdef SYNTAX
        output << "<VarDef>" << endl;
#endif //SYNTAX
        varDefList.push_back(varDef);
        while (currentType == COMMA) {
            readNext();
            varDef = parseVarDef();
#ifdef SYNTAX
            output << "<VarDef>" << endl;
#endif //SYNTAX
            varDefList.push_back(varDef);
        }
        // currentType = SEMICN
        if (currentType != SEMICN) {
            errors.emplace_back(getPreLine(), "i");
        } else {
            readNext();
        }
        return make_shared<VarDeclAST>(std::move(btype), std::move(varDefList));
    }

    shared_ptr<DeclAST> Parser::parseDecl() {
        switch (currentType) {
            case CONSTTK: {
                shared_ptr<ConstDeclAST> constDecl = parseConstDecl();
#ifdef SYNTAX
                output << "<ConstDecl>" << endl;
#endif //SYNTAX
                return make_shared<DeclAST>(std::move(constDecl), 0);
            }
            case INTTK: {
                shared_ptr<VarDeclAST> varDecl = parseVarDecl();
#ifdef SYNTAX
                output << "<VarDecl>" << endl;
#endif //SYNTAX
                return make_shared<DeclAST>(std::move(varDecl), 1);
            }
            default:
                break;
        }
        return nullptr;
    }

    shared_ptr<CompUnitAST> Parser::parseCompUnit() {
        vector<shared_ptr<DeclAST> > declList;
        vector<shared_ptr<FuncDefAST> > funcDefList;
        shared_ptr<MainFuncDefAST> mainFuncDef;
        while (true) {
            switch (currentType) {
                case END:
                    break;
                case CONSTTK: {
                    // const type
                    shared_ptr<DeclAST> decl = parseDecl();
                    declList.push_back(decl);
                    break;
                }
                case VOIDTK: {
                    // func type
                    shared_ptr<FuncDefAST> funcDef = parseFuncDef();
#ifdef SYNTAX
                    output << "<FuncDef>" << endl;
#endif //SYNTAX
                    funcDefList.push_back(funcDef);
                    break;
                }
                case INTTK: {
                    // var or func or main
                    prefetch();
                    switch (prefetchType) {
                        case MAINTK: {
                            // main func
                            mainFuncDef = parseMainFuncDef();
#ifdef SYNTAX
                            output << "<MainFuncDef>" << endl;
#endif //SYNTAX
                            break;
                        }
                        case IDENFR :{
                            // fun or var
                            prefetch();
                            switch (prefetchType) {
                                case LPARENT: {
                                    // func
                                    shared_ptr<FuncDefAST> funcDef = parseFuncDef();
#ifdef SYNTAX
                                    output << "<FuncDef>" << endl;
#endif //SYNTAX
                                    funcDefList.push_back(funcDef);
                                    break;
                                }
                                case ASSIGN:
                                case LBRACK:
                                case COMMA:
                                case SEMICN: {
                                    // var
                                    shared_ptr<DeclAST> decl = parseDecl();
                                    declList.push_back(decl);
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
            if (currentType == END) break;
            // cout << "In compUnit" << endl;
        }
        return make_shared<CompUnitAST>(std::move(declList), std::move(funcDefList), std::move(mainFuncDef));
    }
}