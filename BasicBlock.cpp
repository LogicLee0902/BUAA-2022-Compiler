//
// Created by Leo on 2022/12/2.
//

#include "include/BasicBlock.h"
#include <iostream>
#include <algorithm>
#include <memory>

using namespace std;

void GenerateBlock::run() {
    vector<int> cutDots;
    bool flag = false;
    for (int i = 0; i < IRCodeList.size(); i++) {
        if (!flag) {
            if (IRCodeList[i]->getOp() == MainOT) {
                flag = true;
                setStart(i);
                continue;
            } else {
                continue;
            }
        }
       if (IRCodeList[i]->getOp() == JumpOT) {
           cutDots.push_back(i+1);
       } else if (IRCodeList[i]->getOp() == FuncOT) {
           cutDots.push_back(i);
       } else if (IRCodeList[i]->getOp() == CallOT) {
           cutDots.push_back(i+1);
       } else if (IRCodeList[i]->getOp() == ReturnOT) {
           cutDots.push_back(i+1);
       } else if (IRCodeList[i]->getOp() == LabelOT) {
           cutDots.push_back(i);
       }
    }
    sort(cutDots.begin(), cutDots.end());
    cutDots.erase(unique(cutDots.begin(), cutDots.end()), cutDots.end());

    int cnt = 0;
    while (cnt < cutDots.size()) {
        shared_ptr<BasicBlock> basicBlock;
        for (int i = cutDots[cnt]; i < (cnt == cutDots.size() - 1 ? IRCodeList.size() : cutDots[cnt+1]); i++) {
            if (i == cutDots[cnt]) {
                basicBlock = make_shared<BasicBlock>("B" + to_string(cnt));
            }
            basicBlock->addCode(IRCodeList[i]);
        }
        basicBlocks.push_back(basicBlock);
        ++cnt;
    }
    fstream output("test.txt", ios::out);
    cout << basicBlocks.size() << endl;

    for (int i =0; i < this->getBasicBlocks().size(); i++) {
        cout << "NEW" << endl;
        cout << "Lines = " << basicBlocks[i]->getIRCodes().size() << endl;
        output << "----------------------------" << endl;
        for (int j = 0; j < basicBlocks[i]->getIRCodes().size(); ++ j) {
            //cout << operatorName[basicBlocks[i]->getIRCodes()[j]->getOp()] << endl;
           basicBlocks[i]->getIRCodes()[j]->Print(output);
        }
        cout << "OVer" << endl;
    }

}