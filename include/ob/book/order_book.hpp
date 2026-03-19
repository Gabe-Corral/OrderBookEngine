#pragma once

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <optional>

#include <ob/book/order_node.hpp>
#include <ob/book/price_level.hpp>
#include <ob/types/order.hpp>
#include <ob/types/order_id.hpp>
#include <ob/types/price.hpp>
#include <ob/types/trade.hpp>

namespace ob::book {

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook();

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;

    std::vector<ob::types::Trade> add_order(const ob::types::Order& order);
    bool cancel_order(ob::types::OrderId id);

    std::optional<ob::types::Price> best_bid() const;
    std::optional<ob::types::Price> best_ask() const;

    bool has_order(ob::types::OrderId id) const;

    std::optional<ob::types::Quantity> bid_quantity_at(ob::types::Price price) const;
    std::optional<ob::types::Quantity> ask_quantity_at(ob::types::Price price) const;

    void log_top_of_book() const;

private:
    using BidLevels =
        std::map<
            ob::types::Price,
            std::unique_ptr<PriceLevel>,
            std::greater<>>;

    using AskLevels =
        std::map<
            ob::types::Price,
            std::unique_ptr<PriceLevel>,
            std::less<>>;

    using OrderIndex =
        std::unordered_map<ob::types::OrderId, OrderNode*>;

    PriceLevel* get_or_create_bid_level_(ob::types::Price price);
    PriceLevel* get_or_create_ask_level_(ob::types::Price price);

    void append_to_level_(PriceLevel& level, OrderNode& node);
    void remove_from_level_(PriceLevel& level, OrderNode& node);
    void erase_level_if_empty_(const ob::types::Order& order);

    void match_buy_order_(ob::types::Order& incoming,
                          std::vector<ob::types::Trade>& trades);

    void match_sell_order_(ob::types::Order& incoming,
                           std::vector<ob::types::Trade>& trades);
    void rest_order_(const ob::types::Order& order);

    void log_trade_(const ob::types::Order& incoming,
                    const ob::types::Order& resting,
                    ob::types::Quantity traded_quantity,
                    ob::types::Price trade_price) const;

    BidLevels bids_;
    AskLevels asks_;
    OrderIndex order_index_;
};
}
