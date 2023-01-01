//
// Created by Leo on 2022/10/11.
//

#include "ErrorProcesser.h"
#include <iostream>
#include <vector>
#include <string>

namespace error {
    using namespace ast;

    SymbolTable symbolTable;
    vector<pair<int, string> > errors; // (line, type)

    ErrorProcesser::ErrorProcesser(const char *err, shared_ptr<CompUnitAST> &compUnitAST) : compUnitAST(compUnitAST) {
        this->err.open(err, ios::out);
        this->currentDomain = 0;
        this->currentFunction = "";
        this->isLoop.push(false);
    }

    void ErrorProcesser::analysis() {
        vector<shared_ptr<DeclAST> > &decls = compUnitAST->getDecls();
        for (auto &decl: decls) {
            processDecl(decl);
        }
        vector<shared_ptr<FuncDefAST> > &funcDefs = compUnitAST->getFuncDefs();
        for (auto &funcDef: funcDefs) {
            processFuncDef(funcDef);
        }
        shared_ptr<MainFuncDefAST> &mainFuncDef = compUnitAST->getMainFuncDef();
        processMainDef(mainFuncDef);
    }

    void ErrorProcesser::print() {
        sort(errors.begin(), errors.end(), [](const pair<int, string> &a, const pair<int, string> &b) {
            return a.first < b.first;
        });
        errors.erase(unique(errors.begin(), errors.end()), errors.end());
        for (auto &error: errors) {
            err << error.first << " " << error.second << endl;
        }
    }

    int ErrorProcesser::processAddExp(shared_ptr<AddExpAST> &addExpAST) {
        vector<shared_ptr<MulExpAST> > &mulExps = addExpAST->getMulExp();
        int d = 0;
        for (auto &mulExp: mulExps) {
            d = processMulExp(mulExp);
        }
        return d;
    }

    int ErrorProcesser::processMulExp(shared_ptr<MulExpAST> &mulExpAST) {
        vector<shared_ptr<UnaryExpAST> > &unaryExps = mulExpAST->getUnaryExp();
        int d = 0;
        for (auto &unaryExp: unaryExps) {
            d = processUnaryExp(unaryExp);
        }
        return d;
    }

    void ErrorProcesser::processFuncRParams(shared_ptr<FuncRParamsAST> &funcRParamsAST, vector<int> &dimensions) {
        for (auto &i: funcRParamsAST->getExps()) {
            dimensions.emplace_back(processExp(i));
        }
    }

    int ErrorProcesser::processPrimaryExp(shared_ptr<PrimaryExpAST> &primaryExp) {
        int d = 0;
        switch (primaryExp->getType()) {
            case 0: {
                shared_ptr<ExpAST> exp = dynamic_pointer_cast<ExpAST>(primaryExp->getExp());
                d = processExp(exp);
                break;
            }
            case 1: {
                shared_ptr<LValAST> lval = dynamic_pointer_cast<LValAST>(primaryExp->getExp());
                bool isArray;
                d = processLVal(lval, isArray);
                break;
            }
            case 2: {
                d = 0;
            }
            default: {
                break;
            }
        }
        return d;
    }

    int ErrorProcesser::processUnaryExp(shared_ptr<UnaryExpAST> &unaryExp) {
        int d = 0;
        switch (unaryExp->getType()) {
            case 0: {
                // primaryExp
                d = processPrimaryExp(unaryExp->getPrimaryExp());
                break;
            }
            case 1 : {
                // function
                string name = unaryExp->getName();
                bool flag = false;
                int index;
                for (index = 0; index < symbolTable.getFuncTable().size(); ++index) {
                    if (symbolTable.getFuncTable()[index].getName() == name) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    FuncSymbol funcSymbol = symbolTable.getFuncTable()[index];
                    // check whether the number of parameters is correct
                    int acquiredNum = funcSymbol.getParameterNum();
                    int runningNum = 0;
                    if (unaryExp->getFuncRParams() != nullptr) {
                        runningNum = (int) unaryExp->getFuncRParams()->getExps().size();
                    }
                    if (acquiredNum != runningNum) {
                        errors.emplace_back(unaryExp->getLine(), "d");
                        break;
                    }
                    // check whethr the type of parameters is correct
                    if (runningNum != 0) {
                        vector<int> dimensions;
                        processFuncRParams(unaryExp->getFuncRParams(), dimensions);
                        for (int i = 0; i < dimensions.size(); ++i) {
                            if (funcSymbol.getParameters()[i].getDimension() != dimensions[i]) {
                                errors.emplace_back(unaryExp->getLine(), "e");
                                break;
                            }
                        }
                    }
                    if (funcSymbol.getType() == INTTK) {
                        d = 0;
                    } else {
                        d = -1;
                    }
                } else {
                    // function is not defined
                    errors.emplace_back(unaryExp->getLine(), "c");
                }
                break;
            }
            case 2: {
                d = processUnaryExp(unaryExp->getUnaryExp());
                break;
            }
            default: {
                break;
            }
        }
        return d;
    }

