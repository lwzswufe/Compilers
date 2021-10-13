// 解析文法
#include <string.h>
#include <stdio.h>
#include <utility>
using std::pair;
#include "ParseGrammar.h"

// 从line的st个字符开始识别字符串 忽略非空格符号前的所有空格字符
int GetrWord(const char* line, char* str, int st)
{
    int i{st};
    char c {line[i]};
    memset(str, 0, GRAMMAR_NAME_SIZE);
    // 去除非空白字符前的空白字符
    while (c!=0 && c==' ')
    {
        ++st;
        c = line[++i];
    }
    // 寻找到末尾
    if (c == 0)
    {
        return -1;
    }
    // 读取字符
    while (c!=0 && c!=' ')
    {   
        c = line[++i];
    }
    strncpy(str, line+st, i-st);
    return i;
}

// 创建语法单元结构体
GrammarUnit CreateGrammarUnit(const char* name, bool IsTerminal)
{
    GrammarUnit Unit;
    if (name == nullptr || strlen(name) == 0)
    {
        Unit.type = UNIT_TYPE_NULL;
        Unit.name[0] = 0;
    }
    else if (IsTerminal)
    {
        Unit.type = UNIT_TYPE_TERMINAL;
        strncpy(Unit.name, name, GRAMMAR_NAME_SIZE);
    }
    else
    {
        Unit.type = UNIT_TYPE_NONTERMINAL;
        strncpy(Unit.name, name, GRAMMAR_NAME_SIZE);
    }
    return Unit;
}

// 创建语法单元结构体
GrammarUnit* CreateGrammarTreeNode(const char* name, bool IsTerminal)
{
    GrammarUnit *pUnit = new GrammarUnit;
    memset(pUnit, 0, sizeof(GrammarUnit));
    if (IsTerminal)
    {
        pUnit->type = UNIT_TYPE_TERMINAL;
        strncpy(pUnit->name, name, GRAMMAR_NAME_SIZE);
    }
    else
    {
        pUnit->type = UNIT_TYPE_NONTERMINAL;
        strncpy(pUnit->name, name, GRAMMAR_NAME_SIZE);
    }
    return pUnit;
}


// 检查一行文法有没有-> 并把第一个 -> 转化为 空格
bool CheckLine(char *line)
{   
    int i{0};
    int count{0};
    while(line[i++] != 0)
    {
        if (line[i] == '-' and line[i+1] == '>' and count == 0)
        {
            line[i] = ' ';
            line[i+1] = ' ';
            ++count;
        }
        if (line[i] == '\n')
        {
            line[i] = 0;
        }
    }
    return count > 0;
}

// 读取一行数据 返回一条文法
// 格式  str->str1 symbol1 str2 
Grammar ReadOneGrammar(char* line)
{   
    int st{0};
    char str[GRAMMAR_NAME_SIZE];
    bool IsTerminal;
    Grammar VecG{};
    if (!CheckLine(line))
        return VecG;
    st = GetrWord(line, str, st);
    // printf("get word:%s idx:%d line[2]:%c\n", str, st, line[2]);
    while (st >= 0)
    {   
        if (str[0] >= 'A' && str[0] <= 'Z')
            IsTerminal = false;
        else
            IsTerminal = true;
        GrammarUnit Unit = CreateGrammarUnit(str, IsTerminal);
        VecG.arr.push_back(Unit);
        st = GetrWord(line, str, st);
        // printf("get word:%s idx:%d\n", str, st);
    }
    return VecG;
}

void PrintGrammar(const Grammar &VecG, int pos, const GrammarUnit* pForward)
{   
    int count = 0;
    const char* name;
    for (unsigned i=0; i<VecG.arr.size(); ++i)
    {   
        GrammarUnit Unit{VecG.arr[i]};
        if (Unit.type == UNIT_TYPE_TERMINAL)
            name = Unit.name;
        else
            name = Unit.name;
        if (i == 0)
        {   
            printf("%s ->", name);
        }
        else
        {
            printf(" %s", name);
        }
        if (pos >= 0 && pos == i)
        {
            printf(" @");   // 打印当前节点
        }
    }
    if (pForward != nullptr)
        printf("   forward:%s\n", pForward->name);
    else
        printf("   pos=%d ptr=%p\n", pos, pForward);
}

void PrintItem(const ItemObj &Item, const vector<Grammar> &VecG)
{
    if (Item.production_id < VecG.size())
    {
        PrintGrammar(VecG[Item.production_id], Item.position_id, &(Item.forward));
    }
    else
    {
        printf("production_id: %d out of range[0, %lu-1]", Item.production_id, VecG.size());
    }
}


vector<Grammar> ReadGrammar(const char* filename)
{
    FILE * pFile;
    char buffer [1024];
    // 文法符号集合
    vector<Grammar> VecVecG{};
    pFile = fopen (filename , "r");
    if (pFile == NULL) 
    {
        perror ("Error opening file");   // 输出异常
    }
    else
    {
        while (fgets(buffer, 1024, pFile) != nullptr )
        {   
            Grammar VecG = ReadOneGrammar(buffer);
            VecVecG.push_back(VecG);
            // PrintGrammar(pUnit);
        }
    }
    return VecVecG;
}

