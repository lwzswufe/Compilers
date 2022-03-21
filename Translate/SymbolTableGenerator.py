# author='lwz'
# coding:utf-8


'''
读取"TBstrategy.h"与"TBfunction.h"里的函数名 类变量名 将其作为keywords
'''
from Symbol import *


GLOBAL_DICT = {}


def GetCppVarFromString(string: str) -> Token:
    '''
    从一行变量定义中识别变量
    '''
    if len(string) == 0:
        return None
    string = string.replace("\t", " ")
    type_dict = {"const": False, "static": False, "virtual": False, "reference": False, "pointer":False}
    # 此处只识别TB类型变量
    # 因为TB序列变量类型Series<T> 中 T只能为{Integer,Numeric,String,Bool}
    # 所以函数类型声明中一旦出现* &即可证明声明的变量是 引用/指针类型
    # 识别引用类型
    if "&" in string:
        type_dict["reference"] = True
        string = string.replace("&", " ")
    # 识别指针类型
    if "*" in string:
        type_dict["pointer"] = True
        string = string.replace("*", " ")
    words = string.split(" ")
    temp = []
    for word in words:
        if len(word) == 0:
            pass
        elif word in type_dict.keys():
            type_dict[word] = True
        elif word in ("return", "}"):
            return None
        elif len(word) > 2 and word[0] == "{" and word[-1] == "}":
            pass
        else:
            if word.find(">") > 0 and word[-1] != ">":
                idx = word.find(">")
                temp.append(word[:idx+1])
                temp.append(word[idx+1:])
            else:
                temp.append(word)
    if len(temp) != 2:
        print("error in get var type and name:{}".format(string))
        return None
    _type, name = temp
    if _type not in ("void", "Integer", "Bool", "String", "Numeric") and _type.find("Series") < 0:
        return None
    var = CreateVar(type_str=_type, name=name, static=type_dict["static"], const=type_dict["const"],
                    virtual=type_dict["virtual"], reference=type_dict["reference"], 
                    pointer=type_dict["pointer"])
    return var


def GetCppFunFromString(string: str) -> Token:
    '''
    从一行函数定义代码中 识别函数
    '''
    idx_st = string.find("(")
    idx_ed = string.find(")")
    # 无法定位()内容
    if idx_st < 0 or idx_ed < 0:
        return None
    # 识别返回变量
    string_return = string[:idx_st]
    return_var = GetCppVarFromString(string_return)
    if return_var is None:
        print("error in get return type:{}".format(string))
        return None
    # 逐一识别参数
    string_param = string[idx_st+1:idx_ed]
    param_list = []
    words = string_param.split(",")
    for word in words:
        var = GetCppVarFromString(word)
        if var is not None:
            param_list.append(var)
    # 生成函数对象
    token = Function(name=return_var.name)
    token.SetParamList(param_list)
    token.SetReturnType(return_var)
    return token


def GetKeywordFromLine(line: str) -> Token:
    '''
    初步识别一行代码
    '''
    # 剔除分号后的内容
    idx = line.find(";")
    if idx < 0:
        return None
    else:
        line = line[:idx]
    idx = line.find("//")
    # 剔除注释
    if idx >= 0:
        line = line[:idx]
    idx = line.find("(")
    # 识别变量
    if idx < 0:
        var = GetCppVarFromString(line)
        return var
    # 识别函数
    else:
        fun = GetCppFunFromString(line)
        return fun


def GetKeywordFromFile(filename: str) -> None:
    '''
    从文件里获取函数名，变量名关键字
    识别函数时会忽略非TB类型参数
    '''
    global GLOBAL_DICT
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()
    # 逐行识别关键字
    for line in lines:
        try:
            keyword = GetKeywordFromLine(line)
        except Exception as err:
            print(err)
            print(line)
            return
        if keyword is None:
            pass
        elif isinstance(keyword, Variable):
            # print("add Var:{}".format(keyword.name))
            GLOBAL_DICT[keyword.name.lower()] = keyword
        elif isinstance(keyword, Function):
            if keyword.name.lower() not in GLOBAL_DICT.keys():
                # print("add Fun:{}".format(keyword.name))
                GLOBAL_DICT[keyword.name.lower()] = keyword
        else:
            pass


def SymbolTableGenerator():
    global GLOBAL_DICT
    # 读取全局函数
    GetKeywordFromFile("./Translate/TBfunction.h")
    # 读取类变量 类函数
    GetKeywordFromFile("./Translate/TBstrategy.h")
    # 增加TB行情变量别名
    for word in ["open", "high", "low", "close"]:
        if word in GLOBAL_DICT.keys():
            GLOBAL_DICT[word[0].lower()] = GLOBAL_DICT[word]
    # 添加模板函数
    token = Template("", _type=VariableType(BaseType.TEMPLATE))
    fun_max_token = Function(name="Max")
    fun_max_token.SetParamList([token, token])
    fun_max_token.SetReturnType(token)
    GLOBAL_DICT["max"] = fun_max_token
    fun_min_token = Function(name="Min")
    fun_min_token.SetParamList([token, token])
    fun_min_token.SetReturnType(token)
    GLOBAL_DICT["min"] = fun_min_token
    fun_text_token = Function(name="Text")
    fun_text_token.SetParamList([token])
    str_token = String("")
    fun_min_token.SetReturnType(str_token)
    GLOBAL_DICT["min"] = fun_min_token
    # syntax_dict = {"and": "&& ", "or": "||", "commentary": "//Commentary"}
    # 保留字字典 and or 映射为比较运算符
    reversed_dict = {"if": Reversed("if"), "return": Reversed("return"), "else": Reversed("else"), "for": Reversed("for"),
                     "to": Reversed("to"),  "while": Reversed("while"), "downto": Reversed("downto"), "params": Reversed("params"),
                     "vars": Reversed("vars"), "begin": Reversed("begin"), "end": Reversed("end"), "and": Operator("&&"), 
                     "or": Operator("||")}
    for key in reversed_dict:
        GLOBAL_DICT[key] = reversed_dict[key]
    # 类型字典
    type_dict = {"numeric": Type("numeric"), "integer": Type("integer"), "bool": Type("integer"), "string": Type("string"),
                 "numericseries": Type("numericseries")}
    for key in type_dict:
        GLOBAL_DICT[key] = type_dict[key]
    # 系统字面量字典
    literal_dict = {"invalidnumeric": Literal("invalidnumeric", _type=VariableType(BaseType.FLOAT)), 
                    "true": Literal("true", _type=VariableType(BaseType.BOOL)), 
                    "false": Literal("false", _type=VariableType(BaseType.BOOL))}
    for key in literal_dict:
        GLOBAL_DICT[key] = literal_dict[key]
    return GLOBAL_DICT


if __name__ == "__main__":
    global_dict = SymbolTableGenerator()
    for key in global_dict:
        value: Token = global_dict[key]
        if isinstance(value, Function):
            print("{} {}".format(value.return_var.type.ShowType(), key), end="")
            param_str_list = []
            for param in value.param_list:
                param_str_list.append(param.type.ShowType())
            print("({})".format(", ".join(param_str_list)))
        else:
            print("name:{} type:{} data_type:{}".format(key, type(value), value.type.basetype))