    void ErrorProcesser::processRelExp(shared_ptr<RelExpAST> &relExp) {
        for (auto &i: relExp->getAddExp()) {
            processAddExp(i);
        }
    }

    void ErrorProcesser::processEqExp(shared_ptr<EqExpAST> &eqExp) {
        for (auto &i: eqExp->getRelExp()) {
            processRelExp(i);
        }
    }

    void ErrorProcesser::processLAndExp(shared_ptr<LAndExpAST> &lAndExp) {
        for (auto &i: lAndExp->getEqExp()) {
            processEqExp(i);
        }
    }

    void ErrorProcesser::processLOrExp(shared_ptr<LOrExpAST> &lOrExp) {
        for (auto &i: lOrExp->getLAndExp()) {
            processLAndExp(i);
        }
    }

    void ErrorProcesser::processCond(shared_ptr<CondAST> &cond) {
        processLOrExp(cond->getLOrExp());
    }

    void ErrorProcesser::processConstExp(shared_ptr<ConstExpAST> &constExp) {
        processAddExp(constExp->getAddExp());
    }

    int ErrorProcesser::processExp(shared_ptr<ExpAST> &exp) {
        return processAddExp(exp->getAddExp());
    }

    void ErrorProcesser::processInitVal(shared_ptr<InitValAST> &initVal) {
        if (initVal->getExp() != nullptr) {
            processExp(initVal->getExp());
        } else {
            for (auto &i: initVal->getInitVals()) {
                processInitVal(i);
            }
        }
    }

    void ErrorProcesser::processConstInitVal(shared_ptr<ConstInitValAST> &constInitValAST) {
        if (constInitValAST->getConstExp() != nullptr) {
            processConstExp(constInitValAST->getConstExp());
        } else {
            for (auto &i: constInitValAST->getConstInitVals()) {
                processConstInitVal(i);
            }
        }
    }

    void ErrorProcesser::processConstDecl(shared_ptr<ConstDeclAST> &constDecl) {
        currentType.push(constDecl->getBtype()->getType());
        for (auto &i: constDecl->getConstDefs()) {
            processConstDef(i);
        }
        currentType.pop();
    }

    void ErrorProcesser::processConstDef(shared_ptr<ConstDefAST> &constDef) {
        string name = constDef->getName();
        int line = constDef->getLine();
        int dimension = (int) constDef->getConstExps().size();

        // check whether the name has been defined
        for (auto &i: symbolTable.getConstTable()) {
            if (i.getName() == name && i.getDomain() == currentDomain) {
                errors.emplace_back(line, "b");
                return;
            }
        }

        for (auto &i: symbolTable.getVarTable()) {
            if (i.getName() == name && i.getDomain() == currentDomain) {
                errors.emplace_back(line, "b");
                return;
            }
        }

        for (auto &i: constDef->getConstExps()) {
            processConstExp(i);
        }

        processConstInitVal(constDef->getConstInitVal());
        ConstSymbol constSymbol(name, currentType.top(), currentDomain, dimension);
        symbolTable.getConstTable().push_back(constSymbol);
    }

    void ErrorProcesser::processVarDecl(shared_ptr<VarDeclAST> &varDecl) {
        currentType.push(varDecl->getBType()->getType());
        for (auto &i: varDecl->getVarDefs()) {
            processVarDef(i);
        }
        currentType.pop();
    }

    void ErrorProcesser::processVarDef(shared_ptr<VarDefAST> &varDef) {
        string varName = varDef->getName();
        int dimension = (int) varDef->getConstExps().size();
        int line = varDef->getLine();
        // check whether it contains the duplicated name;
        for (auto &i: symbolTable.getConstTable()) {
            if (i.getName() == varName && i.getDomain() == currentDomain) {
                errors.emplace_back(line, "b");
                break;
            }
        }

        for (auto &i: symbolTable.getVarTable()) {
            if (i.getName() == varName && i.getDomain() == currentDomain) {
                errors.emplace_back(line, "b");
                break;
            }
        }
        vector<shared_ptr<ConstExpAST> > &constExps = varDef->getConstExps();
        for (auto &constExp: constExps) {
            processConstExp(constExp);
        }

        if (varDef->getInitVal() != nullptr) {
            processInitVal(varDef->getInitVal());
        }

        VarSymbol varSymbol(varName, currentType.top(), dimension, currentDomain);
        symbolTable.getVarTable().emplace_back(varSymbol);
    }

