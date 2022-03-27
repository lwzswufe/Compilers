from io import FileIO
from typing import Dict, List
import sys
import os
from unicodedata import name
sys.path.append(os.path.abspath("."))
sys.path.append(os.path.abspath(".."))
from Symbol import Operator, Token, Node, Variable, Reversed, Type, VariableType, BaseType, Function, Literal, CreateVar


'''
对语法分析结果进行语法制导
'''


TYPE_INT = VariableType(BaseType.INT)
TYPE_VOID = VariableType(BaseType.VOID)
TYPE_BOOL = VariableType(BaseType.BOOL)
TYPE_FLOAT = VariableType(BaseType.FLOAT)
TYPE_STRING = VariableType(BaseType.STRING)
TYPE_TEMPLATE = VariableType(BaseType.TEMPLATE)


class UserTypeError(TypeError):
    '''
    用户自定义类型错误
    '''
    def __init__(self, args):
        super().__init__(args)


def PrintCurrentToken(f: FileIO, token: Token, deep: int = 0):
    '''
    显示当前token
    '''
    for i in range(deep):
        f.write("  ")
    if deep == 2:
        s = "{} ({}) line:{}\n".format(token.GetName(), token.type, token.line_id)
    else:
        s = "{} ({})\n".format(token.GetName(), token.type)
    f.write(s)


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
            for i in range(len(token)):
                ShowSubToken(f, token.GetSubToken(i), deep+1)
    else:
        # 显示当前token
        PrintCurrentToken(f, token, deep)
        # 显示子token
        for sub_token in token.sub_tokens:
            ShowSubToken(f, sub_token, deep+1)

def IsRefFunction(node: Node):
    '''
    判断某一个非终结符是不是函数对象
    '''
    if isinstance(node, Node) and node.terminal is not None:
        sub_token = node.terminal
        return isinstance(sub_token, Function) and IsFinalParamIsRef(sub_token)
    return False


def GetCommonType(left_type: VariableType, right_type: VariableType) -> VariableType:
    '''
    获取两个变量的最大公约类型
    '''
    if left_type.basetype == BaseType.SERIES:
        if left_type.membertype is not None:
            return GetCommonType(left_type.membertype, right_type)
    if right_type.basetype == BaseType.SERIES:
        if right_type.membertype is not None:
            return GetCommonType(left_type, right_type.membertype)
    # 左右类型里只有一个时字符串类型
    if (left_type.basetype == BaseType.STRING) != (right_type.basetype == BaseType.STRING):
        raise UserTypeError(" {} can not convert {}".format(left_type, right_type))
    # 返回拓展类型
    if left_type.basetype >= right_type.basetype:
        return left_type
    else:
        return right_type


def IsEquivalentBaseType(data_type: VariableType, target_type: VariableType) -> bool:
    '''
    检查两个类型的基础类型是否等价 忽略 const 引用 指针 属性
    '''
    if target_type is None:
        return True
    if data_type.basetype == target_type.basetype:
        return True
    elif (data_type.basetype == BaseType.INT) and (target_type.basetype == BaseType.FLOAT):
        return True
    elif (data_type.basetype == BaseType.SERIES) and IsEquivalentBaseType(data_type.membertype, target_type):
        return True
    else:
        return False


def IsFinalParamIsRef(fun_token: Function) -> bool:
    '''
    检查函数的最后一个参数是否是引用类型
    '''
    # 检查函数的子对象 是否越界
    if len(fun_token) != 1:
        raise UserTypeError("fun {} sub_tokens {} != 1".format(fun_token.GetName(), len(fun_token)))
    # 检查传入参数是否是比函数要求参数少一个 且最后一个参数要求引用类型
    bra_token = fun_token.GetSubToken(0)
    if len(fun_token.param_list) == len(bra_token) + 1:
        ref_param: Token = fun_token.param_list[-1]
        return ref_param.type.reference
    else:
        return False


