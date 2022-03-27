from io import FileIO
from copy import deepcopy
from typing import Dict, List
import sys
import os
sys.path.append(os.path.abspath("."))
sys.path.append(os.path.abspath(".."))
from Symbol import Operator, Token, Node, Variable, Reversed, Type, Function, Literal, CreateVar, VariableType


'''
对识别好的词法单元进行语法分析
'''


STATE_DEFAULT = 0
STATE_PARAM = 1
STATE_VARS = 2
STATE_MAJOR = 3
STATE_END = 4
# 运算符优先级
OPERATOR_PRIORITY = {"!": 1,
                     "*": 2, "/": 2, "%": 2, 
                     "+": 3, "-": 3,
                     ">=": 4, ">": 4, "<=": 4, "<": 4,
                     "==": 5, "!=": 5,
                     "&&": 6,
                     "||": 7,
                     "=": 8}


class UserSyntaxError(SyntaxError):
    def __init__(self, args):
        super().__init__(args)


def PrintCurrentToken(f: FileIO, token: Token, deep: int = 0):
    '''
    显示当前token
    '''
    for i in range(deep):
        f.write("  ")
    f.write(token.GetName() + "\n")


def ShowSubToken(f: FileIO, token: Token, deep: int = 0):
    '''
    显示子token
    '''
    if isinstance(token, Operator):
        brackets_dict = {"(": ")", "[": "]", "{": "}"} 
        if token.GetName() in brackets_dict.keys():
            # 显示当前token
            PrintCurrentToken(f, token, deep)
            # 显示子token
            for sub_token in token.sub_tokens:
                ShowSubToken(f, sub_token, deep+1)
            r_token = Operator(brackets_dict[token.GetName()])
            PrintCurrentToken(f, r_token, deep)
        else:
            # 显示当前token
            PrintCurrentToken(f, token, deep)
            for sub_token in token.sub_tokens:
                ShowSubToken(f, token.GetSubToken(0), deep+1)
    else:
        # 显示当前token
        PrintCurrentToken(f, token, deep)
        # 显示子token
        for sub_token in token.sub_tokens:
            ShowSubToken(f, sub_token, deep+1)


