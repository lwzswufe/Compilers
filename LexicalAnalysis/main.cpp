#include "LexicalAnalysis.h"

/*
g++ Token.h LexicalAnalysis.h Token.cpp LexicalAnalysis.cpp main.cpp -std=c++11 -o LexicalAnalysis.out
*/

int main()
{   
    char fn[256]{"helloworld.txt"};
    ReadFile(fn);
    OutPutToken("a.txt");
    return 0;
};
