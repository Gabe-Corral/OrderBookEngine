#include <string>
#include <ob/util/logger.hpp>
#include <ob/types/order.hpp>

using namespace ob::types;
using ob::util::Logger;

int main()
{
    Order order{
        .id = 1,
        .side = Side::Buy,
        .price = 125,
        .quantity = 10
    };

    const char* side_str = order.side == Side::Buy ? "BUY" : "SELL";

    Logger::info(
        "Order details\n id: " +
        std::to_string(order.id) +
        " side: " + side_str +
        " price: " +
        std::to_string(order.price) +
        " qty: " +
        std::to_string(order.quantity)
    );

    return 0;
}
