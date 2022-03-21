#include <algorithm>
#ifndef TBTYPE_H
#include "TBtype.h"
#endif
#pragma once


// 返回参数的绝对值
// 参数 value 需要计算其绝对值的实数 
Numeric Abs(Numeric value);
// 快速计算平均值
// Price 用于求和的值，必须是数值型序列值；
// Length 是需要计算的周期数，为整型。
// Series<Numeric> &SumValue 缓存所需临时变量
// 当序列值的CurrentBar小于Length时，该函数返回无效值。
void AverageFC(const Series<Numeric>&Price, Integer Length, Series<Numeric> &SumValue);
// 求平均
// Price 用于求和的值，必须是数值型序列值；
// Length 是需要计算的周期数，为整型。
// 当序列值的CurrentBar小于Length时，该函数返回无效值。
Numeric Average(const Series<Numeric>&Price, Integer Length);
// 求是否上穿
// Price1 求交叉的数据源1;
// Price2 求交叉的数据源2。
// 该函数返回Price1数值型序列值是否上穿Price2数值型序列值，返回值为布尔型。
Bool CrossOver(const Series<Numeric>& Price1,const Series<Numeric>& Price2);
// 求是否下破
// Price1 求交叉的数据源1;
// Price2 求交叉的数据源2。
// 该函数返回Price1数值型序列值是否下破Price2数值型序列值，返回值为布尔型;
Bool CrossUnder(const Series<Numeric>& Price1,const Series<Numeric>& Price2);
// 获取最近N周期条件满足的计数
// TestCondition 传入的条件表达式；
// Length 计算条件的周期数。
Integer CountIf(const Series<Bool> TestCondition, const Integer Length);
// 求累计值
// Series<Numeric> &CumValue 缓存所需临时变量
// Price 用于求累计值的值，必须是数值型序列值。
void Cum(const Series<Numeric>&Price, Series<Numeric> &CumValue);
// Integer date1  yyyymmdd 格式日期  必须在19700101之后
// Integer date2  yyyymmdd 格式日期  必须在19700101之后
// DateDiff(20060203, 20060208) = 5
// DateDiff(20060315, 20060310) = -5
// 两个日期之间的天数间隔
Integer DateDiff(Integer date1, Integer date2);
// 求极值
//  Series<Numeric> Price
//  Integer Length
//  Bool bMax
//  Numeric &ExtremeBar
Numeric Extremes(const Series<Numeric>& Price,Integer Length,Bool bMax, Series<Integer> &Temp);
// 求最高
//  Series<Numeric> Price
//  Integer Length
Numeric Highest(const Series<Numeric>& Price,Integer Length);
// 求最高
//  Series<Numeric> Price
//  Integer Length
Numeric HighestFC(const Series<Numeric>& Price,Integer Length, Series<Integer> &Temp);
// 执行真假值判断，根据逻辑测试的真假值返回不同的数值。
// Conditon 条件表达式；
// TrueValue 条件为True时的返回值；
// FalseValue 条件为False时的返回值。 
Numeric IIF(Bool Conditon,Numeric TrueValue,Numeric FalseValue);
// 返回实数舍入后的整数值。
// Number 需要进行取整处理的实数。
// IntPart (8.9) = 8；
// IntPart (-8.9) = -9。 
Integer IntPart(Numeric Number);
// 求最低
// Price 用于求最低值的值，必须是数值型序列值;
// Length 是需要计算的周期数，为整型。
// 该函数计算指定周期内的数值型序列值的最低值，返回值为浮点数。
Numeric Lowest(const Series<Numeric>& Price,Integer Length);
// 求最低(快速计算版本)
//  Series<Numeric> Price
//  Integer Length
Numeric LowestFC(const Series<Numeric>& Price,Integer Length, Series<Integer> &Temp);
// 求较大值
const Integer Max(const Integer a, const Integer b);
// 求较大值
const Numeric Max(const Numeric a, const Integer b);
// 求较大值
const Numeric Max(const Integer a, const Numeric b);
// 求较大值
const Numeric Max(const Numeric a, const Numeric b);
// 求较小值
const Integer Min(const Integer a, const Integer b);
// 求较小值
const Numeric Min(const Numeric a, const Integer b);
// 求较小值
const Numeric Min(const Integer a, const Numeric b);
// 求较小值
const Numeric Min(const Numeric a, const Numeric b);
// 求动量
//  Series<Numeric> Price
//  Integer Length
Numeric Momentum(const Series<Numeric>& Price,Integer Length);
// 第N个满足条件的Bar距当前的Bar数目
// Con 传入的条件表达式；
// N 求第N个满足条件中的N值，N = 1　表示最近的一个，N = 2 为倒数第二个。
Integer NthCon(const Series<Bool> Con, Integer N);
//  求振荡
//  Series<Numeric> Price
//  Integer FastLength
//  Integer SlowLength
Numeric PriceOscillator(const Series<Numeric>& Price,Integer FastLength,Integer SlowLength);
// 返回某个数字按指定位数舍入后的数字。
// Number 为需要向上舍入的任意实数。
// Num_digits 舍入后的数字的位数。
Numeric Round(Numeric Number,Integer Num_digits);
// Price 用于求标准差的值，必须是数值型序列值;
// Length 是需要计算的周期数，为整型；
// DataType 求估计方差的类型，1 – 求总体方差，2 – 求样本方差。
Numeric StandardDev(const Series<Numeric>&Price, Integer Length, Integer DataType);
// 返回参数的平方。
// x 需要计算其平方的实数。
Numeric Sqr(Numeric x);
// 返回参数的正平方根。
// x 为需要求平方根的数字，如果该数字为负，则函数Sqrt返回无效值。
Numeric Sqrt(Numeric x);
// 求和
// Price 用于求和的值，必须是数值型序列值;
// Length 是需要计算的周期数，为整型。
// 当序列值的CurrentBar小于Length时，该函数返回无效值。
Numeric Summation(const Series<Numeric>&Price,  Integer Length);
// 快速求和
// Price 用于求和的值，必须是数值型序列值;
// Length 是需要计算的周期数，为整型。
// Series<Numeric> &SumValue 缓存所需临时变量
// 当序列值的CurrentBar小于Length时，该函数返回无效值;
void SummationFC(const Series<Numeric>&Price,  Integer Length, Series<Numeric> &SumValue);
// Numeric Time1 当前时间 只计算小数部分  格式为 xxxxx.hhmmss
// TimeDiff(203.104110, 20060208.104120) = 10
// TimeDiff(20060315.1041, 20060310.1043) = 120
// 两个时间之间的秒数间隔
Integer TimeDiff(Numeric Time1, Numeric Time2);
// 浮点数转字符串函数
String Text(Numeric num);
// 整数转字符串函数
String Text(Integet num);
// Price 用于求估算方差的值，必须是数值型序列值;
// Length 是需要计算的周期数，为整型;
// DataType 求估计方差的类型，1 – 求总体方差，2 – 求样本方差.
Numeric VariancePS(const Series<Numeric>& Price, Integer Length, Integer DataType);
// 与TB原函数不一致
// 求指数平均
// Price 用于求指数平均的值，必须是数值型序列值
// Length 是需要计算的周期数，为整型。
void XAverage(const Series<Numeric>& Price,  Integer Length, Series<Numeric> &XAvgValue);

// 在屏幕上输出字符串
void Commentary(String str);

// 自定义时间函数 判断Time1 == Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeEqual(Numeric Time1, Numeric Time2);

// 自定义时间函数 判断Time1 != Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeUnequal(Numeric Time1, Numeric Time2);

// 自定义时间函数 判断Time1 > Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeLatter(Numeric Time1, Numeric Time2);

// 自定义时间函数 判断Time1 < Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeEarlier(Numeric Time1, Numeric Time2);

// 自定义时间函数 判断Time1 >= Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeLatterOrEqual(Numeric Time1, Numeric Time2);

// 自定义时间函数 判断Time1 <= Time2 若两个时间误差小于500ms 则认为两个时间相等
Bool TimeEarlierOrEqual(Numeric Time1, Numeric Time2);