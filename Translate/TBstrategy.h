#ifndef TBTYPE_H
#include "TBtype.h"
#endif
#ifndef TBSIGNAL_H
#include "TBsignal.h"
#endif
#include "Read_ini.h"
#ifndef BASE_STRATEGY
#define BASE_STRATEGY
#define GLOBALVAR_ARRSIZE 512
#define FORCED_COVER true
#define UNFORCED_COVER false
#define TBSTRATEGY_H
#define VarName(x) #x  // 提取变量名称宏函数
struct StrategyProperty
{   
    // 可否在开仓bar平仓 默认false                
    bool CoverAtOpenBar;  
    // 可否在平仓bar开仓 默认false              
    bool OpenAtCoverBar;  
    // 交易属性
    TradeProperty Attribute; 
    // 策略信息
    StrategyMessage Message;          
};

struct BaseStrategy
{
public:
    // 行情更新 运行策略
    virtual void OnBar() = 0;
    // 新的K线 策略状态参数重置
    virtual void OnNewBar() = 0;
    // 新的K线
    void OnNewBar(bool local);
    // 新的交易日
    void OnNewDay();
    // K线更新 重置序列变量 交易状态信息为上一根bar
    void OnUpdateBar();
    // 设置合约属性
    void SetCodeProperty(const CodeProperty* property);
	// 设置策略属性
	void SetStrategyProperty(const StrategyProperty* property);
    // 注册待发送订单队列
    void RegisterOrderDeque(deque<const StrategyOrder*> &OrderDeque);
    // 注册K线
    void RegisterBars(const BarsData* bars);
	// 模拟运行完毕 进行结算。使用当前最新价格计算策略损益
	void Closing();
    // 获取策略信息
    StrategyMessage* GetMessage();
    // 获取序列变量的最大储存长度 最大储存长度要求大于最大周期的2倍
    // 要求行情缓存长度要大于此长度
    size_t GetMaxSeriesSize() const;
    // 获取品种名称
    string GetSymbol() const;
    // 获取策略名称
    String GetStrategyName() const;
    // 信号控制器
    SignalManager* pSignalManager{nullptr};
protected:
    //-------------------合约属性数据--------------------//
    // 合约代码
    String Symbol;
    // 商品大类 如：Enum_CategoryStocks、Enum_CategoryFutures、Enum_CategoryForexs、Enum_CategoryBonds、
    //             Enum_CategoryFunds、Enum_CategoryOptions、Enum_CategorySpotTrans、Enum_CategoryForeignFutures、
    //             Enum_CategoryForeignStocks
    Integer BigCategory;
    // 商品名称
    String SymbolName;
    // 交易所
    ExchangeType ExchangeID;
    // 交易时段，格式为： 09:00-10:15;10:30-11:30;13:30-14:15;14:30-15:00;
    String DealTimes;
    // 小数点位数
    Integer DecDigits; 
    // 最小变动加价(1/100,1/1000) 报价精度
    Numeric PriceScale; 
    // 变动最小单位(1,20,50) 最小价格变动 = fPriceScale * nMinMove;默认为1
    Integer MinMove;
    // 最小变动价格 = PriceScale * MinMove；
    Numeric PriceTick;
    // 合约单位 期货中1张合约包含N吨铜，小麦等
    Integer ContractUnit;
    // 交易所世界标准时间偏移
    Integer UtcOffset;
    // 保证金比率
    Numeric MarginRate;
    // 保证金计算方式
    Integer MarginMode;
    // 最小委托量
    Integer BaseShares;
    // 一手量
    Integer HandShares;
    // 最小交易增量
    Integer IncrementalShares;
    // 币种，如：Enum_CORMB、Enum_COUSA、Enum_COHK、Enum_COGBP、
    // Enum_COJPY、Enum_COCAD、Enum_COAUD、Enum_COEUR、Enum_COCHF、Enum_COKOR
    Integer CurrencyID;
    // 每个整数点的价值
    Integer BigPointValue;
    // 待绘图变量
    vector<Numeric> PlotDataVec;
    // 待绘图变量名称
    vector<string> PlotNameVec;
    // K线数据1
    const BarsData *Data1 {nullptr};
    //-------------------K线数据--------------------//
    // 开盘价
    const Series<Numeric> *Open {nullptr};
    // 最高价BaseStrategy::
    const Series<Numeric> *High {nullptr};
    // 最低价
    const Series<Numeric> *Low {nullptr};
    // 收盘价
    const Series<Numeric> *Close {nullptr};
    // 成交量
    const Series<Integer> *Vol {nullptr};
    // 持仓量
    const Series<Integer> *OpenInt {nullptr};
    // 当前公式应用商品在当前Bar的时间 如果当前时间为11:34:21.356，Time返回值为0.113421356。
    const Series<Numeric> *Time {nullptr};
    // 当前公式应用商品在当前Bar的日期 如果当前Bar日期为2004-1-24，Date返回值为20040124。
    const Series<Integer> *Date {nullptr};
    // 当前公式交易日
    const Series<Integer> *TrueDate {nullptr};
    // 真实波幅
	const Series<Numeric> *TR {nullptr};
    //-------------------日线数据--------------------//
    // 开盘价
    const Series<Numeric> *OpenD {nullptr};
    // 最高价
    const Series<Numeric> *HighD {nullptr};
    // 最低价
    const Series<Numeric> *LowD {nullptr};
    // 收盘价
    const Series<Numeric> *CloseD {nullptr};
    // 成交量
    const Series<Integer> *VolD {nullptr};
    // 持仓量
    const Series<Integer> *OpenIntD {nullptr};
    // 日期
    const Series<Integer> *DateD {nullptr};
    // 当前时间
	const DateTime *_CurrentTime{nullptr};
    // K线时间
    const DateTime *_BarTime{nullptr};
    //-----------------序列数据汇总----------------//
    // 浮点数序列变量列表
    vector<Series<Numeric>*> _VecSeriesNumeric{};
    // 浮点数序列变量名称列表
    vector<string> _VecSeriesNumericName{};
    // 浮点数序列变量列表
    vector<Series<Integer>*> _VecSeriesInteger{};
    // 浮点数序列变量名称列表
    vector<string> _VecSeriesIntegerName{};
    // 布尔序列变量列表
    vector<Series<Bool>*> _VecSeriesBool{};
    // 布尔序列变量列表
    vector<string> _VecSeriesBoolName{};
    //-----------------函数-----------------------//
    // 获取一个指定的时间点用于debug
    DateTime GetDateTime(unsigned year, unsigned month, unsigned day, unsigned hour, unsigned minute, unsigned second);
    // 集合竞价和小节休息过滤
    Bool CallAuctionFilter();
    // 求平均真实范围
    Numeric AvgTrueRange(Integer Length);
    // 当前公式应用商品数据的Bar总数
    Integer BarCount();
    // 当前Bar的状态值，0表示为第一个Bar，1表示为中间的普通Bar，2表示最后一个Bar。
    Integer BarStatus();
    // 获得当前公式应用所需的最大回溯Bar数。
    Integer MaxBarsBack();
    // 获取某个索引的全局变量值。暂时只支持存储数字，可自己通过0和1来表示bool型的true和false。
    // nIndex 要设置全局变量的索引值，该值从0开始计数，不能大于500。
    Numeric GetGlobalVar(Integer nIndex);
    // 设置全局变量值
    // 设置某个索引的全局变量值。暂时只支持存储数字，可自己通过0和1来表示bool型的true和false。
    // nIndex 要设置全局变量的索引值，该值从0开始计数，不能大于500。
    // fVal 要设置变量的值。 
    void SetGlobalVar(Integer nIndex,Numeric fVal);
    // 该函数计算当前周期的真实高点和真实低点的差值，返回值为浮点数。
    Numeric TrueRange();
    // 求真实低点 该函数计算当前周期的最低值和上一个周期收盘价的较低值，返回值为浮点数。
    Numeric TrueLow();
    // 求真实高点 该函数计算当前周期的最高价和上一个周期收盘价的较高值，返回值为浮点数.
    Numeric TrueHigh();
    // 策略持仓情况
	Integer MarketPosition();
	// 获得当前持仓的第一个建仓位置到当前位置的Bar计数。
	Integer BarsSinceEntry();
	// 获得最近平仓位置到当前位置的Bar计数。
	Integer BarsSinceExit();
	// 获得当前持仓的最后一个建仓位置到当前位置的Bar计数。
	Integer BarsSinceLastEntry();
	// 求当天的第一个数据到当前的Bar数
	Numeric BarsSinceToday();
    // 当前公式应用商品在当前Bar的索引值。
    Integer CurrentBar();
    // 获得当前持仓的第一个建仓价格，返回值为浮点数。
    // 只有当MarketPosition != 0时，即有持仓的状况下，该函数才有意义，否则返回0。 
    // 获得当前持仓的第一个建仓价格
    Numeric EntryPrice();
    // 获得上一次建仓价格，返回值为浮点数。
    Numeric LastEntryPrice();
    // 获得上一次开仓日期
    Integer EntryDate();
    // 获得平均建仓价格
    Numeric AvgEntryPrice();
    // 获得当前持仓位置的持仓盈亏
    Numeric PositionProfit();
    // 获取按当前Bar开盘价计算的可用资金
    Numeric Portfolio_CurrentCapital();
    // 获取当前持仓保证金
    Numeric Portfolio_UsedMargin();
    // 投资组合浮动盈亏
    Numeric Portfolio_PositionProfit(); 
    // 投资组合累计交易盈亏
    Numeric Portfolio_TotalProfit(); 
    // 获取当前Bar日信息 1-31
    Integer Day();
    // 获取当前Bar月份信息 1-12
    Integer Month();
    // 获取当前Bar年份信息 0000-9999
    Integer Year();
    // 获取当前Bar 小时信息 0-23
    Integer Hour();
    // 获取当前Bar 分钟信息 0-59
    Integer Minute();
    // 获取当前Bar 秒信息 0-59
    Integer Second();
    // 买入开仓
    // Integer vol 数量
    // Numeric price 价格
    // int line_id  行号
    void Buy(Integer vol, Numeric price, int line_id);
    // 卖出平仓
    // Integer vol 数量
    // Numeric price 价格
    // int line_id  行号
    // bool force 是否强制平仓 默认false
    void Sell(Integer vol, Numeric price, int line_id, bool force=false);
    // 买入平仓
    // Integer vol 数量
    // Numeric price 价格
    // int line_id  行号
    // bool force 是否强制平仓 默认false
    void BuyToCover(Integer vol, Numeric price, int line_id, bool force=false);
    // 卖出开仓
    // Integer vol 数量
    // Numeric price 价格
    // int line_id  行号
    void SellShort(Integer vol, Numeric price, int line_id);
    // 初始化
    void Initial();
    // 从配置文件里设置序列变量的最大储存长度 最大储存长度要求大于最大周期的2倍
    // 要求行情缓存长度要大于此长度
    void SetMaxSeriesSize(const vector<Integer>& VecSeriesLength);
    // 绘图 在指定的第pos位置输出第pos个值
    void PlotNumeric(Integer pos, Numeric Price);
	//-----------------策略属性--------------------//
	String StrategyName{"UnDefine"};
private:
	// 增加第一根K线
	void AddFirstBar();
    // 增加一根新K线
    void AddNewBar();
    // 增加一根新K线(日线)
    void AddNewDailyBar();
	// 输出交易时间段
	void ShowTradePhaseArr();
    // 为K线 更新实时行情
    // 实时行情tick的最高价最低价为 当前交易日 的最高价 最低价
    void UpdateQuote(const Tick* tick, Bool NewBar, Bool NewDailyBar);
    // 为K线 更新本地行情
    // 本地行情tick的最高价最低价为 单根K线    的最高价 最低价
    void UpdateLocalQuote(const Tick* tick, Bool NewBar, Bool NewDailyBar);
	// 设置当前K线的时间
	void SetCurrentBarTimestamp(const double timestamp);
    // 通过当前K线时间戳设置当前K线时间
    void SetCurrentBarTime();
    // 设置交易时间段
    void SetTradePhaseArr(const String& TradeTimeStr);
	// 设置当前时间
	// const char* timestr 时间字符串 "HH:MM:SS"
	// const int millisecond 毫秒时间 "125"
	void SetCurrentTime(const char* timestr, const int millisecond);
    // 通过开仓信号更新账户交易状态
    void UpdateTradeInfoByOpenSignal(const StrategySignal* signal);
    // 通过平仓信号更新账户交易状态
    void UpdateTradeInfoByCoverSignal(const StrategySignal* signal);
    // 通过平仓信号更新账户交易状态
    void UpdateTradeInfoByTradeRecord(const TradeRecord* signal);
    // 重置非当前K线未平仓开仓单的成交状态为上个根Bar的状态
    void ResetVecSignalTradedVol();
    // 重置策略交易賬戶到初始狀態
    void ResetStrategyTradeState();
    // 订单价格检查 订单价格不得高于当前K线最高价 不得低于当前K线最低价 必须是最小变动价位的整数倍
    Numeric OrderPriceCheck(Numeric price, Bool is_buy);
	// 获得当前执行的公式名称
	String FormulaName();
    // 获取默认订单数量
    Integer GetDefaultVolume();
    // 绘图日志信息标题
    char _PlotDataHead[64];
    // 输出绘图信息
    void DumpPlotData();
    // 输出绘图信息
    void DumpTraceData();
    // 当前交易状态
    TradeState CurrentTradeState;
    // 上一个根Bar交易状态
    TradeState LastTradeState;
    // 求当天的第一个数据到当前的Bar数
	Numeric _BarsSinceToday{0};
    // 当前公式应用商品在当前Bar的索引值。
	Integer _CurrentBar{0};
    // 当前Bar的状态值，0表示为第一个Bar，1表示为中间的普通Bar，2表示最后一个Bar
	Integer _BarStatus{0};
    // K线大小 即K线所包含的秒数
	Integer _BarSize{0};
    // 使用的K线数据数量
    Integer _BarsDataNum{0};   
    // 上一次开仓日期
    Integer _EntryDate{0};  
    // 账户初始资金
    Numeric _InitCapital{1000000};  
    // 交易手续费 按照成交手数收取
    Numeric _TradeFee{0.0};
    // 交易手续费 按照成交金额比例收取
    Numeric _TradeFeeRatio{0.0};
    // 订单交易属性
    TradeProperty _OrderProperty;
    // 策略信息
    StrategyMessage _Message;    
    // 全局变量
    Numeric GlobalVarArr[GLOBALVAR_ARRSIZE];
    // 是否允许在开仓bar平仓
    bool _CoverAtOpenBar{false};
    // 是否允许在平仓bar开仓
    bool _OpenAtCoverBar{false};
    // 是否输出局部变量
    bool _IsTraceMsg{false};
    // 缓存上一个K线时间 供DumpPlotData() DumpTraceData()函数使用
    char _LastBarDateTime[32];
    // 当前K线时间
    const char *_BarTimestr {nullptr};
    // 交易时间
    String _TradeTimeStr;
    // 序列变量的最大储存长度 
    size_t _MaxSeriesSize{512};
    // 用戶策略自定義序列變量的最大周期
    Integer _MaxUserSeriesLength{0};
    // 是否已经重置过交易状态  需舍弃最开始的交易信号
    bool _ResetStrategyTradeState{false};
    // 追踪变量名称数组
    vector<string> _VecTraceMsgName;
};
// 策略构造函数指针
typedef BaseStrategy*(*CreateFun)(const char* ini_file, const char* version);
#else 
#endif