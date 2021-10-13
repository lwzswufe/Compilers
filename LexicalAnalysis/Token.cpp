#include "Token.h"
#include "LexicalAnalysis.h"

static char TEMP_STRING[1024];

Token::Token(const char* name, const char* type)
{   
    char s[256];
    strncpy(_name, name, MAX_WORD_LENGTH);
    strncpy(_type, type, MAX_TYPE_LENGTH);
    GetTokenPosition(_line_id, _char_id);
}

const char* Token::GetType()
{   
    return _type;
}

const char* Token::GetStr()
{
    sprintf(TEMP_STRING, "name:%s__type:%s line:%d char:%d", _name, _type, _line_id, _char_id);
    return TEMP_STRING;
}