GrammarUnit GetNullGrammarUnit()
{
    GrammarUnit GU;
    GU.type = UNIT_TYPE_NULL;
    sprintf(GU.name, "NULL");
    return GU;
}

GrammarUnit GetNextGrammarUnit(const Grammar &G, const ItemObj &Item)
{   
    if (Item.position_id+1 < G.arr.size())
        return G.arr[Item.position_id+1];
    else
        return GetNullGrammarUnit();
}

GrammarUnit GetNextGrammarUnit(const vector<Grammar> &VecG, const ItemObj &Item)
{
    return GetNextGrammarUnit(VecG[Item.production_id], Item);
}

bool Equal(const GrammarUnit& LUnit, const GrammarUnit &RUnit)
{
    return strcmp(LUnit.name, RUnit.name) == 0;
}

// 在VecVecItem里寻找具有共同左部的项 没有找到返回-1
// vector<vector<ItemObj>> &VecVecItem  待查找的项集
// vector<Grammar> &VecAG  产生式列表
// ItemObj &Item 待比较的项
int FindCommonLeftItem(const vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecAG, const ItemObj &RItem)
{   
    // 遍历每一个项集
    for (unsigned VecID=0; VecID<VecVecItem.size(); ++VecID)
    {
        const vector<ItemObj> &VecItem{VecVecItem[VecID]};
        // 遍历每一个项
        for (unsigned ItemID=0; ItemID<VecItem.size(); ++ItemID)
        {   
            const ItemObj &LItem = VecItem[ItemID];
            if (LItem.position_id != RItem.position_id)
            {
                continue; // 未找到 继续寻找
            }
            const Grammar &LAG{VecAG[LItem.production_id]};
            const Grammar &RAG{VecAG[RItem.production_id]};
            bool b{true};
            for (int Pos=0; Pos<RItem.position_id; ++Pos)
            {   
                const GrammarUnit &LUnit = LAG.arr[Pos+1];
                const GrammarUnit &RUnit = RAG.arr[Pos+1];
                if (!Equal(LUnit, RUnit))
                {   
                    b = false;
                    break;
                }
            }
            if (b)
                return VecID;   // 找到
        }
    }
    return -1;  // 没有找到
}

// 计算项集
vector<vector<ItemObj>> ItemGenerator(const vector<Grammar> &VecAG)
{
    vector<vector<ItemObj>> VecVecItem {};
    int pro_num{(int)VecAG.size()};
    int MaxSize{0};
    // 产生式循环
    for (int pro_id=0; pro_id<pro_num; ++pro_id)
    {
        const Grammar &AG = VecAG[pro_id];
        if (AG.arr.size() > MaxSize)
            MaxSize = AG.arr.size();
    }
    // 产生式位置循环
    for (int pos=0; pos<MaxSize; ++pos)
    {   
        // 产生式循环
        for (int pro_id=0; pro_id<pro_num; ++pro_id)
        {   
            const Grammar &G = VecAG[pro_id];
            // 坐标验证
            if (pos >= G.arr.size())
            {
                continue;
            }
            ItemObj new_item{pro_id, pos};
            // printf("Product:%d size:%lu Position:%d\n", pro_id, G.arr.size(), pos);
            // 寻找相同左部的项集
            int collection_id = FindCommonLeftItem(VecVecItem, VecAG, new_item);
            if (collection_id >= 0) // 发现有相同左部的项
            {
                VecVecItem[collection_id].push_back(new_item);
            }
            else    // 未发现有相同左部的项
            {   
                vector<ItemObj> VecItem{new_item};
                VecVecItem.push_back(VecItem);
            }
        }
    }
    return VecVecItem;
}

// 寻找可以生成指定非终结符NT 产生式集合
void AddChildNode(const vector<Grammar> &VecG, map<GrammarUnit, set<int>> &GrammarMap, const GrammarUnit& NT)
{
    for (int pro_id: GrammarMap[NT])
    {   
        const Grammar &G = VecG[pro_id];
        // 如果当前产生式右部是非终结符 则继续深入搜索
        if (G.arr[1].type == UNIT_TYPE_TERMINAL)
            continue;
        const GrammarUnit ChildNT = G.arr[1];
        // 产生式右部第一个非中介符与当前符号一致
        if (strcmp(ChildNT.name, NT.name) == 0)
            continue;
        // 迭代
        AddChildNode(VecG, GrammarMap, ChildNT);
        // 把子节点的数据放入当前节点
        for (int i : GrammarMap[ChildNT])
        {
            if (GrammarMap[NT].count(i) == 0)
            {
                GrammarMap[NT].insert(i);
                printf("NonTerminal:%s add product:%d\n", NT.name, i);
            }
        }
    }
}

// 寻找指定非终结符 为产生式右部第一个符号的产生式列表
map<GrammarUnit, set<int>> GeneratorGrammarMap(const vector<Grammar> &VecG)
{   
    map<GrammarUnit, set<int>> GrammarMap{};
    // 产生式循环
    for (unsigned pro_id=0; pro_id<VecG.size(); ++pro_id)
    {
        const Grammar &G = VecG[pro_id];
        GrammarUnit NT = G.arr[0];
        map<GrammarUnit, set<int>>::iterator iter = GrammarMap.find(NT);
        if (iter == GrammarMap.end())
        {
            set<int> SetI{(int)pro_id};
            GrammarMap[NT] = SetI;
        }
        else
        {
            iter->second.insert(pro_id);
        }
    }
    // 字典遍历
    for (pair<GrammarUnit, set<int>> iter: GrammarMap)
    {   
        const GrammarUnit KEY = iter.first;
        AddChildNode(VecG, GrammarMap, KEY);
    }
    return GrammarMap;
}

