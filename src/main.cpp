#include <ob/types/order.hpp>
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

}

int main()
{
    const ob::types::Order order{
        .id = 1,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 10
    };

    ob::util::Logger::info("Created order");

    ob::util::Logger::info(
        std::string("order_id=") + std::to_string(order.id) +
        " side=" + to_string(order.side) +
        " price=" + std::to_string(order.price) +
        " quantity=" + std::to_string(order.quantity)
    );

    return 0;
}