    void ErrorProcesser::processDecl(shared_ptr<DeclAST> &decl) {
        if (decl->getType() == 0) {
            shared_ptr<ConstDeclAST> constDecl = dynamic_pointer_cast<ConstDeclAST>(decl->getDecl());
            processConstDecl(constDecl);
        } else {
            shared_ptr<VarDeclAST> varDecl = dynamic_pointer_cast<VarDeclAST>(decl->getDecl());
            processVarDecl(varDecl);
        }
    }

    void ErrorProcesser::processFuncFParam(shared_ptr<ast::FuncParamAST> &funcParam, vector<VarSymbol> &params) {
        currentType.push(funcParam->getBType()->getType());
        string name = funcParam->getName();
        int dimension = (int) funcParam->getConstExps().size();
        int line = funcParam->getLine();

        for (auto &i: symbolTable.getVarTable()) {
            if (i.getName() == name && i.getDomain() == currentDomain) {
                errors.emplace_back(line, "b");
                currentType.pop();
                return;
            }
        }

        for (int i = 0; i < funcParam->getConstExps().size(); i++) {
            if (i == 0) continue;
            processConstExp(funcParam->getConstExps()[i]);
        }

        VarSymbol varSymbol(name, currentType.top(), dimension, currentDomain);
        symbolTable.getVarTable().emplace_back(varSymbol);
        params.push_back(varSymbol);
        currentType.pop();
    }

    int ErrorProcesser::processLVal(shared_ptr<LValAST> &lVal, bool &con) {
        string name = lVal->getName();
        bool flag = false;
        int line = lVal->getLine();
        int index;
        con = false;

        for (int i = (int) symbolTable.getConstTable().size() - 1; i >= 0; i--) {
            if (symbolTable.getConstTable()[i].getName() == name) {
                flag = true;
                con = true;
                index = i;
                break;
            }
        }

        for (int i = (int) symbolTable.getVarTable().size() - 1; i >= 0; i--) {
            if (symbolTable.getVarTable()[i].getName() == name) {
                flag = true;
                index = i;
                break;
            }
        }

        if (!flag) {
            errors.emplace_back(line, "c");
        }

        if (flag) {
            int dimension, lvalDimension = (int) lVal->getConstExps().size();
            dimension =  con ? symbolTable.getConstTable()[index].getDimension() :
                        symbolTable.getVarTable()[index].getDimension();
            return dimension - lvalDimension;
        }
        // -2 means error
        return -2;
    }

    bool ErrorProcesser::processPrint(std::string &format, int num) {
        int total = 0;
        // check whether the number of %d is equal to print elements
        for (int i = 0; i < format.size(); ++i) {
            if (format[i] == '%') {
                if (format[i + 1] == 'd') {
                    ++total;
                    ++i;
                }
            }
        }
        if (total == num) {
            return true;
        } else {
            return false;
        }
    }

    void ErrorProcesser::processStmt(shared_ptr<ast::StmtAST> &stmt) {
        switch (stmt->getType()) {
            case 0: {
                bool isConst;
                processLVal(stmt->getLVal(), isConst);
                if (isConst) {
                    errors.emplace_back(stmt->getLine(), "h");
                }
                processExp(stmt->getExp());
                break;
            }
            case 1: {
                if (stmt->getExpSingle() != nullptr)
                    processExp(stmt->getExpSingle());
                break;
            }
            case 2: {
                currentDomain++;
                processBlock(stmt->getBlock(), false);
                currentDomain--;
                break;
            }
            case 3: {
                processCond(stmt->getCondIfExps());
                processStmt(stmt->getStmtIf());
                if (stmt->getStmtElse() != nullptr) {
                    processStmt(stmt->getStmtElse());
                }
                break;
            }
            case 4: {
                isLoop.push(true);
                processCond(stmt->getCondWhile());
                processStmt(stmt->getStmtWhile());
                isLoop.pop();
                break;
            }
            case 5: {
                if (!isLoop.top()) {
                    errors.emplace_back(stmt->getLine(), "m");
                }
                break;
            }
            case 6: {
                bool hasReturnValue = false;
                if (stmt->getExpReturn() == nullptr) {
                    hasReturnValue = true;
                }
                if (!hasReturnValue && returnType == VOIDTK) {
                    errors.emplace_back(stmt->getLine(), "f");
                }
                break;
            }
            case 7: {
                bool isConst = false;
                processLVal(stmt->getLValGetinit(), isConst);
                if (isConst) {
                    errors.emplace_back(stmt->getLine(), "h");
                }
                break;
            }
            case 8: {
                // check the num
                bool isEqual = processPrint(stmt->getFormat(), (int) stmt->getExpPrint().size());
                if (!isEqual) {
                    errors.emplace_back(stmt->getLine(), "l");
                }
                // then check the Exp itself thoroughly
                for (auto &i: stmt->getExpPrint()) {
                    processExp(i);
                }
                break;
            }
            default:
                break;
        }
    }