// 判断item是否在 VecItem 中
bool InItemVec(const vector<ItemObj> &VecItem, const ItemObj &Item)
{
    for (ItemObj Obj: VecItem)
    {
        if (Obj.production_id == Item.production_id && Obj.position_id == Item.position_id)
            return true;
    }
    return false;
}

// 计算增广文法
void Closure(vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG)
{
    // 寻找指定非终结符 为产生式右部第一个符号的产生式列表
    map<GrammarUnit, set<int>> GrammarMap = GeneratorGrammarMap(VecG);
    // 项集遍历
    for (unsigned i=1; i<VecVecItem.size(); ++i)
    {   
        vector<ItemObj> &VecItem = VecVecItem[i];
        // 项遍历
        for (unsigned j=0; j<VecItem.size(); ++j)
        {   
            const ItemObj Item = VecItem[j];
            const Grammar &G = VecG[Item.production_id];
            // 检查位置ID是否越界 当前位置是不是终结符
            if (Item.position_id+1 >= G.arr.size() || G.arr[Item.position_id+1].type == UNIT_TYPE_TERMINAL)
                continue;
            GrammarUnit NT = G.arr[Item.position_id+1];
            // 寻找可行产生式
            for (int pro_id: GrammarMap[NT])
            {   
                ItemObj NewItem{pro_id, 0};
                if (!InItemVec(VecItem, NewItem))
                {
                    printf("Collection[%d] add Grammar:%d Position:%d Unit:%s\n", i, Item.production_id, Item.position_id, NT.name);
                    VecItem.push_back(NewItem);
                }
            }
        }
    }
}

// 打印项集
void PrintItemCollections(const vector<vector<ItemObj>>& VecVecItem, const vector<Grammar> &VecG)
{
    unsigned CollectionID{0};
    for (CollectionID=0; CollectionID<VecVecItem.size(); ++CollectionID)
    {
        printf("Collection %u:\n", CollectionID);
        const vector<ItemObj>& VecItem = VecVecItem[CollectionID];
        for (unsigned ItemID=0; ItemID<VecItem.size(); ++ItemID)
        {
            const ItemObj &ItemObj = VecItem[ItemID];
            const Grammar &G = VecG[ItemObj.production_id]; 
            PrintGrammar(G, ItemObj.position_id, &(ItemObj.forward));
        }
    }
}

// 打印分析表
void PrintAnalysisTable(const vector<vector<AnalysisTableUnit>> &LRTable, const vector<GrammarUnit> VecGU)
{   
    // 打印列名
    printf("state ");
    for(const GrammarUnit &GU: VecGU)
    {   
        char s[GRAMMAR_NAME_SIZE]{"      "};
        strncpy(s, GU.name, strlen(GU.name));
        printf(s);
    }
    printf("\n");
    // 打印数据
    for (unsigned i=0; i<LRTable.size(); ++i)
    {   
        const vector<AnalysisTableUnit> &Vec = LRTable[i];
        printf("%02d    ", i);
        for (const AnalysisTableUnit ATU: Vec)
        {
            printf("%c%d    ", ATU.type, ATU.num);
        }
        printf("\n");
    }
}


// 获取除文法开始符号之外的所有文法符号
vector<GrammarUnit> GetAllGrammarUnit(const vector<Grammar> &VecG)
{
    set<GrammarUnit> SetGU{};
    vector<GrammarUnit> VecGU{};
    for (Grammar G: VecG)
    {
        for (unsigned i=1; i<G.arr.size(); ++i)
        {   
            GrammarUnit &GU = G.arr[i];
            if (SetGU.count(GU) == 0)
            {
                SetGU.insert(GU);
                VecGU.push_back(GU);
            }
        }
    }
    return VecGU;
}

// 寻找Item所属的 项集 未找到返回 -1
int FindTargetId(const vector<vector<ItemObj>> &VecVecItem, const ItemObj &Item)
{   
    for (unsigned i=0; i<VecVecItem.size(); ++i)
    {
        if (InItemVec(VecVecItem[i], Item))
            return (int)i;  // 找到
    }
    return -1;  // 未找到
}

