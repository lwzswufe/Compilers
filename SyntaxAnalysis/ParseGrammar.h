#include "Grammar.h"
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <stack>
using std::stack;
using std::deque;
#include <set>
using std::set;

struct MarkNode
{
    const GrammarUnit* pNode;       // 节点
    int Level;                      // 深度
};
// 打印语法信息
void PrintGrammar(const Grammar &VecG, int Pos=-1, const GrammarUnit* pForward=nullptr);
// 打印项集
void PrintItemCollections(const vector<vector<ItemObj>>& VecVecItem, const vector<Grammar> &VecG);
// 打印分析表
void PrintAnalysisTable(const vector<vector<AnalysisTableUnit>> &LRTable, const vector<GrammarUnit> VecGU);
// 打印语法分析树
void PrintGrammarAnalysisTree(const GrammarUnit* pRoot);
// 创建语法单元结构体
GrammarUnit CreateGrammarUnit(const char* name, bool IsTerminal);
// 读取一套文法
vector<Grammar> ReadGrammar(const char* filename);
// 计算项集
vector<vector<ItemObj>> ItemGenerator(const vector<Grammar> &VecG);
// 计算增广文法
void Closure(vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG);
// 计算项集转换函数
// map<GrammarUnit, int> 每个项集的转换字典 
// GrammarUnit 终结符/非终结符  int 转换后的项集
void GotoItem(vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG);
// 生成语法单元数组 用于分析表
vector<GrammarUnit> GeneratorGrammarUnitVec(const vector<Grammar> &VecG);
// 分析表生成算法
vector<vector<AnalysisTableUnit>> GeneratorAnalysysTable(const vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG);
// 语法分析
// 返回语法分析树头节点
GrammarUnit* GrammarAnalysis(const vector<vector<AnalysisTableUnit>> &Table, const vector<Grammar> &VecG, stack<GrammarUnit> &VecInput);
// 寻找文法符号Beta的首元素集合 暂时不考虑符号为空的情况
map<GrammarUnit, set<GrammarUnit>> First(const vector<Grammar> &VecAG);
// 计算项集
vector<vector<ItemObj>> ItemGeneratorLR1(const vector<Grammar> &VecAG, map<GrammarUnit, set<GrammarUnit>> &FirstTerminalMap);
// 分析表生成算法 LR1
vector<vector<AnalysisTableUnit>> GeneratorAnalysysTableLR1(const vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG);