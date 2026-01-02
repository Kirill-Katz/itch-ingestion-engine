#pragma once

#include <cstddef>
#include <stdint.h>
#include <cstring>
#include <string>
#include <stdexcept>

namespace ITCHv1 {

enum class MessageType {
    SYSTEM_EVENT                = 'S',
    STOCK_DIRECTORY             = 'R',
    STOCK_TRADING_ACTION        = 'H',
    REG_SHO                     = 'Y',
    MARKET_PARTICIPANT_POSITION = 'L',
    MWCB_DECLINE_LEVEL_MESSAGE  = 'V',
    MWCB_STATUS_MESSAGE         = 'W',
    IPO_QUOTING_PERIOD_UPD      = 'K',
    LULD_AUCTION_COLLAR         = 'J',
    OPERATIONAL_HALT            = 'h',

    ADD_ORDER_NO_MPID = 'A',
    ADD_ORDER_MPID    = 'F',

    ORDER_EXECUTED       = 'E',
    ORDER_EXECUTED_PRICE = 'C',
    ORDER_CANCEL         = 'X',
    ORDER_DELETE         = 'D',
    ORDER_REPLACE        = 'U',

    NON_CROSS_TRADE_MSG = 'P',
    CROSS_TRADE_MSG     = 'Q',
    BROKEN_TRADE_MSG    = 'B',

    NOII_MSG = 'I',

    DIRECT_LISTING_CAPITAL_RAISE = 'O',
};

#define ITCH_MESSAGE_LIST(X) \
    X(SYSTEM_EVENT,                 SystemEvent,                 system_event) \
    X(STOCK_DIRECTORY,              StockDirectory,              stock_directory) \
    X(STOCK_TRADING_ACTION,         TradingAction,               trading_action) \
    X(REG_SHO,                      RegSho,                      reg_sho) \
    X(MARKET_PARTICIPANT_POSITION,  MarketParticipantPos,        market_participant_pos) \
    X(MWCB_DECLINE_LEVEL_MESSAGE,   MwcbDeclineLevel,            mwcb_decline_level) \
    X(MWCB_STATUS_MESSAGE,          MwcbStatus,                  mwcb_status) \
    X(IPO_QUOTING_PERIOD_UPD,       IpoQuotationPeriodUpd,       ipo_quotation_period_upd) \
    X(LULD_AUCTION_COLLAR,          LuldAuctionCollar,           luld_auction_collar) \
    X(OPERATIONAL_HALT,             OperationalHalt,             operational_halt) \
    X(ADD_ORDER_NO_MPID,            AddOrderNoMpid,              add_order_no_mpid) \
    X(ADD_ORDER_MPID,               AddOrderMpid,                add_order_mpid) \
    X(ORDER_EXECUTED,               OrderExecuted,               order_executed) \
    X(ORDER_EXECUTED_PRICE,         OrderExecutedPrice,          order_executed_price) \
    X(ORDER_CANCEL,                 OrderCancel,                 order_cancel) \
    X(ORDER_DELETE,                 OrderDelete,                 order_delete) \
    X(ORDER_REPLACE,                OrderReplace,                order_replace) \
    X(NON_CROSS_TRADE_MSG,          TradeMessageNonCross,        trade_msg_non_cross) \
    X(CROSS_TRADE_MSG,              TradeMessageCross,           trade_msg_cross) \
    X(BROKEN_TRADE_MSG,             BrokenTrade,                 broken_trade) \
    X(NOII_MSG,                     Noii,                        noii) \
    X(DIRECT_LISTING_CAPITAL_RAISE, DirectListingCapitalRaise,   direct_listing_capital_raise)

struct SystemEvent {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     event_code;
};

struct StockDirectory {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char  stock[8];
    char  market_category;
    char  financial_status_indicator;
    uint32_t round_lot_size;
    char round_lots_only;
    char issue_classification;
    char issue_sub_type[2];
    char authenticity;
    char short_sale_threshold_indicator;
    char ipo_flag;
    char luld_reference_price_tier;
    char etp_flag;
    uint32_t etp_leverage_factor;
    char inverse_indicator;
};

struct TradingAction {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     stock[8];
    char     trading_state;
    char     reserved;
    char     reason[4];
};

struct RegSho {
    uint16_t locate_code;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     stock[8];
    char     reg_sho_action;
};

struct MarketParticipantPos {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     mpid[4];
    char     stock[8];
    char     primary_market_maker;
    char     market_maker_mode;
    char     market_participant_state;
};

struct MwcbDeclineLevel {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t level1;
    uint64_t level2;
    uint64_t level3;
};

struct MwcbStatus {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     breached_level;
};

struct IpoQuotationPeriodUpd {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     stock[8];
    uint32_t ipo_quotation_release_time;
    char     ipo_quatation_release_qualifier;
    uint32_t ipo_price;
};

struct LuldAuctionCollar {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     stock[8];
    uint32_t auction_collar_reference_price;
    uint32_t upper_auction_collar_price;
    uint32_t lower_auction_collar_price;
    uint32_t auction_collar_extension;
};

struct OperationalHalt {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    char     stock[8];
    char     market_code;
    char     operational_halt_action;
};

struct AddOrderNoMpid {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    char     buy_sell;
    uint32_t shares;
    char     stock[8];
    uint32_t price;
};

struct AddOrderMpid {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    char     buy_sell;
    uint32_t shares;
    char     stock[8];
    uint32_t price;
    char     attribution[4];
};

struct OrderExecuted {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    uint32_t executed_shares;
    uint64_t match_number;
};

struct OrderExecutedPrice {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    uint32_t executed_shares;
    uint64_t match_number;
    char     printable;
    uint32_t execution_price;
};

struct OrderCancel {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    uint32_t cancelled_shares;
};

struct OrderDelete {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
};

struct OrderReplace {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    uint64_t new_reference_number;
    uint32_t shares;
    uint32_t price;
};

struct TradeMessageNonCross {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t order_reference_number;
    char     buy_sell;
    uint32_t shares;
    char     stock[8];
    uint32_t price;
    uint64_t match_number;
};

struct TradeMessageCross {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint32_t shares;
    char     stock[8];
    uint32_t cross_price;
    uint64_t match_number;
    char     cross_type;
};

struct BrokenTrade {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;
    uint64_t match_number;
};

struct Noii {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;