// 计算项集转换函数
// map<GrammarUnit, int> 每个项集的转换字典 
// GrammarUnit 终结符/非终结符  int 转换后的项集
void GotoItem(vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG)
{   
    bool AddNewItem{true};
    // 获取文法符号
    vector<GrammarUnit> VecGU = GetAllGrammarUnit(VecG);
    // 
    int count{0};
    while (AddNewItem)
    {   
        AddNewItem = false;
        ++count;
        // 项集循环
        for (unsigned i=0; i<VecVecItem.size(); ++i)
        {
            vector<ItemObj> &VecItem = VecVecItem[i];    // 项集
            for (GrammarUnit GU: VecGU)
            {   
                int TargetId = -1;
                for (ItemObj Item: VecItem)
                {   // 检查下一个符号与当前搜索的符号一致
                    GrammarUnit NextGU = GetNextGrammarUnit(VecG, Item);
                    // if (i == 6)
                    // {
                    //     printf("Next GrammarUnit:%s  Next:%s Collections:%d %d ", GU.name, NextGU.name, i, TargetId);
                    //     PrintItem(Item, VecG);
                    // }
                    if (!Equal(GU, NextGU))
                        continue;
                    // 项 向后移动一位
                    ItemObj NewItem{Item};
                    NewItem.position_id++;
                    if (TargetId < 0)
                    {
                        TargetId = FindTargetId(VecVecItem, NewItem);
                    }
                    else
                    {
                        vector<ItemObj> &TargetVecItem = VecVecItem[TargetId];    // 项集
                        if (!InItemVec(TargetVecItem, NewItem))
                        {
                            TargetVecItem.push_back(NewItem);
                            AddNewItem = true;
                            printf("cycle %d Collections[%d] add", count, TargetId);
                            PrintGrammar(VecG[NewItem.production_id], NewItem.position_id);
                        }
                    }
                }
            }
        }
    }
}

// 生成语法单元字典 用于分析表
vector<GrammarUnit> GeneratorGrammarUnitVec(const vector<Grammar> &VecG)
{   
    GrammarUnit End{UNIT_TYPE_TERMINAL, "$"};
    set<GrammarUnit> SetGU{End};
    for (Grammar G: VecG)
    {
        for (unsigned i=1; i<G.arr.size(); ++i)
        {   
            GrammarUnit &GU = G.arr[i];
            if (SetGU.count(GU) == 0)
            {
                SetGU.insert(GU);
            }
        }
    }
    vector<GrammarUnit> VecGU {};
    // 按照先终结符 后非终结符的顺序 创建字典
    for (GrammarUnit GU: SetGU)
    {   
        if (GU.type == UNIT_TYPE_TERMINAL)
            VecGU.push_back(GU);
    }
    for (GrammarUnit GU: SetGU)
    {   
        if (GU.type == UNIT_TYPE_NONTERMINAL)
            VecGU.push_back(GU);
    }
    return VecGU;
}

// 生成语法单元字典 用于分析表
map<GrammarUnit, int> GeneratorGrammarUnitMap(const vector<Grammar> &VecG)
{   
    GrammarUnit End{UNIT_TYPE_TERMINAL, "$"};
    set<GrammarUnit> SetGU{End};
    for (Grammar G: VecG)
    {
        for (unsigned i=1; i<G.arr.size(); ++i)
        {   
            GrammarUnit &GU = G.arr[i];
            if (SetGU.count(GU) == 0)
            {
                SetGU.insert(GU);
            }
        }
    }
    map<GrammarUnit, int> MapGU {};
    int count = 0;
    // 按照先终结符 后非终结符的顺序 创建字典
    for (GrammarUnit GU: SetGU)
    {   
        if (GU.type == UNIT_TYPE_TERMINAL)
            MapGU[GU] = count++;
    }
    for (GrammarUnit GU: SetGU)
    {   
        if (GU.type == UNIT_TYPE_NONTERMINAL)
            MapGU[GU] = count++;
    }
    return MapGU;
}