class TypeCheck(object):
    def __init__(self):
        '''
        类型检查类
        '''
        self.temp_var_num = 0   # 临时对象计数
        self.root = None        # 根节点
        self.body = None        # 程序主体部分
        self.vars = None        # 变量定义部分
        self.idx = -1           # 当前检查语句的编号 方便后期插入语句
        self.insert_statement = 0  # 程序在本次类型检查中插入的语句数量
        # 全局变量字典 不可变
        self.global_symbol_dict: Dict[str, Token] = {}
        # 局部变量字典 可变
        self.local_symbol_dict: Dict[str, Token] = {}

    def GetNextIndex(self) -> int:
        '''
        获取下一条需要检查的语句的编号
        '''
        next_idx = self.idx + 1 + self.insert_statement
        self.insert_statement = 0
        return next_idx

    def CreateTempVar(self, var_type: VariableType) -> Token:
        '''
        创建临时变量
        '''
        # 临时变量计数
        self.temp_var_num += 1
        name = "temp_var{:03d}".format(self.temp_var_num)
        token = CreateVar(name, var_type)
        # 变量定义区添加变量
        self.vars.append(token)
        return var_type

    def InsertStatement(self, token: Token) -> None:
        '''
        插入一条语句到程序主体语句
        '''
        new_node = Node("E")
        new_node.SetPriority(0)
        new_node.LinkToken(token)
        new_node.AddSubToken(token)
        self.body.sub_tokens.insert(self.idx, new_node)
        self.insert_statement += 1

    def ReplaceStatement(self, token: Token) -> None:
        '''
        插入一条语句到程序主体语句
        '''
        self.body.sub_tokens[self.idx] = token

    def TheReferenceParamFunAssign(self, fun_token: Function) -> Token:
        '''
        识别特殊语法
        调整有引用类型的参数的函数的语法
        样例
        ref_var = fun(var1, var_2, var3) =>  fun(var1, var2, ref_var)
        '''
        node = fun_token.father_token
        # 提取赋值运算符
        assign_node = node.father_token
        # 引用对象 原赋值运算符左节点
        ref_node = assign_node.GetSubToken(0)
        # 获取函数附带的括号对象
        bra_token = fun_token.GetSubToken(0)
        # 将引用对象添加为参数对象
        bra_token.AddSubToken(ref_node)
        # 替换之前的函数语句
        self.ReplaceStatement(fun_token)
        return fun_token

    def TheReferenceParamFunOperator(self, fun_token: Function) -> Token:
        '''
        识别特殊语法
        调整有引用类型的参数的函数的语法
        样例
        assign_var = var_0 + fun(var1, var_2) =>  fun(var1, var_2, ref_var)
                                                assign_var = var_0 + ref_var
        '''
        # 创建临时对象
        ref_token = self.CreateTempVar(fun_token.param_list[-1].type)
        # 获函数括号对象
        bra_token = fun_token.GetSubToken(0)
        # 将临时对象添加为函数参数
        bra_token.AddSubToken(ref_token)
        # 将函数语句放到本条语句之前
        self.InsertStatement(fun_token)
        # 返回引用对象 用于替换原有对象
        return ref_token

    def MarkOperatorType(self, node: Node) -> VariableType:
        '''
        标记终结符为运算符的 节点的数据类型
        '''
        op_token = node.terminal
        name = op_token.GetName()
        # 比较运算符 逻辑运算符
        if name in ('==', '!=', '>', '>=', '<', '<='):
            node.SetDataType(TYPE_BOOL)
            self.MarkTokenType(node.GetSubToken(0))
            self.MarkTokenType(node.GetSubToken(-1))
        elif name in ('!', '&&', "||"):
            node.SetDataType(TYPE_BOOL)
            for sub_token in node.sub_tokens:
                sub_token.SetDataType(TYPE_BOOL)
            if len(node) >= 3:
                self.MarkTokenType(node.GetSubToken(0))
            self.MarkTokenType(node.GetSubToken(-1))
        # 算数运算符
        elif name in ('+', '-', '*', '/', '%'):
            if len(node) <= 1:
                pass
            elif len(node) == 2:
                member_type = self.MarkTokenType(node.GetSubToken(1))
                node.SetDataType(member_type)
            elif len(node) == 3:
                # 二元运算符计算时 结算结果为较大类型 如果是容器类 则自动提取容器成员类型
                left_type = self.MarkTokenType(node.GetSubToken(0))
                right_type = self.MarkTokenType(node.GetSubToken(-1))
                # 寻找两个类型的共同类型
                try:
                    common_type = GetCommonType(left_type, right_type)
                    node.type = common_type
                except UserTypeError as err:
                    left_token = node.GetSubToken(0)
                    right_token = node.GetSubToken(-1)
                    print("line {}:{}  token {} {}  error:{}".format(op_token.line_id, op_token.char_id, left_token.GetName(), right_token.GetName(), err))
            else:
                pass
        elif name == "=":
            # 标记左节点
            self.MarkTokenType(node.GetSubToken(0))
            # 等号类型等于右节点
            member_type = self.MarkTokenType(node.GetSubToken(-1))
            node.SetDataType(member_type)
        # 括号 仅限运算表达式的类型
        elif name in ('(', '['):
            if len(node) == 1:
                node.type = self.MarkTokenType(node.GetSubToken(0))
        # 标记子节点类型
        # for sub_token in node.sub_tokens:
        #     self.MarkTokenType(sub_token)
        op_token.SetDataType(node.type)
        return node.type

    def MarkVariableType(self, token: Token):
        '''
        标记变量的数据类型
        '''
        name = token.GetName()
        # 全局变量
        if name in self.global_symbol_dict.keys():
            token.SetDataType(self.global_symbol_dict[name].type)
        # 局部变量
        elif name in self.local_symbol_dict.keys():
            token.SetDataType(self.local_symbol_dict[name].type)
        else:
            pass
        if len(token) > 0:
            self.MarkTokenType(token.GetSubToken(0))
        # 序列变量的情况
        if token.type.basetype == BaseType.SERIES:
            # 有下标访问符号的情况
            if len(token) > 0:
                return token.type.membertype
        return token.type

    def MarkNodeType(self, node: Node) -> VariableType:
        '''
        标记 语法节点 (非终结符) 的类型
        '''
        # 非终结符
        terminal = node.terminal
        if isinstance(terminal, Operator):
            self.MarkOperatorType(node)
        elif isinstance(terminal, Type):
            self.MarkDefineType(node)
        elif terminal is not None:
            self.MarkTokenType(terminal)
        else:
            a = 1
        node.SetDataType(terminal.type)
        return node.type

    def MarkLiteralType(self, token: Token) -> VariableType:
        '''
        标记字面量类型
        '''
        name = token.GetName()
        is_string = sum([c.isalpha() for c in name]) > 0
        if name in self.global_symbol_dict.keys():
            token.SetDataType(self.global_symbol_dict[name].type)
        elif is_string or name.find('"') > 0:
            token.SetDataType(TYPE_STRING)
        elif name.find(".") > 0:
            token.SetDataType(TYPE_FLOAT)
        else:
            token.SetDataType(TYPE_INT)
        return token.type

    def MarkFunctionType(self, token: Token) -> VariableType:
        '''
        标记函数类型
        '''
        name = token.GetName()
        token.SetDataType(self.global_symbol_dict[name].type)
        if len(token) > 0:
            self.MarkTokenType(token.GetSubToken(0))
        # 模板函数确认类型
        if IsEquivalentBaseType(token.type, TYPE_TEMPLATE):
            bra_token = token.GetSubToken(0)
            param_type = self.MarkTokenType(bra_token.GetSubToken(0))
            token.SetDataType(param_type)
            bra_token.SetDataType(param_type)
        return token.type

    def MarkReversedType(self, token: Token) -> VariableType:
        '''
        标记保留字的类型
        '''
        name = token.GetName()
        if name in ("if"):
            if len(token) > 0:
                cond_token = token.GetSubToken(0)
                cond_token.SetDataType(TYPE_BOOL)
        # 标记子节点类型
        for sub_token in token.sub_tokens:
            self.MarkTokenType(sub_token)
        return token.type

    def MarkDefineType(self, node: Node) -> VariableType:
        '''
        标记定义语句的类型
        '''
        type_token = node.GetSubToken(0)
        var_token = node.GetSubToken(1)
        # 查询符号表
        name = var_token.GetName()
        if name in self.global_symbol_dict.keys():
            var_token.SetDataType(self.global_symbol_dict[name].type)
        elif name in self.local_symbol_dict.keys():
            var_token.SetDataType(self.local_symbol_dict[name].type)
        else:
            pass
        type_token.type = var_token.type
        # 标注默认值类型
        if len(node) > 2:
            value_node: Node = node.GetSubToken(2)
            value_node.SetDataType(var_token.type)
            self.MarkNodeType(value_node)
        node.type = var_token.type
        return node.type

    def MarkTokenType(self, token: Token) -> VariableType:
        '''
        标记每一个节点数据类型 返回节点的数据类型
        '''
        # 变量
        if isinstance(token, Variable):
            self.MarkVariableType(token)
        # 字面量
        elif isinstance(token, Literal):
            self.MarkLiteralType(token)
        # 函数
        elif isinstance(token, Function):
            self.MarkFunctionType(token)
        # 保留字
        elif isinstance(token, Reversed):
            self.MarkReversedType(token)
        # 运算符 在MarkNodeType里处理
        elif isinstance(token, Operator):
            for sub_token in token.sub_tokens:
                self.MarkTokenType(sub_token)
        # 非终结符
        elif isinstance(token, Node):
            token.type = self.MarkNodeType(token)
        return token.type

    def CheckOperatorTokenType(self, token: Token, target_type: VariableType):
        '''
        检查在运算符后面使用带引用参数的函数的情况
        '''
        node: Node = token.father_token
        if not isinstance(node, Node):
            for sub_token in token.sub_tokens:
                self.CheckTokenType(sub_token, target_type)
            return
        node.SetDataType(target_type)
        terminal = node.terminal
        if terminal is None:
            return
        name = terminal.GetName()
        if name == "=":
            # 在赋值运算符后使用此类函数的情况
            sub_node = node.GetSubToken(-1)
            # 如果赋值运算符右节点是函数 且 该函数的最后一个参数是引用类型
            if IsRefFunction(sub_node):
                return_token = self.TheReferenceParamFunAssign(sub_node.terminal)
                # 检查函数节点类型
                self.CheckTokenType(return_token, TYPE_VOID)
                # 清空当前节点的子节点
                node.sub_tokens.clear()
                # 将新生成的节点作为非终结符对应的节点
                node.AddSubToken(return_token)
                # 返回 原有token已经被替换为函数
                return
        elif name in ("+", "-", '*', "/", "%", "!"):
            # 在运算符后使用此类函数的情况
            for i, sub_token in enumerate(node.sub_tokens):
                if IsRefFunction(sub_token):
                    return_token = self.TheReferenceParamFunOperator(sub_token.terminal)
                    # 使用临时对象替换原来的函数对象
                    sub_token.sub_tokens.clear()
                    sub_token.AddSubToken(return_token)
                    sub_token.terminal = return_token
        # 如果该运算符的类型未知 标记类型
        # if IsEquivalentBaseType(token.type, TYPE_VOID):
        #     # 自底向上标记类型
        #     self.MarkTokenType(token)
        # 检查子节点数量
        # 括号 非函数参数的情况
        if name in ('(', '[', '{'):
            for sub_token in token.sub_tokens:
                self.CheckTokenType(sub_token, target_type)
            pass
        elif len(node) <= 0:
            raise UserTypeError("line {}:{} token:{} has less than 0 sub tokenes".format(node.line_id, node.char_id, name))
        elif len(node) == 1:
            pass
        # 一元运算符
        elif len(node) == 2:
            if name not in ('-', '!'):
                raise UserTypeError("line {}:{} token:{} has more than 2 sub tokenes".format(node.line_id, node.char_id, name))
            sub_token: Token = node.GetSubToken(1)
            self.CheckTokenType(sub_token, node.type)
        # 二元运算符
        elif len(node) == 3:
            # 检查左右节点类型是否一致 不一致的话考虑窄化/泛化
            left_token = node.GetSubToken(0)
            right_token = node.GetSubToken(-1)
            # 不一致的情况
            if not IsEquivalentBaseType(left_token.type, right_token.type):
                pass
            # 检查右值运算符
            if name == '=' and left_token.type.const:
                raise UserTypeError("line {}:{} token:{} at left of the = is const".format(node.line_id, node.char_id, name))
            # 获取共同类型
            try:
                if name in ("&&", "||"):
                    common_type = TYPE_BOOL
                else:
                    common_type = GetCommonType(left_token.type, right_token.type)
                # 检查左右节点
                self.CheckTokenType(left_token, common_type)
                self.CheckTokenType(right_token, common_type)
            except UserTypeError as err:
                print("line {}:{}  token {} {}  error:{}".format(node.line_id, node.char_id, left_token.GetName(), right_token.GetName(), err))
        else:
            raise UserTypeError("line {}:{} token:{} has more than 2 sub tokenes".format(node.line_id, node.char_id, name))

    def CheckNodeType(self, node: Node, target_type: VariableType):
        '''
        检查文法符号的类型
        '''
        if IsEquivalentBaseType(target_type, TYPE_VOID) and not IsEquivalentBaseType(node.type, TYPE_VOID):
                target_type = node.type
        if node.terminal is not None and isinstance(node.terminal, Operator):
            self.CheckOperatorTokenType(node.terminal, target_type)
        else:
            for token in node.sub_tokens:
                self.CheckTokenType(token, target_type)

    def CheckVariableTokenType(self, token: Token, target_type: VariableType):
        '''
        变量节点类型检查
        '''
        name = token.GetName()
        # 检查下标访问类型 整数
        if len(token) > 0:
            sub_token: Token = token.GetSubToken(0)
            sub_token.SetDataType(TYPE_INT)
            self.CheckTokenType(sub_token, TYPE_INT)
        # 全局变量的情况
        if name in self.global_symbol_dict.keys():
            origin_type = self.global_symbol_dict[name].type
            # 检查数据类型 数据类型和目标类型的基础类型是否一致
            if not IsEquivalentBaseType(origin_type, target_type):
                raise TypeError("line {}:{} error symbol:{} type:{} target:{}".format(token.line_id, token.char_id, name, origin_type, target_type))
        # 局部变量的情况  可调整变量类型
        elif name in self.local_symbol_dict.keys():
            origin_type = self.local_symbol_dict[name].type
            # 检查数据 数据类型和目标类型的基础类型是否一致
            if not IsEquivalentBaseType(origin_type, target_type):
                self.local_symbol_dict[name].SetDataType(target_type)
                token.SetDataType(target_type)
                print("line {}:{} symbol:{} set type {} -> {}".format(token.line_id, token.char_id, name, origin_type, target_type))
        else:
            raise TypeError("line {}:{} error symbol:{}".format(token.line_id, token.char_id, name))

    def CheckFunctionTokenType(self, token: Token, target_type: VariableType):
        '''
        '''
        if len(token) == 0:
            # 自动为函数补全括号
            brackets = Operator("(")
            token.AddSubToken(brackets)
        # 提取括号
        bra_token: Token = token.GetSubToken(0)
        # 检查参数数量
        if len(bra_token) != len(token.param_list):
            print("function {} need {} params but get {}".format(token.GetName(), len(token.param_list), len(bra_token)))
            pass
        # 依次检查参数类型
        for i, sub_token in enumerate(bra_token.sub_tokens):
            # 检查参数类型 模板类型暂时翻译为浮点数类型
            if token.param_list[i].type.basetype == BaseType.TEMPLATE:
                self.CheckTokenType(sub_token, TYPE_FLOAT)
            else:
                self.CheckTokenType(sub_token, token.param_list[i].type)
            
    def CheckTokenType(self, token: Token, target_type: VariableType):
        '''
        检查token类型
        '''
        name = token.GetName()
        # 检查当前节点的类型是否与要求类型一致
        # if not IsEquivalentBaseType(token, target_type):
        #     raise UserTypeError("line {}:{} target type {} user type:{}".format(token.line_id, token.char_id, token.type, target_type))
        # 指针类型检查
        # elif token.type.pointer == target_type.pointer:
        #     pass
        # 对于操作符
        if isinstance(token, Operator):
            self.CheckOperatorTokenType(token, target_type)
        # 函数
        elif isinstance(token, Function):
            self.CheckFunctionTokenType(token, target_type)
        # 变量
        elif isinstance(token, Variable):
            self.CheckVariableTokenType(token, target_type)
        # 保留字
        elif isinstance(token, Reversed):
            # if 语句
            if name == "if":
                # 要求第一个子单元类型为布尔 其他语句的类型为 void
                for i, sub_token in enumerate(token.sub_tokens):
                    sub_token: Token = token.GetSubToken(i)
                    if i == 0:
                        sub_token.SetDataType(TYPE_BOOL)
                        self.CheckTokenType(sub_token, TYPE_BOOL)
                    else:
                        sub_token.SetDataType(TYPE_VOID)
                        self.CheckTokenType(sub_token, TYPE_VOID)
        # 字面量
        elif isinstance(token, Literal):
            is_string = sum([c.isalpha() for c in name]) > 0
            if is_string or name.find('"') > 0:
                if name not in self.global_symbol_dict:
                    token.SetDataType(TYPE_STRING)
            elif name.find(".") > 0:
                token.SetDataType(TYPE_FLOAT)
            else:
                token.SetDataType(TYPE_INT)
        # 文法符号
        elif isinstance(token, Node):
            self.CheckNodeType(token, target_type)
        else:
            pass
        # 对子节点进行检查

    def Check(self, root: Token):
        '''
        类入口
        '''
        self.root = root
        for token in root.sub_tokens:
            # 只对函数主体部分进行类型检查 忽略定义部分
            if token.GetName() == "begin":
                self.body = token
                self.idx = 0
                # 逐条检查语句类型 程序可能会插入语句
                while self.idx < len(token):
                    print("line: {}/{}".format(self.idx, len(token)))
                    self.MarkTokenType(token.GetSubToken(self.idx))
                    self.CheckTokenType(token.GetSubToken(self.idx), TYPE_VOID)
                    self.idx = self.GetNextIndex()
            elif token.GetName() in ("params", "vars"):
                for sub_token in token.sub_tokens:
                    self.MarkTokenType(sub_token)

    def Show(self):
        '''
        显示类型检查的结果
        '''
        with open("TypeCheck.txt", "w") as f:
            ShowSubToken(f, self.root, 0)

    def SetGlobalSymbolTable(self, symbol_table: Dict[str, Token]):
        '''
        设置符号表
        '''
        self.global_symbol_dict = symbol_table

    def SetLocalSymbolTable(self, symbol_table: Dict[str, Token]):
        '''
        设置符号表
        '''
        self.local_symbol_dict = symbol_table


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
    from Translate.SyntaxAnalysis import SyntaxParser
    sp = SyntaxParser()
    sp.SetSymbolTable(global_symbol_table)
    tree: Token = sp.Parse(token_list)
    sp.Show()
    # 类型检查
    tc = TypeCheck()
    tc.SetGlobalSymbolTable(global_symbol_table)
    tc.SetLocalSymbolTable(sp.local_symbol_dict)
    tc.Check(tree)
    tc.Show()