    uint64_t paired_shares;
    uint64_t imbalance_shares;
    char     imbalance_direction;

    char     stock[8];
    uint32_t far_price;
    uint32_t near_price;
    uint32_t current_reference_price;
    char     cross_type;
    char     price_variation_indicator;
};

struct DirectListingCapitalRaise {
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t timestamp;

    char     stock[8];
    char     open_eligibility_status;
    uint32_t minimum_allowable_price;
    uint32_t maximum_allowable_price;
    uint32_t near_execution_price;
    uint64_t near_execution_time;
    uint32_t lower_price_range_collar;
    uint32_t upper_price_range_collar;
};

struct Message {
    MessageType type;
    size_t      size;
    union {
        SystemEvent system_event;
        StockDirectory stock_directory;
        TradingAction trading_action;
        RegSho reg_sho;
        MarketParticipantPos market_participant_pos;
        MwcbDeclineLevel mwcb_decline_level;
        MwcbStatus mwcb_status;
        IpoQuotationPeriodUpd ipo_quotation_period_upd;
        LuldAuctionCollar luld_auction_collar;
        OperationalHalt operational_halt;
        AddOrderNoMpid add_order_no_mpid;
        AddOrderMpid add_order_mpid;
        OrderExecuted order_executed;
        OrderExecutedPrice order_executed_price;
        OrderCancel order_cancel;
        OrderDelete order_delete;
        OrderReplace order_replace;
        TradeMessageNonCross trade_msg_non_cross;
        TradeMessageCross trade_msg_cross;
        BrokenTrade broken_trade;
        Noii noii;
        DirectListingCapitalRaise direct_listing_capital_raise;
    };
};

class ItchParser {
public:
    template <typename Handler>
    void parse(std::byte const *  src, size_t len, Handler& handler);