// 分析表生成算法
vector<vector<AnalysisTableUnit>> GeneratorAnalysysTable(const vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG)
{   
    // 分析表
    vector<vector<AnalysisTableUnit>> LRTable{};
    //
    AnalysisTableUnit Undefine{AT_UNIT_TYPE_NULL}, Error{AT_UNIT_TYPE_ERROR};
    // 生成语法单元字典 
    vector<GrammarUnit> VecGU = GeneratorGrammarUnitVec(VecG);
    // 项集循环
    for (unsigned i=0; i<VecVecItem.size(); ++i)
    {   
        // 添加一行数据
        vector<AnalysisTableUnit> VecAT{};
        // printf("add line:%d", i);
        LRTable.push_back(VecAT);
        const vector<ItemObj> &VecItem = VecVecItem[i];    // 项集
        // 符号循环
        for (unsigned j=0; j<VecGU.size(); ++j)
        {   
            // 添加一列数据
            AnalysisTableUnit ATU{AT_UNIT_TYPE_NULL, 0};
            LRTable[i].push_back(ATU);
            // 当前符号
            const GrammarUnit &GU = VecGU[j];
            // printf(" symbol:%s id:%d terminal:%d", GU.name, i, GU.type);
            int TargetId = -1;
            for (ItemObj Item: VecItem)
            {   // 检查下一个符号与当前搜索的符号一致
                GrammarUnit NextGU = GetNextGrammarUnit(VecG, Item);
                if (!Equal(GU, NextGU))  // 不一致的情况
                {   // 当前符号为终结符 且 表状态为未定义 且 产生式下一个语法单元为空
                    if (NextGU.type == UNIT_TYPE_NULL && GU.type == UNIT_TYPE_TERMINAL)
                    {
                        if (strcmp(GU.name, "$") == 0 && Item.production_id == 0)
                        {
                            AnalysisTableUnit ATU{AT_UNIT_TYPE_ACCEPT, 0}; // 归约
                            LRTable[i][j] = ATU;                         
                        }
                        else if (LRTable[i][j].type == AT_UNIT_TYPE_NULL)
                        {
                            AnalysisTableUnit ATU{AT_UNIT_TYPE_REDUCE, Item.production_id}; // 归约
                            LRTable[i][j] = ATU;
                        }
                    }
                    continue;
                }
                // 项 向后移动一位
                ItemObj NewItem{Item};
                NewItem.position_id++;
                TargetId = FindTargetId(VecVecItem, NewItem);                           // 跳转
                // printf("symbol:%s %u %u->%d terminal:%d\n", GU.name, j, i, TargetId, GU.type);
                if (TargetId >= 0)
                {
                    if (GU.type == UNIT_TYPE_TERMINAL)
                    {   
                        AnalysisTableUnit ATU{AT_UNIT_TYPE_SHIFT, TargetId};            // 移入
                        LRTable[i][j] = ATU;   
                    }
                    else
                    {   
                        if (LRTable[i][j].type == AT_UNIT_TYPE_NULL)
                        {
                            // printf("set goto:%u %u->%d %s\n", j, i, TargetId, GU.name);
                            // PrintItem(NewItem, VecG);
                            AnalysisTableUnit ATU{AT_UNIT_TYPE_GOTO, TargetId};            // 跳转
                            LRTable[i][j] = ATU; 
                        }  
                    }
                }
                else
                {
                    // if (!InItemVec(TargetVecItem, NewItem))
                    // {
                    //     printf("cycle %d Collections[%d] add", count, TargetId);
                    //     PrintGrammar(VecG[NewItem.production_id], NewItem.position_id);
                    // }
                }
            }
        }
    }
    return LRTable;
}
// 归约函数
// 对栈顶的N个元素 按照文法G进行归约
bool Reduce(stack<GrammarUnit>& StkSym, stack<int> &StkState, const Grammar &G)
{   
    bool Success{false};
    for (unsigned i=G.arr.size(); i>1; --i)
    {   // 栈符号不足 或者 对应符号与文法G不同
        if (StkSym.empty() || !Equal(StkSym.top(), G.arr[i-1]))
            return false;
        StkState.pop();
        StkSym.pop();
    }
    // 栈顶添加文法产生式左部符号
    StkSym.push(G.arr[0]);
    return true;
}

void PrintStack(const stack<int> &StkState, const stack<GrammarUnit> &StkSym, const stack<GrammarUnit> &StkInput, const AnalysisTableUnit &ATU)
{
    static int print_count{0};
    if (++print_count > 30)
    {
        throw(777);
    }
    stack<int> SState = StkState;
    stack<GrammarUnit> SSym = StkSym, SInput = StkInput;
    int count = 0;
    while (!SState.empty())
    {
        printf("%d ", SState.top());
        SState.pop();
        ++count;
    }
    for (; count<10; ++count)
        printf("  ");
    count = 0;
    while (!SSym.empty())
    {
        printf("%s ", SSym.top().name);
        SSym.pop();
        ++count;
    }
    for (; count<10; ++count)
        printf("  ");
    count = 0;
    while (!SInput.empty())
    {
        printf("%s ", SInput.top().name);
        SInput.pop();
        ++count;
    }
    for (; count<10; ++count)
        printf("   ");
    printf("%c%d\n", ATU.type, ATU.num);
}

void AddChild(GrammarUnit* pParent, GrammarUnit* pChild)
{
    if (pParent->child == nullptr)
    {
        pParent->child = pChild;
        return;
    }
    GrammarUnit *pCur{pParent->child};
    while (pCur->brother != nullptr)
    {
        pCur = pCur->brother;
    }
    pCur->brother = pChild;
    pCur->parent = pParent;
}

// 归约根节点最右N个符号 为一个新符号
void InsertRightChild(GrammarUnit* pParent, GrammarUnit* pNew, int N)
{   
    stack<GrammarUnit*> StkChild{};
    if (pParent->child == nullptr)
    {   // 无子节点
        return;
    }
    // 将pParent的子节点加入栈
    GrammarUnit *pCur{pParent->child};
    while (pCur != nullptr)
    {   
        StkChild.push(pCur);
        pCur = pCur->brother;
    }
    if (StkChild.size() < N)
    {
        printf("child num:%lu < %d\n", StkChild.size(), N);
    }
    // 从栈中取出N个节点作为pNew的子节点
    for (int i=0; i<N; ++i)
    {   
        if (i == N-1)
        {
            pNew->child = StkChild.top();
        }
        pCur = StkChild.top();
        pCur->parent = pNew;
        StkChild.pop();
    }
    // 更新pNew为pParent的子节点
    if (StkChild.empty())
    {
        pParent->child = pNew;
    }
    else
    {   
        StkChild.top()->brother = pNew;
    }
}

