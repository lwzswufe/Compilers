#include <string.h>
#include <stdio.h>
#define MAX_WORD_LENGTH 256
#define MAX_TYPE_LENGTH 32
// 词法单元类 
struct Token
{
public:
    Token(const char* name, const char* type);
    const char* GetType();
    const char* GetStr();
protected:
    char _type[MAX_TYPE_LENGTH];
    char _name[MAX_WORD_LENGTH];
    int _line_id{0};    // 词法单元所在行号
    int _char_id{0};    // 词法单元首元素在该行的中的位置
}; 

// 关键字类型
struct KeyWord:public Token
{
public:
    KeyWord(const char* name): Token(name, "KeyWord"){};
};

// 符号类型
struct Symbol: public Token
{
    Symbol(const char* name): Token(name, "Symbol"){};
};

// 变量名称
struct Variate: public Token
{
    Variate(const char* name): Token(name, "Variate"){};
};

// 字面量
struct Literal: public Token
{
    Literal(const char* name): Token(name, "Literal"){};
};


