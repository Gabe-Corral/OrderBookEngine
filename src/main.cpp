#include <ob/book/order_book.hpp>
#include <ob/types/order.hpp>
#include <ob/types/side.hpp>
#include <ob/util/logger.hpp>

#include <string>

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

void log_order(const ob::types::Order& order, std::string_view prefix)
{
    ob::util::Logger::info(
        std::string(prefix) +
        " id=" + std::to_string(order.id) +
        " side=" + to_string(order.side) +
        " price=" + std::to_string(order.price) +
        " qty=" + std::to_string(order.quantity)
    );
}


} // namespace

int main()
{
    ob::book::OrderBook book;

    const ob::types::Order sell_1{
        .id = 1,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 10
    };

    const ob::types::Order sell_2{
        .id = 2,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 5
    };

    const ob::types::Order buy_1{
        .id = 3,
        .side = ob::types::Side::Buy,
        .price = 10150,
        .quantity = 12
    };

    log_order(sell_1, "Add:");
    book.add_order(sell_1);
    book.log_top_of_book();

    log_order(sell_2, "Add:");
    book.add_order(sell_2);
    book.log_top_of_book();

    log_order(buy_1, "Add:");
    book.add_order(buy_1);
    book.log_top_of_book();

    return 0;
}