// 语法分析
GrammarUnit* GrammarAnalysis(const vector<vector<AnalysisTableUnit>> &Table, const vector<Grammar> &VecG, stack<GrammarUnit> &StkInput)
{
    // 获取语法单元数组 
    map<GrammarUnit, int> MapGU = GeneratorGrammarUnitMap(VecG);
    // 状态栈
    stack<int> StkState{};
    StkState.push(0);
    // 符号栈
    stack<GrammarUnit> StkSym{};
    // 分析状态
    bool AnalysisState{true};
    // 上一步是否是归约
    bool IsReduce{false};
    // 语法单元编码 分析表列ID
    int GUID;
    // 状态 分析表行ID
    int State;
    // 创建语法分析树 根节点
    GrammarUnit *Root = CreateGrammarTreeNode("S", false), *pCur{nullptr};
    while(AnalysisState)
    {   
        // 获取当前状态
        State = StkState.top();
        // 获取当前符号ID
        if (IsReduce)
            GUID = MapGU[StkSym.top()];
        else 
            GUID = MapGU[StkInput.top()];
        IsReduce = false;
        // 查表
        const AnalysisTableUnit ATU = Table[State][GUID];
        // 输出栈
        PrintStack(StkState, StkSym, StkInput, ATU);
        // 按照分析表的内容执行动作
        switch (ATU.type)
        {
        case AT_UNIT_TYPE_NULL:         // 分析表类型 未定义
            throw("error analysis type 0\n");
            break;  
        case AT_UNIT_TYPE_ACCEPT:       // 分析表类型 接受
            AnalysisState = false;
            break;
        case AT_UNIT_TYPE_REDUCE:       // 分析表类型 归约
            for (unsigned i=VecG[ATU.num].arr.size(); i>1; --i)
            {   // 栈符号不足 或者 对应符号与文法G不同
                if (StkSym.empty() || !Equal(StkSym.top(), VecG[ATU.num].arr[i-1]))
                {
                    printf("error in reduce\n");
                    PrintGrammar(VecG[ATU.num]);
                    throw(7);
                }
                StkState.pop();         // 移出状态
                StkSym.pop();           // 移出符号
            }
            pCur = CreateGrammarTreeNode(VecG[ATU.num].arr[0].name, false);
            InsertRightChild(Root, pCur, VecG[ATU.num].arr.size()-1);
            // 栈顶添加文法产生式左部符号
            StkSym.push(VecG[ATU.num].arr[0]);
            IsReduce = true;
            break;
        case AT_UNIT_TYPE_SHIFT:        // 分析表类型 移入
            if (StkInput.empty())
                printf("error empty Input\n");
            StkSym.push(StkInput.top());
            pCur = CreateGrammarTreeNode(StkInput.top().name, true);
            AddChild(Root, pCur);       // 将新节点放到 根节点下
            StkInput.pop();
            StkState.push(ATU.num);     // 状态更新
            break;
        case AT_UNIT_TYPE_GOTO:         // 分析表类型 跳转
            StkState.push(ATU.num);     // 状态更新
            break;
        case AT_UNIT_TYPE_ERROR:        // 分析表类型 错误
            break;
        default:
            break;
        }
    }
    return Root;
}

// 遍历 遍历顺序 先左子节点 再 本节点 再其余子节点
void TraverseTree(const GrammarUnit* pCur, int Level, vector<MarkNode> &VecNode)
{   
    static int count{0};
    if (++count > 30)
        throw(30);
    const GrammarUnit* pChild = pCur->child;
    // printf("level%d %s \n", Level, pCur->name);
    if (pChild != nullptr)
    {
        TraverseTree(pChild, Level+1, VecNode);
        pChild = pChild->brother;
    }
    // if (pCur->type == UNIT_TYPE_TERMINAL)
    if (true)
    {
        MarkNode Node{pCur, Level};
        VecNode.push_back(Node);
    }
    while (pChild != nullptr)
    {
        TraverseTree(pChild, Level+1, VecNode);
        pChild = pChild->brother;
    }
}

void PrintGrammarAnalysisTree(const GrammarUnit* pRoot)
{   
    printf("start ...\n");
    vector<MarkNode> VecNode{};
    TraverseTree(pRoot, 0, VecNode);
    printf("traverse over\n");
    int count = 0, Level = 0;
    while (count < VecNode.size())
    {   
        for (MarkNode Node: VecNode)
        {
            if (Node.Level == Level)
            {
                printf("%.2s", Node.pNode->name);
                ++count;
            }
            else
                printf("   ");
        }
        printf("\n");
        ++Level;
    }
}
// 根据指定位置获取产生式右部的文法符号
GrammarUnit GetGrammarUnitByPosition(const ItemObj &Item, int position, const vector<Grammar> &VecAG)
{
    int size = (int)VecAG[Item.production_id].arr.size();
    if (position+1 >= size || position<0)
        return CreateGrammarUnit("", true);
    else
        return VecAG[Item.production_id].arr[Item.position_id+1];
}

// 寻找文法符号Beta的首元素集合
map<GrammarUnit, set<GrammarUnit>> First(const vector<Grammar> &VecAG)
{   
    map<GrammarUnit, set<GrammarUnit>> FirstTerminalMap {};
    // 获取文法符号
    vector<GrammarUnit> VecGU = GetAllGrammarUnit(VecAG);
    for (GrammarUnit GU: VecGU)
    {   
        set<GrammarUnit> SetGU{};
        if (GU.type == UNIT_TYPE_TERMINAL)
            SetGU.insert(GU);
        FirstTerminalMap[GU] = SetGU;
    }
    bool AddNew{true};
    GrammarUnit NullGU = GetNullGrammarUnit();
    while (AddNew)
    {   
        // 重置标记
        AddNew = false;
        // 遍历所有产生式
        for (const Grammar &G: VecAG)
        {   
            GrammarUnit LeftGU = G.arr[0];
            for (unsigned i=1; i< G.arr.size(); ++i)
            {   
                GrammarUnit GU = G.arr[i];
                for (GrammarUnit FirstGU: FirstTerminalMap[GU])
                {
                    if (FirstTerminalMap[LeftGU].count(FirstGU) == 0)
                    {   
                        FirstTerminalMap[LeftGU].insert(FirstGU);
                        AddNew = true;
                        printf("First(%s) add %s\n", LeftGU.name, FirstGU.name);
                    }
                }
                // 如果当前符号的产生式无法推导除空符号 就跳出循环
                if (FirstTerminalMap[GU].count(GU) == 0)
                    break;
            }   
            
        }
    }
    return FirstTerminalMap;
}

// 按照 A-> alpha @ B beta, a的模式展开公式
const set<GrammarUnit> FindForwardTerminal(ItemObj Item, const vector<Grammar> &VecAG, map<GrammarUnit, set<GrammarUnit>> &FirstTerminalMap)
{   
    // 获取@符号左侧的文法符号
    // GrammarUnit alpha = GetGrammarUnitByPosition(Item, Item.position_id - 1);
    // 获取@符号右侧的第一个文法符号
    // GrammarUnit B = GetGrammarUnitByPosition(Item, Item.position_id);
    // 获取@符号右侧的第二个文法符号
    GrammarUnit beta = GetGrammarUnitByPosition(Item, Item.position_id + 1, VecAG);
    // 空文法符号
    GrammarUnit NullGU = CreateGrammarUnit("", true);
    // 向前看符号集合
    set<GrammarUnit> SetGU;
    if (Equal(beta, NullGU))
    {   // Beta为空
        SetGU.insert(Item.forward);
    }
    else // Beta不为空
    {
        // 寻找Beta的首终结符集合
        SetGU = FirstTerminalMap[beta];
    }
    return SetGU;
}

// 判断item是否在 VecItem 中
bool InItemVecLR1(const vector<ItemObj> &VecItem, const ItemObj &Item)
{
    for (ItemObj Obj: VecItem)
    {
        if (Obj.production_id == Item.production_id && Obj.position_id == Item.position_id && Equal(Item.forward, Obj.forward))
            return true;
    }
    return false;
}


// 寻找Item所属的 项集 未找到返回 -1
int FindTargetIdLR1(const vector<vector<ItemObj>> &VecVecItem, const ItemObj &Item)
{   
    for (unsigned i=0; i<VecVecItem.size(); ++i)
    {
        if (InItemVecLR1(VecVecItem[i], Item))
            return (int)i;  // 找到
    }
    return -1;  // 未找到
}

