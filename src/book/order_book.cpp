#include <ob/book/order_book.hpp>

#include <stdexcept>
#include <optional>

#include <ob/types/side.hpp>
#include <ob/types/trade.hpp>
#include <ob/util/logger.hpp>

namespace ob::book {

namespace {

const char* to_string(ob::types::Side side)
{
    switch (side) {
        case ob::types::Side::Buy:
            return "BUY";
        case ob::types::Side::Sell:
            return "SELL";
    }

    return "UNKNOWN";
}

}

OrderBook::~OrderBook()
{
    // NOTE: use a safer approach here
    for (auto& [price, level] : bids_) {
        OrderNode* current = level->head;
        while (current != nullptr) {
            OrderNode* next = current->next;
            delete current;
            current = next;
        }
    }

    for (auto& [price, level] : asks_) {
        OrderNode* current = level->head;
        while (current != nullptr) {
            OrderNode* next = current->next;
            delete current;
            current = next;
        }
    }
}

std::vector<ob::types::Trade>
OrderBook::add_order(const ob::types::Order& order)
{
    if (order_index_.contains(order.id)) {
        throw std::runtime_error("duplicate order id");
    }

    std::vector<ob::types::Trade> trades;

    ob::types::Order incoming = order;

    if (incoming.side == ob::types::Side::Buy) {
        match_buy_order_(incoming, trades);
    } else {
        match_sell_order_(incoming, trades);
    }

    if (incoming.quantity > 0) {
        rest_order_(incoming);
    }

    return trades;
}

bool OrderBook::cancel_order(ob::types::OrderId id)
{
    const auto it = order_index_.find(id);
    if (it == order_index_.end()) {
        return false;
    }

    OrderNode* node = it->second;
    PriceLevel* level = node->parent_level;
    const ob::types::Order order = node->order;

    remove_from_level_(*level, *node);

    order_index_.erase(it);
    delete node;

    erase_level_if_empty_(order);

    return true;
}

PriceLevel* OrderBook::get_or_create_bid_level_(ob::types::Price price)
{
    auto it = bids_.find(price);
    if (it != bids_.end()) {
        return it->second.get();
    }

    auto level = std::make_unique<PriceLevel>();
    level->price = price;

    PriceLevel* raw_ptr = level.get();
    bids_.emplace(price, std::move(level));
    return raw_ptr;
}

PriceLevel* OrderBook::get_or_create_ask_level_(ob::types::Price price)
{
    auto it = asks_.find(price);
    if (it != asks_.end()) {
        return it->second.get();
    }

    auto level = std::make_unique<PriceLevel>();
    level->price = price;

    PriceLevel* raw_ptr = level.get();
    asks_.emplace(price, std::move(level));
    return raw_ptr;
}

void OrderBook::append_to_level_(PriceLevel& level, OrderNode& node)
{
    node.parent_level = &level;
    node.previous = level.tail;
    node.next = nullptr;

    if (level.tail != nullptr) {
        level.tail->next = &node;
    } else {
        level.head = &node;
    }

    level.tail = &node;
    level.total_quantity += node.order.quantity;
}

void OrderBook::remove_from_level_(PriceLevel& level, OrderNode& node)
{
    if (node.previous != nullptr) {
        node.previous->next = node.next;
    } else {
        level.head = node.next;
    }

    if (node.next != nullptr) {
        node.next->previous = node.previous;
    } else {
        level.tail = node.previous;
    }

    level.total_quantity -= node.order.quantity;

    node.previous = nullptr;
    node.next = nullptr;
    node.parent_level = nullptr;
}

void OrderBook::erase_level_if_empty_(const ob::types::Order& order)
{
    if (order.side == ob::types::Side::Buy) {
        const auto it = bids_.find(order.price);
        if (it != bids_.end() && it->second->head == nullptr) {
            bids_.erase(it);
        }
        return;
    }

    const auto it = asks_.find(order.price);
    if (it != asks_.end() && it->second->head == nullptr) {
        asks_.erase(it);
    }
}

std::optional<ob::types::Price> OrderBook::best_bid() const
{
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<ob::types::Price> OrderBook::best_ask() const
{
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

void OrderBook::log_top_of_book() const
{
    auto bid = best_bid();
    auto ask = best_ask();

    std::string msg = "Top of Book -> ";

    msg += "Bid: ";
    msg += bid ? std::to_string(*bid) : "None";

    msg += " | Ask: ";
    msg += ask ? std::to_string(*ask) : "None";

    ob::util::Logger::info(msg);
}

void OrderBook::rest_order_(const ob::types::Order& order)
{
    PriceLevel* level = nullptr;

    if (order.side == ob::types::Side::Buy) {
        level = get_or_create_bid_level_(order.price);
    } else {
        level = get_or_create_ask_level_(order.price);
    }

    OrderNode* node = new OrderNode{
        .order = order,
        .previous = nullptr,
        .next = nullptr,
        .parent_level = level
    };

    append_to_level_(*level, *node);
    order_index_.emplace(order.id, node);
}

void OrderBook::match_buy_order_(ob::types::Order& incoming,
                      std::vector<ob::types::Trade>& trades)
{
    while (incoming.quantity > 0 && !asks_.empty()) {
        auto best_ask_it = asks_.begin();
        PriceLevel& level = *best_ask_it->second;

        if (incoming.price < level.price) {
            break;
        }

        OrderNode* resting = level.head;
        if (resting == nullptr) {
            asks_.erase(best_ask_it);
            continue;
        }

        const auto traded =
            std::min(incoming.quantity, resting->order.quantity);

        trades.push_back(ob::types::Trade{
            .incoming_order_id = incoming.id,
            .resting_order_id = resting->order.id,
            .price = level.price,
            .quantity = traded
        });

        incoming.quantity -= traded;
        resting->order.quantity -= traded;
        level.total_quantity -= traded;

        if (resting->order.quantity == 0) {
            const ob::types::Order filled_order = resting->order;

            remove_from_level_(level, *resting);
            order_index_.erase(filled_order.id);
            delete resting;

            erase_level_if_empty_(filled_order);
        }
    }
}

void OrderBook::match_sell_order_(ob::types::Order& incoming,
                       std::vector<ob::types::Trade>& trades)
{
    while (incoming.quantity > 0 && !bids_.empty()) {
        auto best_bid_it = bids_.begin();
        PriceLevel& level = *best_bid_it->second;

        if (incoming.price > level.price) {
            break;
        }

        OrderNode* resting = level.head;
        if (resting == nullptr) {
            bids_.erase(best_bid_it);
            continue;
        }

        const auto traded =
            std::min(incoming.quantity, resting->order.quantity);

        trades.push_back(ob::types::Trade{
            .incoming_order_id = incoming.id,
            .resting_order_id = resting->order.id,
            .price = level.price,
            .quantity = traded
        });

        incoming.quantity -= traded;
        resting->order.quantity -= traded;
        level.total_quantity -= traded;

        if (resting->order.quantity == 0) {
            const ob::types::Order filled_order = resting->order;

            remove_from_level_(level, *resting);
            order_index_.erase(filled_order.id);
            delete resting;

            erase_level_if_empty_(filled_order);
        }
    }
}

// NOTE: do we need this?
bool OrderBook::has_order(ob::types::OrderId id) const
{
    return order_index_.contains(id);
}

std::optional<ob::types::Quantity> OrderBook::bid_quantity_at(ob::types::Price price) const
{
    const auto it = bids_.find(price);
    if (it == bids_.end()) {
        return std::nullopt;
    }

    return it->second->total_quantity;
}

std::optional<ob::types::Quantity> OrderBook::ask_quantity_at(ob::types::Price price) const
{
    const auto it = asks_.find(price);
    if (it == asks_.end()) {
        return std::nullopt;
    }

    return it->second->total_quantity;
}

void OrderBook::log_trade_(const ob::types::Order& incoming,
                           const ob::types::Order& resting,
                           ob::types::Quantity traded_quantity,
                           ob::types::Price trade_price) const
{
    ob::util::Logger::info(
        std::string("TRADE") +
        " incoming_id=" + std::to_string(incoming.id) +
        " incoming_side=" + to_string(incoming.side) +
        " resting_id=" + std::to_string(resting.id) +
        " resting_side=" + to_string(resting.side) +
        " price=" + std::to_string(trade_price) +
        " qty=" + std::to_string(traded_quantity)
    );
}

}