class SyntaxParser(object):
    def __init__(self) -> None:
        # 全局变量字典 不可变
        self.global_symbol_dict: Dict[str, Token] = {}
        # 局部变量字典 可变
        self.local_symbol_dict: Dict[str, Token] = {}
        # 当前状态 标记程序处理区域
        self.state = STATE_DEFAULT
        # 程序根节点
        self.root = Node("root")
        # 当前处理起始token
        self.begin_idx = 0
        # 当前处理终止token
        self.forward_idx = 0
        # 当前处理token
        self.cur_token = self.root
        # 原始token列表
        self.token_list: List[Token] = []
        # 原始token数量
        self.token_num: int = 0
        # 参数名称集合
        self.params_set = set()
        # 局部变量集合
        self.local_var_set = set()
        self.line_id = 0

    def SetSymbolTable(self, symbol_table: Dict[str, Token]):
        '''
        设置符号表
        '''
        self.global_symbol_dict = symbol_table

    def GetNextToken(self) -> Token:
        '''
        获取下一个词法单元
        越界返回None
        '''
        if self.forward_idx >= self.token_num:
            return None
        token = self.token_list[self.forward_idx]
        self.forward_idx += 1
        # 变量/函数/保留字 查询符号表
        if isinstance(token, Variable):
            name = token.GetName().lower()
            # 使用符号表的类型信息
            if name in self.global_symbol_dict.keys():
                # 全局变量
                new_token = deepcopy(self.global_symbol_dict[token.GetName().lower()])
            elif name in self.local_symbol_dict.keys():
                # 局部变量
                new_token = deepcopy(self.local_symbol_dict[token.GetName().lower()])
            else:
                # 如果当前符号在程序的参数区/变量定义区
                if self.state == STATE_PARAM or self.state == STATE_VARS:
                    pass
                new_token = token
            # 保留原有位置信息
            new_token.line_id = token.line_id
            new_token.char_id = token.char_id
            if token.line_id > self.line_id:
                # print("line: {}".format(token.line_id))
                self.line_id = token.line_id
            # 设置运算符优先级(含 and or)
            if isinstance(new_token, Operator) and new_token.GetName() in OPERATOR_PRIORITY.keys():
                new_token.SetPriority(OPERATOR_PRIORITY[new_token.GetName()])
            return new_token
        else:
            if isinstance(token, Operator) and token.GetName() in OPERATOR_PRIORITY.keys():
                token.SetPriority(OPERATOR_PRIORITY[token.GetName()])
            return token

    def ResetForwardIndex(self):
        '''
        Forward_idx 复位
        将forward_idx 向前移动一位
        '''
        if self.forward_idx > 0:
            self.forward_idx -= 1

    def AddLocalVar(self, new_token: Token):
        '''
        为本地符号表添加局部变量
        '''
        self.local_symbol_dict[new_token.GetName().lower()] = new_token
        if self.state == STATE_PARAM:
            self.params_set.add(new_token.GetName().lower())
        else:
            self.local_var_set.add(new_token.GetName().lower())

    def ParseBrackets(self, brackets_token: Token) -> Token:
        '''
        解析括号以及括号里的逗号
        ()
        []
        {}
        '''
        # print("brackets in:{}".format(brackets_token.GetName()))
        brackets_dict = {"(": ")", "[": "]", "{": "}"} 
        right_brackets = brackets_dict[brackets_token.GetName()]
        next_token = self.GetNextToken()
        # 检查下一个符号
        while next_token is not None:
            # 操作符
            if isinstance(next_token, Operator):
                # 是逗号 继续循环
                if next_token.GetName() == ",":
                    pass
                # 注释
                elif next_token.GetName() == "//":
                    pass
                # 是分号 继续循环 仅限花括号
                elif next_token.GetName() == ";":
                    if brackets_token.GetName() == "{":
                        pass
                # 发现内层括号
                elif next_token.GetName() in brackets_dict.keys():
                    sub_brackets_token = self.ParseBrackets(next_token)
                    brackets_token.AddSubToken(sub_brackets_token)
                # 是右括号号 退出循环
                elif next_token.GetName() == right_brackets:
                    # print("brackets end:{}".format(next_token.GetName()))
                    break
                else:
                    raise UserSyntaxError("line {}:{} missing {} we get:{}".format(
                                          next_token.line_id, next_token.char_id, right_brackets, next_token.GetName()))
            # 保留字
            elif isinstance(next_token, Reversed):
                if next_token.GetName() == "if":
                    if_token = self.ParseIfGrammer(next_token)
                    brackets_token.AddSubToken(if_token)
                elif next_token.GetName() == "for":
                    pass  # 待开发！！！
            # 变量/函数/字面量
            else:
                # 表达式
                exp_token = self.ParseExpression(next_token)
                if len(exp_token) > 0:
                    brackets_token.AddSubToken(exp_token)
                # 花括号 开始下一语句
                # if brackets_token.GetName() == "{":
                #     continue
                # else:
                #     raise UserSyntaxError("line {}:{} missing {} we get:{} name:{}".format(
                #                         next_token.line_id, next_token.char_id, right_brackets, type(next_token), next_token.GetName()))
            next_token = self.GetNextToken()
        return brackets_token

    def ParseFunction(self, fun_token: Token) -> Token:
        '''
        解析函数
        Fun()
        Fun(param1)
        Fun(param1, param2)
        '''
        next_token = self.GetNextToken()
        # 检查右括号
        if not isinstance(next_token, Operator) or next_token.GetName() != "(":
            self.ResetForwardIndex()
            return fun_token
        else:
            brackets_token = self.ParseBrackets(next_token)
            fun_token.AddSubToken(brackets_token)
            return fun_token

    def ParseExpression(self, next_token: Token) -> Token:
        '''
        解析表达式 文法如下
        E -> E op E  二元运算符
        E -> op E   一元运算符
        E -> i     字面量/变量/函数
        E -> (E)     括号
        '''
        exp_node = Node("E")  # 返回的表达式节点
        exp_node.SetPriority(99)
        exp_node.line_id = next_token.line_id
        cur_node = exp_node
        last_node = next_token
        while next_token is not None:
            # 运算符
            if isinstance(next_token, Operator):
                # 左括号
                if next_token.GetName() in ("(", "[", "{"):
                    tmp_node = self.ParseBrackets(next_token)
                    new_node = Node("E")
                    new_node.SetPriority(0)
                    new_node.LinkToken(tmp_node)
                    new_node.AddSubToken(tmp_node)
                    if next_token.GetName() == '[':
                        last_node.AddSubToken(tmp_node)
                    elif isinstance(last_node, Variable) or isinstance(last_node, Function):
                        last_node.AddSubToken(new_node)
                    else:
                        cur_node.AddSubToken(new_node)
                    # print("expression add sub bracteks")
                # 右括号
                elif next_token.GetName() in (")", "]", "}"):
                    # print("expression end with {}".format(next_token.GetName()))
                    self.ResetForwardIndex()    # 复位
                    break
                # 逗号 直接返回
                elif next_token.GetName() == ",":
                    self.ResetForwardIndex()    # 复位
                    break
                # 分号 直接返回
                elif next_token.GetName() == ";":
                    break
                # 注释 跳过
                elif next_token.GetName() == "//":
                    pass
                else:
                    # 识别负号
                    if next_token.GetName() == "-":
                        # 减号的情况 前一个符号是 变量/函数/字面量/括号
                        # [ ( 等括号会调用括号识别程序 导致 本应该是前一个符号的 ]) 变化为])
                        if isinstance(last_node, Variable) or isinstance(last_node, Function) or isinstance(last_node, Literal) or\
                           (isinstance(last_node, Operator) and last_node.GetName() in ('[', '(')):
                            pass
                        # 负号的情况 设置优先级为单运算符级别
                        else:
                            next_token.SetPriority(1)
                            print("recognition negative sign {}:{}".format(next_token.line_id, next_token.char_id))
                    # 当前节点为空的情况 表达式初始节点
                    new_node = Node("E")
                    new_node.SetPriority(next_token.priority)
                    new_node.LinkToken(next_token)
                    # 比较当前节点 与 前一个节点的优先级 若 当前节点优先级较高 则 根据优先级向上回溯
                    while cur_node.priority <= next_token.priority:
                        # 当前节点优先级较高 则去寻找当前节点的父节点
                        father_token: Node = cur_node.father_token
                        if father_token is not None:
                            cur_node = cur_node.father_token
                        else:
                            break
                    # 新节点优先级较高
                    if cur_node.priority > next_token.priority:
                        # 新节点优先级较高 新节点成为前节点的右子节点
                        if len(cur_node) > 2 or (len(cur_node) > 0 and cur_node is exp_node):
                            # 当前节点的最右子节点成为新节点的左子节点
                            sub_node = cur_node.GetSubToken(-1)
                            new_node.AddSubToken(sub_node)
                            cur_node.DelSubToken(-1)
                        # 在执行 添加左子节点后 再添加当前符号节点
                        new_node.AddSubToken(next_token)
                        cur_node.AddSubToken(new_node)
                    # 新节点优先级较低
                    else:
                        # 在执行 添加当前节点作新节点为左子节点后 再添加当前符号节点
                        new_node.AddSubToken(cur_node)
                        new_node.AddSubToken(next_token)
                    # 更新最新节点
                    cur_node = new_node
                # exp_node.AddSubToken(next_token)
            # 保留字
            elif isinstance(next_token, Reversed):
                self.ResetForwardIndex()    # 复位
                return exp_node
            else:
                # 函数
                if isinstance(next_token, Function):
                    terminal_node = self.ParseFunction(next_token)
                # 变量                                   字面量
                elif isinstance(next_token, Variable) or isinstance(next_token, Literal):
                    terminal_node = next_token
                # 新建语法节点
                new_node = Node("E")
                new_node.SetPriority(0)
                new_node.AddSubToken(terminal_node)
                new_node.LinkToken(terminal_node)
                cur_node.AddSubToken(new_node)
            # 获取下一个词法单元
            last_node = next_token
            next_token = self.GetNextToken()
        # endwhile 表达式分析结束
        # 无效表达式的情况 返回根节点
        if len(exp_node) == 0:
            return exp_node
        return_node = exp_node.GetSubToken(0)
        return_node.line_id = exp_node.line_id
        return return_node

    def ParseDefineGrammer(self, type_token: Token) -> Token:
        '''
        解析定义语句
        文法
        D -> Type Var;
        D -> Type Var DefaultValue;
        '''
        new_node = Node("D")
        new_node.SetPriority(0)
        new_node.AddSubToken(type_token)
        new_node.line_id = type_token.line_id
        next_token = self.GetNextToken()
        new_node.LinkToken(type_token)
        new_node.AddSubToken(next_token)
        # print("cur:{} next:{}".format(type_token.GetName(), next_token.GetName()))
        if isinstance(next_token, Variable):
            var_token = next_token
            # print("find var:{}".format(var_token.GetName()))
            default_str = ""
            next_token = self.GetNextToken()
            # 有默认值的情况 格式 (默认值)
            if isinstance(next_token, Operator) and next_token.GetName() == "(":
                bra_token = self.ParseBrackets(next_token)
                # 解析默认值！！！！！
                if len(bra_token) > 0:
                    new_node.AddSubToken(bra_token.GetSubToken(0))
                # 获取语句末尾的分号
                next_token = self.GetNextToken()
                print("default next:{}".format(next_token.GetName()))
            # 检查语句末尾的分号
            if isinstance(next_token, Operator) and next_token.GetName() == ";":
                pass
            else:
                raise UserSyntaxError("line {}:{} missing ; we get:{}".format(next_token.line_id, next_token.char_id, next_token.GetName()))
            print("find new var name:{} type:{}".format(var_token.GetName(), type_token.GetName()))
            new_token = CreateVar(name=var_token.GetName(), type_str=type_token.GetName(), default=default_str)
            self.AddLocalVar(new_token)
            # self.cur_token.AddSubToken(new_token)
            return new_node
        # 异常 类型后面没有跟变量名称
        else:
            raise UserSyntaxError("line {}:{} missing variable name".format(next_token.line_id, next_token.char_id))

    def ParseConditionStatment(self, token: Token) -> Token:
        '''
        解析 if 与 else下属的分支语句
        '''
        # 检查是否存在 {}代码段
        if isinstance(token, Operator) and token.GetName() == "{":
            # 解析{}内的代码
            bra_token = self.ParseBrackets(token)
            return bra_token
        elif isinstance(token, Reversed):
            # 解析下一级if
            if token.GetName() == "if":
                if_token = self.ParseIfGrammer(token)
                return if_token
            else:
                raise UserSyntaxError("line {}:{} Unexcept Reversed:{} in if".format(token.line_id, token.char_id, token.GetName()))
        else:
            # 读取一条表达式 作为if的执行语句
            exp_token = self.ParseExpression(token)
            if len(exp_token) == 0:
                raise UserSyntaxError("line {}:{} Unexcept Reversed:{} in if".format(token.line_id, token.char_id, token.GetName()))
            else:
                return exp_token

    def ParseIfGrammer(self, if_token: Token) -> Token:
        '''
        解析 If语句
        if 对象下有三个 成员 condition if_do else_do（可选）
        if () {} else {}
        if () {} else if () {} else {}
        if () if () {} else {} else if () {} else {}
        '''
        next_token = self.GetNextToken()
        # 检查左括号
        if not isinstance(next_token, Operator) or next_token.GetName() != "(":
            raise UserSyntaxError("line {}:{} missing ( after if, we get:{} name:{}".format(
                                  next_token.line_id, next_token.char_id, type(next_token), next_token.name))
        # 解析语句
        bra_token = self.ParseBrackets(next_token)
        if_token.AddSubToken(bra_token)
        # 检查是否存在 {}代码段
        next_token = self.GetNextToken()
        sub_token = self.ParseConditionStatment(next_token)
        if_token.AddSubToken(sub_token)
        # 检查else
        next_token = self.GetNextToken()
        if isinstance(next_token, Reversed) and next_token.GetName() == "else":
            # 解析else下的条件语句
            next_token = self.GetNextToken()
            sub_token = self.ParseConditionStatment(next_token)
            if_token.AddSubToken(sub_token)
        else:
            self.ResetForwardIndex()    # 复位
        return if_token

    def ParseSingleToken(self):
        '''
        解析单个词法单元
        '''
        token: Token = self.GetNextToken()
        # print("cur:{} type:{}".format(token.GetName(), type(token)))
        # 处理保留字
        if isinstance(token, Reversed):
            name = token.GetName()
            if name == "params":
                self.root.AddSubToken(token)
                self.state = STATE_PARAM
                self.cur_token = token
                return
            elif name == "vars":
                self.root.AddSubToken(token)
                self.state = STATE_VARS
                self.cur_token = token
                return
            elif name == "begin":
                self.root.AddSubToken(token)
                self.state = STATE_MAJOR
                self.cur_token = token
                return
            elif name == "end":
                # self.root.AddSubToken(token)
                self.state = STATE_END
                return
            elif name == "if":
                if_token = self.ParseIfGrammer(token)
                self.cur_token.AddSubToken(if_token)
            else:
                pass
        # 处理类型定义语句
        elif isinstance(token, Type):
            def_token = self.ParseDefineGrammer(token)
            self.cur_token.AddSubToken(def_token)
        # 处理表达式语句
        else:
            exp_token = self.ParseExpression(token)
            if len(exp_token) > 0:
                self.cur_token.AddSubToken(exp_token)

    def Parse(self, token_list: List[Token]) -> Token:
        '''
        解析词法单元
        识别变量/函数/保留字
        '''
        self.token_list = token_list
        self.token_num = len(token_list)
        print("get {} token".format(self.token_num))
        while self.begin_idx < self.token_num:
            self.ParseSingleToken()
            self.begin_idx = max(self.forward_idx, self.begin_idx + 1)
        print(">>>> SyntaxAnalysis Over <<<<")
        return self.root

    def Show(self):
        '''
        输出程序的语法结构到指定文件
        '''
        with open("Syntax.txt", "w") as f:
            ShowSubToken(f, self.root, 0)


if __name__ == "__main__":
    import os
    import sys
    sys.path.append(os.path.abspath("."))
    sys.path.append(os.path.abspath(".."))
    # 加载全局符号表
    from Translate.SymbolTableGenerator import SymbolTableGenerator
    global_symbol_table = SymbolTableGenerator()
    # 加载词法分析结果
    from Translate.LexicalAnalysis import LexicalAnalysis
    token_list = LexicalAnalysis("./Translate/test.tb")
    # 创建语法分析器
    sp = SyntaxParser()
    sp.SetSymbolTable(global_symbol_table)
    sp.Parse(token_list)
    sp.Show()