    void ErrorProcesser::processBlockItem(shared_ptr<ast::BlockItemAST> &blockItem) {
        if (blockItem->getType() == 0) {
            shared_ptr<ast::DeclAST> decl = dynamic_pointer_cast<ast::DeclAST>(blockItem->getBlockItem());
            processDecl(decl);
        } else {
            shared_ptr<ast::StmtAST> stmt = dynamic_pointer_cast<StmtAST>(blockItem->getBlockItem());
            processStmt(stmt);
        }
    }

    void ErrorProcesser::processBlock(shared_ptr<ast::BlockAST> &block, bool isFunction) {
        vector<shared_ptr<ast::BlockItemAST> > &blockItems = block->getBlockItems();
        for (auto &i: blockItems) {
            processBlockItem(i);
        }
        // pop out the variables in this block
        /* for (auto i = symbolTable.getVarTable().begin(); i != symbolTable.getVarTable().end();) {
             if (i->getDomain() == currentDomain) {
                 symbolTable.getVarTable().erase(i);
             } else {
                 i++;
             }
         }
         for (auto i = symbolTable.getConstTable().begin(); i != symbolTable.getConstTable().end();) {
             if (i->getDomain() == currentDomain) {
                 symbolTable.getConstTable().erase(i);
             } else {
                 i++;
             }
         } */
        symbolTable.clearCurrentVarTable(currentDomain);
        symbolTable.clearCurrentConstTable(currentDomain);
        if (blockItems.empty()) {
            if (isFunction && returnType == INTTK)
                errors.emplace_back(block->getLine(), "g");
            return;
        }
        shared_ptr<BlockItemAST> last = blockItems.back();
        if (last->getType() == 0) {
            if (isFunction && returnType == INTTK) {
                errors.emplace_back(block->getLine(), "g");
            }
        } else {
            shared_ptr<StmtAST> stmt = dynamic_pointer_cast<StmtAST>(last->getBlockItem());
            if (stmt->getType() == 6) {
                if (stmt->getExpReturn() == nullptr && returnType == INTTK) {
                    errors.emplace_back(block->getLine(), "g");
                }
            } else {
                if (isFunction && returnType == INTTK) {
                    errors.emplace_back(block->getLine(), "g");
                }
            }
        }
    }

    void ErrorProcesser::processFuncDef(shared_ptr<ast::FuncDefAST> &func) {
        currentType.push(func->getFuncType()->getCategory());
        returnType = func->getFuncType()->getCategory();
        isLoop.push(false);
        string funcName = func->getName();
        int line = func->getLine();
        for (auto &i: symbolTable.getFuncTable()) {
            if (i.getName() == funcName) {
                errors.emplace_back(line, "b");
                break;
            }
        }
        currentDomain++;
        shared_ptr<ast::FuncFParamsAST> &params = func->getParams();
        vector<VarSymbol> paramList;
        int total = 0;
        if (params != nullptr) {
            for (auto &i: params->getFuncParams()) {
                processFuncFParam(i, paramList);
                total++;
            }
        }
        FuncSymbol funcSymbol(funcName, currentType.top(), paramList);
        symbolTable.getFuncTable().push_back(funcSymbol);
        shared_ptr<ast::BlockAST> &block = func->getBlock();
        if (block != nullptr) {
            processBlock(block, true);
        }
        currentType.pop();
        isLoop.pop();
        currentDomain--;
    }

    void ErrorProcesser::processMainDef(shared_ptr<MainFuncDefAST> &mainFuncDef) {
        isLoop.push(false);
        string name = "main";
        currentDomain++;
        currentType.push(INTTK);
        returnType = INTTK;
        int line = mainFuncDef->getLine();
        for (auto &i: symbolTable.getFuncTable()) {
            if (i.getName() == name) {
                errors.emplace_back(line, "b");
                break;
            }
        }
        shared_ptr<BlockAST> &block = mainFuncDef->getBlock();
        processBlock(block, true);
        vector<VarSymbol> paramList;
        FuncSymbol funcSymbol(name, currentType.top(), paramList);
        symbolTable.getFuncTable().push_back(funcSymbol);
        currentDomain--;
        isLoop.pop();
        currentType.pop();
    }

    ErrorProcesser::~ErrorProcesser() {
        isLoop.pop();
        this->err.close();
    }
}
