from typing import List
from enum import IntEnum


class BaseType(IntEnum):
    VOID = 0        # 二进制 未定义
    BOOL = 1        # 逻辑变量
    INT = 2         # 整数
    FLOAT = 3       # 浮点数
    STRING = 4      # 字符串
    SERIES = 5      # 序列变量类型
    TEMPLATE = 6    # 模板类型


class VariableType(object):
    def __init__(self, basetype: BaseType, static: bool = False, const: bool = False, virtual: bool = False, 
                 reference: bool = False, pointer: bool = False):
        self.basetype = basetype
        self.static = static
        self.const = const
        self.virtual = virtual
        self.reference = reference
        self.pointer = pointer
        self.membertype = None

    def __str__(self) -> str:
        return self.ShowType()

    def SetMemberType(self, mtype) -> None:
        '''
        设置容器类储存对象的类型
        '''
        self.membertype = mtype

    def ShowType(self) -> str:
        '''
        显示类型
        '''
        bt = self.basetype
        if bt is None:
            return ""
        if bt == BaseType.VOID:
            typestr = "void"
        elif bt == BaseType.INT:
            typestr = "int"
        elif bt == BaseType.FLOAT:
            typestr = "float"
        elif bt == BaseType.BOOL:
            typestr = "bool"
        elif bt == BaseType.STRING:
            typestr = "string"
        elif bt == BaseType.SERIES:
            if type(self) == type(self.membertype):
                typestr = "series<{}>".format(self.membertype.ShowType())
            else:
                typestr = "series<error_type>"
        elif bt == BaseType.TEMPLATE:
            typestr = "template"
        else:
            typestr = "error_type"
        # 类型的其他属性
        const_str = ""
        static_str = ""
        reference_str = ""
        pointer_str = ""
        if self.const:
            const_str = "const "
        if self.static:
            static_str = "static "
        if self.reference:
            reference_str = "&"
        if self.pointer:
            pointer_str = "*"
        typestr = "{}{}{}{}{}".format(static_str, const_str, typestr, pointer_str, reference_str)
        return typestr


UNDEFINE_TYPE = VariableType(BaseType.VOID)


class Token(object):
    '''
    符号对象
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        self.name = name        # 名称
        self.type = _type       # 数据类型
        self.sub_tokens = []    # 子token
        self.father_token = None  # 父节点 供回溯使用
        self.syn_attr = None    # 综合属性
        self.inh_attr = None    # 继承属性
        self.line_id = 0        # 行号
        self.char_id = 0        # 位置

    def __len__(self) -> int:
        '''
        获取子token数量
        '''
        return len(self.sub_tokens)

    def GetInfo(self) -> str:
        '''
        输出该符号的 位置 名称 类型 信息
        '''
        return "line {}:{} name:{} type:{}".format(self.line_id, self.char_id, self.name, str(self.type))

    def SetPosition(self, line_id: int, char_id: int):
        '''
        设置token在源码中的位置
        '''
        self.line_id = line_id
        self.char_id = char_id

    def AddSubToken(self, token) -> None:
        '''
        添加子对象
        '''
        self.sub_tokens.append(token)
        # 设定子节点的父节点
        if isinstance(token, Token):
            token.father_token = self

    def GetSubToken(self, idx: int):
        '''
        添加子对象
        '''
        if idx >= len(self) or -idx > len(self):
            return None
        else:
            return self.sub_tokens[idx]

    def DelSubToken(self, idx: int):
        '''
        删除指定位置的子对象
        '''
        if idx >= len(self) or -idx > len(self):
            return
        else:
            del self.sub_tokens[idx]

    def GetName(self) -> str:
        '''
        获取词名
        '''
        return self.name.lower()

    def SetDataType(self, datatype: VariableType) -> None:
        '''
        更改数据类型
        '''
        self.type = datatype


class Node(Token):
    '''
    程序树结构中间节点
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)
        self.priority = 0

    def SetPriority(self, priority: int):
        '''
        设置运算符优先级 1 为最高
        '''
        self.priority = priority

    def GetName(self) -> str:
        '''
        获取词名
        '''
        return self.name


