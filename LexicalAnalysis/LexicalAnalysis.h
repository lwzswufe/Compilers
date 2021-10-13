#include <string.h>
#define MAX_WORD_LENGTH 256
#define STATE_DEFAULT 0 
#define STATE_COMMENT 1 


enum LexicalState
{
    StateDefault = 0,   // 默认状态
    StateComment,       // 注释状态
    StateString,
};

// 读取一个文件
void ReadFile(const char* filename);

// 输出词法单元
void OutPutToken(const char* filename);

// 获取当前词法所在位置 行号 字符号
void GetTokenPosition(int &line_id, int &char_id);
