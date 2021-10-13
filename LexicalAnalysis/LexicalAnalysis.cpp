#include <cstdio>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <set>
using std::set;
#include "Token.h"
#include "LexicalAnalysis.h"
/*
词法分析器
读取源代码 识别词法单元
*/

static int BEGIN {0};
static int FORWARD {0};
// 行号 表示第几行
static int LINE_ID {0};
// 当前行起始位置
static int LINE_START_ID;
static const int BUFFER_SIZE{1024};
static const char* CONTEXT{nullptr};
static set<string> KEYWORD_SET{"if", "else", "while", "class", "struct", "int", "long", "float", 
                               "true", "false", "bool", "double", "char", "const", "static"};
static vector<Token*> VEC_TOKEN {};

// 获取下一个字符
char NextChar()
{   
    char c = CONTEXT[FORWARD++];
    if (c == '\n')
    {   
        printf("find new line %d\n", FORWARD);
        ++LINE_ID;
        LINE_START_ID = FORWARD;
    }
    return c; 
}


// 获取当前词法所在位置 行号 字符号
void GetTokenPosition(int &line_id, int &char_id)
{
    line_id = LINE_ID;
    char_id = BEGIN - LINE_START_ID;
}

// 识别单行注释
void IdentifySingleLineComment()
{
    char c = NextChar();
    while (c != '\n' && c > 0)
    {
        c = NextChar();
    }
}

// 识别小数部分
void IdentifyFloat(char c)
{   
    while ('0' <= c && c <= '9')
    {
        c = NextChar();
    }
    if (c == '.')
    {
        // 错误警告 两个小数点
    }
    --FORWARD;
    // 识别完毕 提取字符
    char s[MAX_WORD_LENGTH];
    strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
    s[FORWARD - BEGIN] = 0;
    Token* pToken = new Literal(s);
    VEC_TOKEN.push_back(pToken);
}

// 识别整数
void IdentifyNumber(char c)
{   
    while ('0' <= c && c <= '9')
    {
        c = NextChar();
    }
    if (c == '.')
    {   
        c = NextChar();
        IdentifyFloat(c);
    }
    else
    {   
        --FORWARD;
        // 识别完毕 提取字符
        char s[MAX_WORD_LENGTH];
        strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
        s[FORWARD - BEGIN] = 0;
        Token* pToken = new Literal(s);
        VEC_TOKEN.push_back(pToken);
    }
}

// 判断一个字符是否属于能组成单词的字符
bool IsWord(char c)
{   
    bool b{false};
    switch (c)
    {
        case '0' ... '9':
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            b = true; break;
        default:
            break;
    }
    return b;
}

// 识别单词(含数字 下划线)
void IdentifyVariate(char c)
{
    while (IsWord(c))
    {
        c = NextChar();
    }
    --FORWARD;
    // 识别完毕 提取字符
    char s[MAX_WORD_LENGTH];
    strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
    s[FORWARD - BEGIN] = 0;
    Token* pToken{nullptr};
    if (KEYWORD_SET.count(s) == 0)
    {
        pToken = new Variate(s);
    }
    else
    {
        pToken = new KeyWord(s);
    }
    VEC_TOKEN.push_back(pToken);
}

// 识别字符串字面量
void IdentifyString(char c, char last_c)
{
    char last{0};
    while (c > 0 && ( c != last_c || last == '\\'))
    {   
        last = c;
        c = NextChar();
    }
    if (c != last_c)
    {
        // 终止符号错误
    }
    // 识别完毕 提取字符
    char s[MAX_WORD_LENGTH];
    strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
    s[FORWARD - BEGIN] = 0;
    Token* pToken{nullptr};
    pToken = new Literal(s);
    VEC_TOKEN.push_back(pToken);
}

// 识别单一字符 符号 圆括号 方括号 #号
void IndentifySingleSymbol(char c)
{   
    char s[MAX_WORD_LENGTH];
    s[0] = c;
    s[1] = 0;
    Token* pToken = new Symbol(s);
    VEC_TOKEN.push_back(pToken);
}

// 识别等号 单一符号 双相同符号  单一符号加等号 
void IdentifySingleDoubleEqual(char c, char last_c)
{   
    Token* pToken{nullptr};
    char s[MAX_WORD_LENGTH];
    if (c == last_c || c == '=')
    {
        switch (c)
        {
        case '/':
            // 进入注释模式
            printf("comment start\n");
            IdentifySingleLineComment(); break;
        case '*':
            // 报错 ** 无意义
            break;
        default: 
            strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
            s[FORWARD - BEGIN] = 0;
            pToken = new Symbol(s);
        }        
    }
    else
    {
        --FORWARD;
        strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
        s[FORWARD - BEGIN] = 0;
        pToken = new Symbol(s);
    }
    if (pToken != nullptr)
        VEC_TOKEN.push_back(pToken);
}

// 识别等号 单一符号 双相同符号
void IdentifySingleDouble(char c, char last_c)
{
    Token* pToken{nullptr};
    char s[MAX_WORD_LENGTH];
    if (c == last_c)
    {
        strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
        s[FORWARD - BEGIN] = 0;
        pToken = new Symbol(s);
    }
    else
    {
        --FORWARD;
        strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
        s[FORWARD - BEGIN] = 0;
        pToken = new Symbol(s);
    }
    VEC_TOKEN.push_back(pToken);
}

// 识别 单一符号加等号 
void IdentifyEqual(char c, char last_c)
{   
    Token* pToken{nullptr};
    char s[MAX_WORD_LENGTH];
    if (c == '=')
    {
        
        strncpy(s, CONTEXT+BEGIN, FORWARD - BEGIN);
        s[FORWARD - BEGIN] = 0;
        pToken = new Symbol(s); 
        VEC_TOKEN.push_back(pToken);
    }
}

void Analysis(const char* context)
{   
    char c{0}, last_c{0};
    CONTEXT = context;
    if (CONTEXT == nullptr)
        return;
    else
        printf("%s", context);
    c = NextChar();
    while (c > 0 && FORWARD < BUFFER_SIZE - 128)
    {   
        // printf("%c", CONTEXT[FORWARD]);
        switch (c)
        {
            case EOF:
                // 加载新缓冲区
                break;
            case '/':
            case '*':
            case '>':
            case '<':
            case '+':
            case '-':
            case '=':
                last_c = c;
                c = NextChar();
                IdentifySingleDoubleEqual(c, last_c); break;
            case '{':
            case '}':
            case '[':
            case ']':
            case '#':
            case '.':
            case ',':
            case ':':
            case '%':
            case '~':
            case '^':
                IndentifySingleSymbol(c); break;
            case '&':
            case '|':
                last_c = c;
                c = NextChar();
                IdentifySingleDouble(c, last_c); break;
                break;
            case '!':
                last_c = c;
                c = NextChar();
                IdentifyEqual(c, last_c);
                break;
            case '"':
            case '\'':
                last_c = c;
                c = NextChar();
                IdentifyString(c, last_c); break;
            case '\\':
                // 报错 转义字符不能单独使用
                break;
            case '0' ... '9':
                c = NextChar();
                IdentifyNumber(c); break;
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '_':
                c = NextChar();
                IdentifyVariate(c); break;
            default:
                // pass
                break;
        }
        BEGIN = FORWARD;
        c = NextChar();
    }
    printf("\n>>>>>>>>>>scan over<<<<<<<<<<\n");
};

// 读取一个文件
void ReadFile(const char* filename)
{
    FILE * pFile;
    char buffer [BUFFER_SIZE];

    pFile = fopen (filename , "r");
    if (pFile == NULL) 
    {
        perror ("Error opening file");   // 输出异常
    }
    else
    {
        while ( ! feof (pFile) )
        {
            int result = fread(buffer, 1, BUFFER_SIZE, pFile);
            printf("read %d char from file final char is:", result);
            for (int i=-3; i<=2; ++i)
            {
                printf("pos[%d] = %d\n", i, buffer[result+i]);
            }
            if ( result == 0)
                break;
            else
                Analysis(buffer); 
        }
        fclose (pFile);
    }

}

// 输出词法单元
void OutPutToken(const char* filename)
{   
    int count{0};
    for( Token* pToken: VEC_TOKEN)
    {
        printf("Token:%d %s\n", ++count, pToken->GetStr());
    }
    printf("Total %d Token\n", count);
}