    template <typename SpecificHandler>
    void parse_specific(std::byte const * src, size_t len, SpecificHandler& handler);

    Message parse_msg(std::byte const * src);
};

inline uint64_t load_be48(const std::byte* p) {
    // improved 48 bit be load, 3 instructions with gcc 13.3 vs 16 instructions for shift version
    uint64_t x;
    __builtin_memcpy(&x, p, 8);
    x = __builtin_bswap64(x);
    return x & 0x0000FFFFFFFFFFFFULL;
}

inline uint64_t load_be64(const std::byte* p) {
    return (uint64_t(p[0]) << 56) |
           (uint64_t(p[1]) << 48) |
           (uint64_t(p[2]) << 40) |
           (uint64_t(p[3]) << 32) |
           (uint64_t(p[4]) << 24) |
           (uint64_t(p[5]) << 16) |
           (uint64_t(p[6]) << 8)  |
           uint64_t(p[7]);
}

inline uint32_t load_be32(const std::byte* p) {
    return (uint32_t(p[0]) << 24) |
           (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8)  |
           uint32_t(p[3]);
}

inline uint16_t load_be16(const std::byte* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

inline SystemEvent parseSystemEvent(std::byte const * src) {
    SystemEvent sysEvent;
    sysEvent.stock_locate = load_be16(src);
    src += 2;
    sysEvent.tracking_number = load_be16(src);
    src += 2;
    sysEvent.timestamp = load_be48(src);
    src += 6;
    sysEvent.event_code = static_cast<char>(src[0]);
    src += 1;
    return sysEvent;
}

inline StockDirectory parseStockDirectory(std::byte const * src) {
    StockDirectory stockDir;
    stockDir.stock_locate = load_be16(src);
    src += 2; // offsets and this additions compile to the same thing :(
    stockDir.tracking_number = load_be16(src);
    src += 2;
    stockDir.timestamp = load_be48(src);
    src += 6;
    std::memcpy(stockDir.stock, src, 8);
    src += 8;
    stockDir.market_category = static_cast<char>(src[0]);
    src += 1;
    stockDir.financial_status_indicator = static_cast<char>(src[0]);
    src += 1;
    stockDir.round_lot_size = load_be32(src);
    src += 4;
    stockDir.round_lots_only = static_cast<char>(src[0]);
    src += 1;
    stockDir.issue_classification = static_cast<char>(src[0]);
    src += 1;
    std::memcpy(stockDir.issue_sub_type, src, 2);
    src += 2;
    stockDir.authenticity = static_cast<char>(src[0]);
    src += 1;
    stockDir.short_sale_threshold_indicator = static_cast<char>(src[0]);
    src += 1;
    stockDir.ipo_flag = static_cast<char>(src[0]);
    src += 1;
    stockDir.luld_reference_price_tier = static_cast<char>(src[0]);
    src += 1;
    stockDir.etp_flag = static_cast<char>(src[0]);
    src += 1;
    stockDir.etp_leverage_factor = load_be32(src);
    src += 4;
    stockDir.inverse_indicator = static_cast<char>(src[0]);
    src += 1;

    return stockDir;
}

inline TradingAction parseTradingAction(std::byte const * src) {
    TradingAction tradingAction;

    tradingAction.stock_locate = load_be16(src);
    src += 2;
    tradingAction.tracking_number = load_be16(src);
    src += 2;
    tradingAction.timestamp = load_be48(src);
    src += 6;
    std::memcpy(tradingAction.stock, src, 8);
    src += 8;
    tradingAction.trading_state = static_cast<char>(src[0]);
    src += 1;
    tradingAction.reserved = static_cast<char>(src[0]);
    src += 1;
    std::memcpy(tradingAction.reason, src, 4);
    src += 4;

    return tradingAction;
}

inline RegSho parseRegSho(std::byte const * src) {
    RegSho regSho;

    regSho.locate_code = load_be16(src);
    src += 2;
    regSho.tracking_number = load_be16(src);
    src += 2;
    regSho.timestamp = load_be48(src);
    src += 6;
    std::memcpy(regSho.stock, src, 8);
    src += 8;
    regSho.reg_sho_action = static_cast<char>(src[0]);
    src += 1;

    return regSho;
}

inline MarketParticipantPos parseMarketParticipantPos(std::byte const * src) {
    MarketParticipantPos marketPartPos;

    marketPartPos.stock_locate = load_be16(src);
    src += 2;
    marketPartPos.tracking_number = load_be16(src);
    src += 2;
    marketPartPos.timestamp = load_be48(src);
    src += 6;
    std::memcpy(marketPartPos.mpid, src, 4);
    src += 4;
    std::memcpy(marketPartPos.stock, src, 8);
    src += 8;
    marketPartPos.primary_market_maker = static_cast<char>(src[0]);
    src += 1;
    marketPartPos.market_maker_mode = static_cast<char>(src[0]);
    src += 1;
    marketPartPos.market_participant_state = static_cast<char>(src[0]);
    src += 1;

    return marketPartPos;
}

inline MwcbDeclineLevel parseMwcbDeclineLevel(std::byte const * src) {
    MwcbDeclineLevel mwcbDeclineLevel;

    mwcbDeclineLevel.stock_locate = load_be16(src);
    src += 2;
    mwcbDeclineLevel.tracking_number = load_be16(src);
    src += 2;
    mwcbDeclineLevel.timestamp = load_be48(src);
    src += 6;

    mwcbDeclineLevel.level1 = load_be64(src);
    src += 8;
    mwcbDeclineLevel.level2 = load_be64(src);
    src += 8;
    mwcbDeclineLevel.level3 = load_be64(src);
    src += 8;

    return mwcbDeclineLevel;
}

inline MwcbStatus parseMwcbStatus(std::byte const * src) {
    MwcbStatus mwcbStatus;

    mwcbStatus.stock_locate = load_be16(src);
    src += 2;
    mwcbStatus.tracking_number = load_be16(src);
    src += 2;
    mwcbStatus.timestamp = load_be48(src);
    src += 6;
    mwcbStatus.breached_level = static_cast<char>(src[0]);
    src += 1;

    return mwcbStatus;
}

inline IpoQuotationPeriodUpd parseIpoQuotationPeriodUpd(std::byte const * src) {
    IpoQuotationPeriodUpd ipoQuotingPerUpd;

    ipoQuotingPerUpd.stock_locate = load_be16(src);
    src += 2;
    ipoQuotingPerUpd.tracking_number = load_be16(src);
    src += 2;
    ipoQuotingPerUpd.timestamp = load_be48(src);
    src += 6;
    std::memcpy(ipoQuotingPerUpd.stock, src, 8);
    src += 8;
    ipoQuotingPerUpd.ipo_quotation_release_time = load_be32(src);
    src += 4;
    ipoQuotingPerUpd.ipo_quatation_release_qualifier = static_cast<char>(src[0]);
    src += 1;
    ipoQuotingPerUpd.ipo_price = load_be32(src);
    src += 4;

    return ipoQuotingPerUpd;
};

inline LuldAuctionCollar parseLuldAuctionCollar(std::byte const * src) {
    LuldAuctionCollar luldActionCollar;

    luldActionCollar.stock_locate = load_be16(src);
    src += 2;
    luldActionCollar.tracking_number = load_be16(src);
    src += 2;
    luldActionCollar.timestamp = load_be48(src);
    src += 6;
    std::memcpy(luldActionCollar.stock, src, 8);
    src += 8;
    luldActionCollar.auction_collar_reference_price = load_be32(src);
    src += 4;
    luldActionCollar.upper_auction_collar_price = load_be32(src);
    src += 4;
    luldActionCollar.lower_auction_collar_price = load_be32(src);
    src += 4;
    luldActionCollar.auction_collar_extension = load_be32(src);
    src += 4;

    return luldActionCollar;
}

inline OperationalHalt parseOperationalHalt(std::byte const * src) {
    OperationalHalt operationalHalt;

    operationalHalt.stock_locate = load_be16(src);
    src += 2;
    operationalHalt.tracking_number = load_be16(src);
    src += 2;
    operationalHalt.timestamp = load_be48(src);
    src += 6;
    std::memcpy(operationalHalt.stock, src, 8);
    src += 8;
    operationalHalt.market_code = static_cast<char>(src[0]);
    src += 1;
    operationalHalt.operational_halt_action = static_cast<char>(src[0]);
    src += 1;

    return operationalHalt;
}

inline AddOrderNoMpid parseAddOrderNoMpid(std::byte const * src) {
    AddOrderNoMpid addOrderNoMpid;

    addOrderNoMpid.stock_locate = load_be16(src);
    src += 2;
    addOrderNoMpid.tracking_number = load_be16(src);
    src += 2;
    addOrderNoMpid.timestamp = load_be48(src);
    src += 6;
    addOrderNoMpid.order_reference_number = load_be64(src);
    src += 8;
    addOrderNoMpid.buy_sell = static_cast<char>(src[0]);
    src += 1;
    addOrderNoMpid.shares = load_be32(src);
    src += 4;
    std::memcpy(addOrderNoMpid.stock, src, 8);
    src += 8;
    addOrderNoMpid.price = load_be32(src);
    src += 4;

    return addOrderNoMpid;
}

inline AddOrderMpid parseAddOrderMpid(std::byte const * src) {
    AddOrderMpid addOrderMpid;

    addOrderMpid.stock_locate = load_be16(src);
    src += 2;
    addOrderMpid.tracking_number = load_be16(src);
    src += 2;
    addOrderMpid.timestamp = load_be48(src);
    src += 6;
    addOrderMpid.order_reference_number = load_be64(src);
    src += 8;
    addOrderMpid.buy_sell = static_cast<char>(src[0]);
    src += 1;
    addOrderMpid.shares = load_be32(src);
    src += 4;
    std::memcpy(addOrderMpid.stock, src, 8);
    src += 8;
    addOrderMpid.price = load_be32(src);
    src += 4;
    std::memcpy(addOrderMpid.attribution, src, 4);
    src += 4;

    return addOrderMpid;
}

inline OrderExecuted parseOrderExecuted(std::byte const * src) {
    OrderExecuted orderExecuted;

    orderExecuted.stock_locate = load_be16(src);
    src += 2;
    orderExecuted.tracking_number = load_be16(src);
    src += 2;
    orderExecuted.timestamp = load_be48(src);
    src += 6;
    orderExecuted.order_reference_number = load_be64(src);
    src += 8;
    orderExecuted.executed_shares = load_be32(src);
    src += 4;
    orderExecuted.match_number = load_be64(src);
    src += 8;

    return orderExecuted;
}

inline OrderExecutedPrice parseOrderExecutedPrice(std::byte const * src) {
    OrderExecutedPrice orderExecutedPrice;

    orderExecutedPrice.stock_locate = load_be16(src);
    src += 2;
    orderExecutedPrice.tracking_number = load_be16(src);
    src += 2;
    orderExecutedPrice.timestamp = load_be48(src);
    src += 6;
    orderExecutedPrice.order_reference_number = load_be64(src);
    src += 8;
    orderExecutedPrice.executed_shares = load_be32(src);
    src += 4;
    orderExecutedPrice.match_number = load_be64(src);
    src += 8;
    orderExecutedPrice.printable = static_cast<char>(src[0]);
    src += 1;
    orderExecutedPrice.execution_price = load_be32(src);
    src += 4;

    return orderExecutedPrice;
}

inline OrderCancel parseOrderCancel(std::byte const * src) {
    OrderCancel orderCancel;

    orderCancel.stock_locate = load_be16(src);
    src += 2;
    orderCancel.tracking_number = load_be16(src);
    src += 2;
    orderCancel.timestamp = load_be48(src);
    src += 6;
    orderCancel.order_reference_number = load_be64(src);
    src += 8;
    orderCancel.cancelled_shares = load_be32(src);
    src += 4;

    return orderCancel;
}

inline OrderDelete parseOrderDelete(std::byte const * src) {
    OrderDelete orderDelete;

    orderDelete.stock_locate = load_be16(src);
    src += 2;
    orderDelete.tracking_number = load_be16(src);
    src += 2;
    orderDelete.timestamp = load_be48(src);
    src += 6;
    orderDelete.order_reference_number = load_be64(src);
    src += 8;

    return orderDelete;
}

inline OrderReplace parseOrderReplace(std::byte const * src) {
    OrderReplace orderReplace;

    orderReplace.stock_locate = load_be16(src);
    src += 2;
    orderReplace.tracking_number = load_be16(src);
    src += 2;
    orderReplace.timestamp = load_be48(src);
    src += 6;
    orderReplace.order_reference_number = load_be64(src);
    src += 8;
    orderReplace.new_reference_number = load_be64(src);
    src += 8;
    orderReplace.shares = load_be32(src);
    src += 4;
    orderReplace.price = load_be32(src);
    src += 4;

    return orderReplace;
}

inline TradeMessageNonCross parseTradeMessageNonCross(std::byte const * src) {
    TradeMessageNonCross tradeMessageNonCross;

    tradeMessageNonCross.stock_locate = load_be16(src);
    src += 2;
    tradeMessageNonCross.tracking_number = load_be16(src);
    src += 2;
    tradeMessageNonCross.timestamp = load_be48(src);
    src += 6;
    tradeMessageNonCross.order_reference_number = load_be64(src);
    src += 8;
    tradeMessageNonCross.buy_sell = static_cast<char>(src[0]);
    src += 1;
    tradeMessageNonCross.shares = load_be32(src);
    src += 4;
    std::memcpy(tradeMessageNonCross.stock, src, 8);
    src += 8;
    tradeMessageNonCross.price = load_be32(src);
    src += 4;
    tradeMessageNonCross.match_number = load_be64(src);
    src += 8;

    return tradeMessageNonCross;
}

inline TradeMessageCross parseTradeMessageCross(std::byte const * src) {
    TradeMessageCross tradeMessageCross;

    tradeMessageCross.stock_locate = load_be16(src);
    src += 2;
    tradeMessageCross.tracking_number = load_be16(src);
    src += 2;
    tradeMessageCross.timestamp = load_be48(src);
    src += 6;
    tradeMessageCross.shares = load_be32(src);
    src += 4;
    std::memcpy(tradeMessageCross.stock, src, 8);
    src += 8;
    tradeMessageCross.cross_price = load_be32(src);
    src += 4;
    tradeMessageCross.match_number = load_be64(src);
    src += 8;
    tradeMessageCross.cross_type = static_cast<char>(src[0]);
    src += 1;

    return tradeMessageCross;
}

inline BrokenTrade parseBrokenTrade(std::byte const * src) {
    BrokenTrade brokenTrade;

    brokenTrade.stock_locate = load_be16(src);
    src += 2;
    brokenTrade.tracking_number = load_be16(src);
    src += 2;
    brokenTrade.timestamp = load_be48(src);
    src += 6;
    brokenTrade.match_number = load_be64(src);
    src += 8;

    return brokenTrade;
}

inline Noii parseNoii(std::byte const * src) {
    Noii noii;

    noii.stock_locate = load_be16(src);
    src += 2;
    noii.tracking_number = load_be16(src);
    src += 2;
    noii.timestamp = load_be48(src);
    src += 6;
    noii.paired_shares = load_be64(src);
    src += 8;
    noii.imbalance_shares = load_be64(src);
    src += 8;
    noii.imbalance_direction = static_cast<char>(src[0]);
    src += 1;
    std::memcpy(noii.stock, src, 8);
    src += 8;
    noii.far_price = load_be32(src);
    src += 4;
    noii.near_price = load_be32(src);
    src += 4;
    noii.current_reference_price = load_be32(src);
    src += 4;
    noii.cross_type = static_cast<char>(src[0]);
    src += 1;
    noii.price_variation_indicator = static_cast<char>(src[0]);
    src += 1;

    return noii;
}

inline DirectListingCapitalRaise parseDirectListingCapitalRaise(std::byte const * src) {
    DirectListingCapitalRaise directListingCapitalRaise;

    directListingCapitalRaise.stock_locate = load_be16(src);
    src += 2;
    directListingCapitalRaise.tracking_number = load_be16(src);
    src += 2;
    directListingCapitalRaise.timestamp = load_be48(src);
    src += 6;
    std::memcpy(directListingCapitalRaise.stock, src, 8);
    src += 8;
    directListingCapitalRaise.open_eligibility_status = static_cast<char>(src[0]);
    src += 1;
    directListingCapitalRaise.minimum_allowable_price = load_be32(src);
    src += 4;
    directListingCapitalRaise.maximum_allowable_price = load_be32(src);
    src += 4;
    directListingCapitalRaise.near_execution_price = load_be32(src);
    src += 4;
    directListingCapitalRaise.near_execution_time = load_be64(src);
    src += 8;
    directListingCapitalRaise.lower_price_range_collar = load_be32(src);
    src += 4;
    directListingCapitalRaise.upper_price_range_collar = load_be32(src);
    src += 4;

    return directListingCapitalRaise;
}

[[gnu::noinline, gnu::cold]] static void bad_type(char t) {
    throw std::runtime_error("Unknown message type: " + std::to_string(t));
}

inline Message ItchParser::parse_msg(std::byte const * src) {
    uint16_t size = load_be16(src);
    src += 2;

    auto raw_type = char(src[0]);
    MessageType type = static_cast<MessageType>(raw_type);
    src += 1;

    Message msg{};
    msg.type = type;
    msg.size = size;

    switch (type) {
    #define X(NAME, TYPE, FIELD) \
        case MessageType::NAME: { \
            TYPE m = parse##TYPE(src); \
            msg.FIELD = m; \
            asm volatile("" : : "r,m"(msg.FIELD)); \
            return msg; \
        }
        ITCH_MESSAGE_LIST(X)

        default:
            bad_type(raw_type);
    #undef X
    }

    return msg;
}

template<typename SpecificHandler>
void ItchParser::parse_specific(std::byte const * src, size_t len, SpecificHandler& handler) {
    std::byte const * end = src + len;

    while (end - src >= 3) {
        uint16_t size = load_be16(src);
        if (end - src < 2 + size) {
            break;
        }
        src += 2;

        auto raw_type = char(src[0]);
        MessageType type = static_cast<MessageType>(raw_type);
        src += 1;

        switch (type) {
        #define X(NAME, TYPE, FIELD) \
            case MessageType::NAME: { \
                auto m = parse##TYPE(src); \
                handler.handle_##FIELD(m); \
                handler.handle_after(); \
                break; \
            }

            ITCH_MESSAGE_LIST(X)

            default:
                bad_type(raw_type);
        #undef X
        }

        src += size - 1;
    }
}

template <typename Handler>
void ItchParser::parse(std::byte const *  src, size_t len, Handler& handler) {
    std::byte const * end = src + len;

    while (end - src >= 3) {
        uint16_t size = load_be16(src);
        if (end - src < 2 + size) {
            break;
        }

        Message msg = parse_msg(src);
        handler.handle(msg);

        src += msg.size + 2;
    }
}

}