class Literal(Token):
    '''
    字面量类型
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)


class KeyWord(Token):
    '''
    关键字类型
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)


class Operator(Node):
    '''
    运算符类型
    '''
    pass


class Reversed(Token):
    '''
    保留字类型
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)


class Type(Token):
    '''
    数据变量类型
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)


class Function(Token):
    '''
    函数类型
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE) -> None:
        super().__init__(name, _type)
        self.param_list: List[Token] = []        # 参数类型列表
        self.return_var: Token = Variable("", _type)

    def SetParamList(self, params: List[Token]):
        '''
        设置参数列表
        '''
        self.param_list = params

    def SetReturnType(self, token: Token):
        '''
        设置返回值
        '''
        self.return_var = token
        self.type = token.type


class Variable(Token):
    '''
    变量
    '''
    def __init__(self, name: str, _type: VariableType = UNDEFINE_TYPE, comments: str = "", default: str = ""):
        super().__init__(name, _type)
        self.comments = comments
        self.default = default

    def __str__(self):
        return "type:{}".format(self._type)


class Integer(Variable):
    pass


class Numeric(Variable):
    pass


class Template(Variable):
    pass


class Bool(Variable):
    pass


class String(Variable):
    pass


class Series(Variable):
    pass


# 括号 包括圆括号() 方括号[]
class Brackets(Operator):
    def __str__(self):
        if len(self.sub_tokens) > 0:
            string_list = [str(unit) for unit in self.sub_tokens]
            return "{}{}{}{}{}".format(self.name[0], self.prefix, self.separator.join(string_list), self.suffix, self.name[-1])
        else:
            return self.prefix + self.name + self.suffix


# 根据指定的类型_type  创建一个TB变量对象
# 创建失败返回None
def CreateVar(name: str, type_str: str, static: bool = False, const: bool = False, virtual: bool = False, 
              reference: bool = False, pointer: bool = False, comments: str = "", default: str = "") -> VariableType:
    type_str = type_str.lower()
    var = None
    if type_str.find("series") < 0:
        if type_str == "integer":
            _type = VariableType(BaseType.INT, static, const, virtual, reference, pointer)
            var = Integer(name, _type, comments, default)
        elif type_str == "numeric":
            _type = VariableType(BaseType.FLOAT, static, const, virtual, reference, pointer)
            var = Numeric(name, _type, comments, default)
        elif type_str == "bool":
            _type = VariableType(BaseType.BOOL, static, const, virtual, reference, pointer)
            var = Bool(name, _type, comments, default)
        elif type_str == "string":
            _type = VariableType(BaseType.STRING, static, const, virtual, reference, pointer)
            var = String(name, _type, comments, default)
        elif type_str == "void":
            _type = VariableType(BaseType.VOID, static, const, virtual, reference, pointer)
            var = Variable(name, _type, comments, default)
        else:
            pass
            # raise TypeError("we find non TB type:{} name:{}".format(type_str, name))
    else:
        idx = type_str.find("<")
        # 提取子类名
        if idx >= 0:
            # Series<type>
            type_str = type_str[idx+1:-1]
        else:
            type_str = type_str[:-6]
        _type = VariableType(BaseType.SERIES, static, const, virtual, reference, pointer)
        # 识别储存变量类型
        if type_str == "integer":
            sub_type = VariableType(BaseType.INT)
        elif type_str == "numeric":
            sub_type = VariableType(BaseType.FLOAT)
        elif type_str == "bool":
            sub_type = VariableType(BaseType.BOOL)
        else:
            raise TypeError("we find non TB type: {}".format(type_str))
        _type.SetMemberType(sub_type)
        var = Series(name, _type, comments, default)
    return var


def CopyToken(ano_token: Token):
    '''
    Token类复制函数
    '''
    pass
