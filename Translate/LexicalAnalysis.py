from Symbol import *


'''
词法分析
'''


# 符号
SYMBOL_SET = {"{", "}", "(", ")", "[", "]", "+", "-", "*", "/", "%", "=", "==", "!=", ",", ";", '"', "//", "<", ">", "<=", ">=", "&&", "||"}
# 符号字符列表
SYMBOL_CHAR_SET = set()
for symbol in SYMBOL_SET:
    for c in symbol:
        if c not in SYMBOL_CHAR_SET:
            SYMBOL_CHAR_SET.add(c)
# 当前行号
LINE_ID = 0
# 起始字符坐标
BEGIN_ID = 0
# 终止字符坐标
FORWARD_ID = 0


# 词法错误
class LexicalError(RuntimeError):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)


# 识别单词 包括变量/函数/保留字
def IdentifyWord(line: str) -> Token:
    global BEGIN_ID, FORWARD_ID
    FORWARD_ID += 1
    while FORWARD_ID < len(line):
        c = line[FORWARD_ID]
        FORWARD_ID += 1
        if c.isalnum() or c == "_":
            pass
        else:
            break
    FORWARD_ID -= 1
    word = line[BEGIN_ID: FORWARD_ID]
    token = Variable(word)
    token.SetPosition(LINE_ID, BEGIN_ID)
    # print("word:{} {}~{}".format(word, BEGIN_ID, FORWARD_ID))
    BEGIN_ID = FORWARD_ID
    return token


# 识别数字 整数 浮点数
def IdentifyNumeric(line: str) -> Token:
    global BEGIN_ID, FORWARD_ID
    FORWARD_ID += 1
    while FORWARD_ID < len(line):
        c = line[FORWARD_ID]
        FORWARD_ID += 1
        if c.isnumeric() or c == ".":
            pass
        else:
            break
    FORWARD_ID -= 1
    word = line[BEGIN_ID: FORWARD_ID]
    token = Literal(word)
    token.SetPosition(LINE_ID, BEGIN_ID)
    BEGIN_ID = FORWARD_ID
    return token


# 识别注释
def IdentifyComments(line: str) -> Token:
    global BEGIN_ID, FORWARD_ID, LINE_ID
    # 注释持续到行尾
    word = line[BEGIN_ID:BEGIN_ID+2]
    comments = line[BEGIN_ID+2:]
    FORWARD_ID = len(line)
    BEGIN_ID = FORWARD_ID
    token = Operator(name=word)
    token.SetPosition(LINE_ID, BEGIN_ID)
    return token


# 识别字符串字面量
def IdentifyString(line: str) -> Token:
    global BEGIN_ID, FORWARD_ID, LINE_ID
    FORWARD_ID += 1
    while FORWARD_ID < len(line):
        c = line[FORWARD_ID]
        FORWARD_ID += 1
        if c == '"':
            FORWARD_ID += 1
            # print(line[BEGIN_ID: FORWARD_ID - 1])
            break
    FORWARD_ID -= 1
    word = line[BEGIN_ID: FORWARD_ID]
    # 检查尾部符号是不是"
    if word[-1] != '"':
        raise LexicalError('Line:{} {}~{} {} is not end with "'.format(LINE_ID, BEGIN_ID, FORWARD_ID-1, word))
    token = Literal(word)
    token.SetPosition(LINE_ID, BEGIN_ID)
    BEGIN_ID = FORWARD_ID
    return token


# 识别符号
def IdentifySymbol(line: str) -> Token:
    global BEGIN_ID, FORWARD_ID, LINE_ID
    # 优先识别双字符运算符
    FORWARD_ID += 2
    word = line[BEGIN_ID: FORWARD_ID]
    # 不是双字符运算符的情况
    if word not in SYMBOL_SET:
        # 识别单字符运算符
        FORWARD_ID -= 1
        word = line[BEGIN_ID: FORWARD_ID]
        # 非单字符运算符的情况
        if word not in SYMBOL_SET:
            raise LexicalError("Line:{} {}~{} {} is not a symbol".format(LINE_ID, BEGIN_ID, FORWARD_ID-1, word))
    # print("symbol:{} {}~{}".format(word, BEGIN_ID, FORWARD_ID))
    if word == "//":
        token = IdentifyComments(line)
    elif word == '"':
        token = IdentifyString(line)
    else:
        token = Operator(word)
        token.SetPosition(LINE_ID, BEGIN_ID)
        BEGIN_ID = FORWARD_ID
    return token


def LexicalRecognize(line: str) -> List[Token]:
    '''
    识别一行代码里的语义单元
    '''
    global BEGIN_ID, FORWARD_ID
    token_list: List[Token] = []
    BEGIN_ID = 0
    FORWARD_ID = 0
    while BEGIN_ID < len(line):
        c = line[BEGIN_ID]
        # 识别单词
        if c.isalpha() or c == "_":
            token = IdentifyWord(line)
            token_list.append(token)
        # 识别数字
        elif c.isnumeric():
            token = IdentifyNumeric(line)
            token_list.append(token)
        # 识别符号
        elif c in SYMBOL_CHAR_SET:
            token = IdentifySymbol(line)
            token_list.append(token)
        # 无意义字符
        else:
            BEGIN_ID += 1
            FORWARD_ID = BEGIN_ID
    return token_list


def LexicalAnalysis(filename: str) -> List[Token]:
    '''
    词法分析主程序
    '''
    global LINE_ID
    LINE_ID = 0
    token_list = []
    fw = open("Lexical.txt", "w")
    with open(filename, "r") as f:
        line = f.readline()
        while len(line) > 0:
            r_list = LexicalRecognize(line)
            # 输出词法分析器结果
            if len(r_list) > 0:
                for token in r_list:
                    fw.write(token.GetName())
                    fw.write(", ")
                fw.write("\n")
            token_list += r_list
            line = f.readline()
            LINE_ID += 1
    print(">>>> LexicalAnalysis Over <<<<")
    return token_list


if __name__ == "__main__":
    LexicalAnalysis("test.tb")