// 计算项集
vector<vector<ItemObj>> ItemGeneratorLR1(const vector<Grammar> &VecAG, map<GrammarUnit, set<GrammarUnit>> &FirstTerminalMap)
{
  
    int pro_num{(int)VecAG.size()};
    int MaxSize{0};
    // 产生式循环
    ItemObj new_item{0, 0, CreateGrammarUnit("$", true)};
    vector<vector<ItemObj>> VecVecItem {{new_item}};
    unsigned read_num{0};
    int collecton_id{0};
    // 获取文法符号
    vector<GrammarUnit> VecGU = GetAllGrammarUnit(VecAG);
    for (GrammarUnit GU: VecGU)
    {
        printf("%s ", GU.name);
    }
    printf("\n");
    int count{0}, collection_count{1};
    bool AddNewItem {true};
    // printf("GU number:%lu VecVecItem:%lu\n", VecGU.size(), VecVecItem.size());
    while (AddNewItem)
    {   
        AddNewItem = false;
        // 项集循环
        for (unsigned i=0; i<VecVecItem.size(); ++i)
        {
            printf("collection_id:%u\n", i);
            // 遍历下一个传入符号
            for (GrammarUnit GU: VecGU)
            {   
                int TargetId = -1;
                // 遍历项
                for (unsigned j=0; j<VecVecItem[i].size(); ++j)
                {   
                    ItemObj Item = VecVecItem[i][j];
                    // 检查下一个符号与当前搜索的符号一致
                    // printf("before equal pos:%d \n", Item.position_id);
                    GrammarUnit NextGU = GetNextGrammarUnit(VecAG, Item);
                    // PrintItem(Item, VecAG);
                    // 下一个符号和当前符号不一样
                    if (!Equal(GU, NextGU))
                        continue;
                    // PrintItem(Item, VecAG);
                    // 项 向后移动一位
                    ItemObj NewItem{Item};
                    NewItem.position_id++;
                    if (TargetId < 0)
                    {
                        TargetId = FindTargetIdLR1(VecVecItem, NewItem);
                        // 未知项集  创建新的项集
                        if (TargetId < 0)
                        {   
                            TargetId = VecVecItem.size();
                            vector<ItemObj> NewVecItem{NewItem};
                            printf("create new collection:%d\n", TargetId);
                            printf("cycle %d Collections[%d] add ", count, TargetId);
                            PrintItem(NewItem, VecAG);
                            VecVecItem.push_back(NewVecItem);
                            AddNewItem = true;
                        }
                    }
                    else
                    {   // 已找到项集的情况
                        vector<ItemObj> &TargetVecItem = VecVecItem[TargetId];    // 项集
                        if (!InItemVecLR1(TargetVecItem, NewItem))                // 新项不在原项集里
                        {
                            TargetVecItem.push_back(NewItem);
                            AddNewItem = true;
                            printf("cycle %d Collections[%d] add ", count, TargetId);
                            PrintItem(NewItem, VecAG);
                        }
                    }
                    // 当前符号可以展开的情况
                    if (NextGU.type == UNIT_TYPE_NONTERMINAL)
                    {
                        set<GrammarUnit> SetGU = FindForwardTerminal(Item, VecAG, FirstTerminalMap);
                        // 遍历产生式 寻找可以展开当前非终结符 GU的产生式
                        for (unsigned pro_id=0; pro_id<VecAG.size(); ++pro_id)
                        {   
                            const Grammar& G = VecAG[pro_id];
                            if (Equal(G.arr[0], GU))
                            {
                                for (GrammarUnit EndGU: SetGU)
                                {   
                                    ItemObj NewItem{pro_id, 0, EndGU};
                                    if (!InItemVecLR1(VecVecItem[i], NewItem)) 
                                    {
                                        VecVecItem[i].push_back(NewItem);
                                        printf("cycle %d Collections[%d] add ", count, i);
                                        PrintItem(NewItem, VecAG);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return VecVecItem;
}


// 分析表生成算法 LR1
vector<vector<AnalysisTableUnit>> GeneratorAnalysysTableLR1(const vector<vector<ItemObj>> &VecVecItem, const vector<Grammar> &VecG)
{   
    // 分析表
    vector<vector<AnalysisTableUnit>> LRTable{};
    //
    AnalysisTableUnit Undefine{AT_UNIT_TYPE_NULL}, Error{AT_UNIT_TYPE_ERROR};
    // 生成语法单元字典 
    vector<GrammarUnit> VecGU = GeneratorGrammarUnitVec(VecG);
    // 项集循环
    for (unsigned i=0; i<VecVecItem.size(); ++i)
    {   
        // 添加一行数据
        vector<AnalysisTableUnit> VecAT{};
        // printf("add line:%d", i);
        LRTable.push_back(VecAT);
        const vector<ItemObj> &VecItem = VecVecItem[i];    // 项集
        // 符号循环
        for (unsigned j=0; j<VecGU.size(); ++j)
        {   
            // 添加一列数据
            AnalysisTableUnit ATU{AT_UNIT_TYPE_NULL, 0};
            LRTable[i].push_back(ATU);
            // 当前符号
            const GrammarUnit &GU = VecGU[j];
            // printf(" symbol:%s id:%d terminal:%d", GU.name, i, GU.type);
            int TargetId = -1;
            for (ItemObj Item: VecItem)
            {   // 检查下一个符号与当前搜索的符号一致
                GrammarUnit NextGU = GetNextGrammarUnit(VecG, Item);
                if (!Equal(GU, NextGU))  // 不一致的情况
                {   // 当前符号为终结符 且 表状态为未定义 且 产生式下一个语法单元为空
                    if (NextGU.type == UNIT_TYPE_NULL && GU.type == UNIT_TYPE_TERMINAL)
                    {   // 下个符号与向前看符号一致
                        if (Equal(GU, Item.forward))
                        {   
                            if (Item.production_id == 0)
                            {
                                AnalysisTableUnit ATU{AT_UNIT_TYPE_ACCEPT, 0}; // 接受
                                LRTable[i][j] = ATU;                         
                            }
                            else
                            {
                                AnalysisTableUnit ATU{AT_UNIT_TYPE_REDUCE, Item.production_id}; // 归约
                                LRTable[i][j] = ATU;
                            }                     
                        }
                    }
                    continue;
                }
                // 项 向后移动一位
                ItemObj NewItem{Item};
                NewItem.position_id++;
                TargetId = FindTargetIdLR1(VecVecItem, NewItem);                           // 跳转
                // printf("symbol:%s %u %u->%d terminal:%d\n", GU.name, j, i, TargetId, GU.type);
                if (TargetId >= 0)
                {
                    if (GU.type == UNIT_TYPE_TERMINAL)
                    {   
                        AnalysisTableUnit ATU{AT_UNIT_TYPE_SHIFT, TargetId};            // 移入
                        LRTable[i][j] = ATU;   
                    }
                    else
                    {   
                        if (LRTable[i][j].type == AT_UNIT_TYPE_NULL)
                        {
                            // printf("set goto:%u %u->%d %s\n", j, i, TargetId, GU.name);
                            // PrintItem(NewItem, VecG);
                            AnalysisTableUnit ATU{AT_UNIT_TYPE_GOTO, TargetId};            // 跳转
                            LRTable[i][j] = ATU; 
                        }  
                    }
                }
                else
                {
                    // if (!InItemVec(TargetVecItem, NewItem))
                    // {
                    //     printf("cycle %d Collections[%d] add", count, TargetId);
                    //     PrintGrammar(VecG[NewItem.production_id], NewItem.position_id);
                    // }
                }
            }
        }
    }
    return LRTable;
}