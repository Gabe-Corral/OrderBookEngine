#pragma once

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

#include <ob/book/order_node.hpp>
#include <ob/book/price_level.hpp>
#include <ob/types/order.hpp>
#include <ob/types/order_id.hpp>
#include <ob/types/price.hpp>

namespace ob::book {

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook();

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;

    void add_order(const ob::types::Order& order);
    bool cancel_order(ob::types::OrderId id);

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

    BidLevels bids_;
    AskLevels asks_;
    OrderIndex order_index_;
};
}
