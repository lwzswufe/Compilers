#include <string>
using std::string;
#include "ParseGrammar.h"

/*
g++ Grammar.h ParseGrammar.h ParseGrammar.cpp Test.cpp -o Test.o
*/

void ReadGrammarTest1()
{
    vector<Grammar> VecG = ReadGrammar("Grammar1.txt");
    for (Grammar A: VecG)
    {
        PrintGrammar(A);
    }
    // 计算项集
    vector<vector<ItemObj>> VecItemCollection = ItemGenerator(VecG);
    PrintItemCollections(VecItemCollection, VecG);
    // 增广文法 使用Closure函数补充项集
    Closure(VecItemCollection, VecG);
    PrintItemCollections(VecItemCollection, VecG);
    // 使用GoTo函数补充项集
    GotoItem(VecItemCollection, VecG);
    PrintItemCollections(VecItemCollection, VecG);
    // 计算分析表
    vector<vector<AnalysisTableUnit>> LRTable = GeneratorAnalysysTable(VecItemCollection, VecG);
    const vector<GrammarUnit> VecGU = GeneratorGrammarUnitVec(VecG);
    PrintAnalysisTable(LRTable, VecGU);
    // 翻译
    // 输入数据
    vector<string> VecS{"id", "+", "id", "*", "id", "$"};
    // vector<string> VecS{"id", "+", "id", "$"};
    stack<GrammarUnit> StkInput{};
    // 倒序添加
    for (unsigned i=VecS.size(); i>0; --i)
    {
        StkInput.push(CreateGrammarUnit(VecS[i-1].c_str(), true));
    }
    // 语法分析
    const GrammarUnit* pRoot = GrammarAnalysis(LRTable, VecG, StkInput);
    PrintGrammarAnalysisTree(pRoot);
}


void ReadGrammarTest2()
{
    vector<Grammar> VecG = ReadGrammar("Grammar2.txt");
    for (Grammar A: VecG)
    {
        PrintGrammar(A);
    }
    // 计算First集合
    map<GrammarUnit, set<GrammarUnit>> FirstTerminalMap = First(VecG);
    // 计算项集
    vector<vector<ItemObj>> VecItemCollection = ItemGeneratorLR1(VecG, FirstTerminalMap);
    PrintItemCollections(VecItemCollection, VecG);
    // 计算分析表
    vector<vector<AnalysisTableUnit>> LRTable = GeneratorAnalysysTableLR1(VecItemCollection, VecG);
    const vector<GrammarUnit> VecGU = GeneratorGrammarUnitVec(VecG);
    PrintAnalysisTable(LRTable, VecGU);
    // 翻译
    // 输入数据
    vector<string> VecS{"c", "d", "c", "c", "d", "$"};
    // vector<string> VecS{"id", "+", "id", "$"};
    stack<GrammarUnit> StkInput{};
    // 倒序添加
    for (unsigned i=VecS.size(); i>0; --i)
    {
        StkInput.push(CreateGrammarUnit(VecS[i-1].c_str(), true));
    }
    // 语法分析
    const GrammarUnit* pRoot = GrammarAnalysis(LRTable, VecG, StkInput);
    PrintGrammarAnalysisTree(pRoot);
}

int main()
{   
    ReadGrammarTest2();
    return 0;
}