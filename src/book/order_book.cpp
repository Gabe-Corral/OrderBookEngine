#include <ob/book/order_book.hpp>

#include <stdexcept>

#include <ob/types/side.hpp>

namespace ob::book {

OrderBook::~OrderBook()
{
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

void OrderBook::add_order(const ob::types::Order& order)
{
    if (order_index_.contains(order.id)) {
        throw std::runtime_error("duplicate order id");
    }

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

}
