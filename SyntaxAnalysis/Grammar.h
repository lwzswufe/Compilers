#include <vector>
#include <string.h>
#define UNIT_TYPE_NULL        0     // 语法单元类型 未定义
#define UNIT_TYPE_TERMINAL    1     // 语法单元类型 终结符
#define UNIT_TYPE_NONTERMINAL 2     // 语法单元类型 非终结符
#define AT_UNIT_TYPE_NULL    '0'    // 分析表类型 未定义
#define AT_UNIT_TYPE_ACCEPT  'a'    // 分析表类型 接受
#define AT_UNIT_TYPE_REDUCE  'r'    // 分析表类型 归约
#define AT_UNIT_TYPE_SHIFT   's'    // 分析表类型 移入
#define AT_UNIT_TYPE_GOTO    'g'    // 分析表类型 跳转
#define AT_UNIT_TYPE_ERROR   'e'    // 分析表类型 错误
#define GRAMMAR_NAME_SIZE    31     // 语法单元名称大小
#define FORWARD_CHAR_NUM     32     // 向前看字符数量
// 文法单元类
struct GrammarUnit
{   
    char type;
    char name[GRAMMAR_NAME_SIZE];
    char forward[FORWARD_CHAR_NUM];
    GrammarUnit *child;             // 子节点
    GrammarUnit *brother;           // 兄弟节点
    GrammarUnit *parent;            // 父节点
    friend bool operator<(const GrammarUnit& LGU, const GrammarUnit& RGU) {return strcmp(LGU.name, RGU.name) < 0;};
};

// 增广文法
struct Grammar
{
    std::vector<GrammarUnit> arr;          // 起始文法符号符
};


// 分析表单元
struct AnalysisTableUnit
{
    char type;  // 类型 接受/错误/归约/移入/跳转
    int  num;   // 错误/归约/移入/跳转 编号
};

// 项
struct ItemObj
{
    int production_id;  // 产生式编号
    int position_id;    // 位置编码    
    GrammarUnit forward;// 向前看符号
};