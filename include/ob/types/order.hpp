#pragma once

#include <ob/types/order_id.hpp>
#include <ob/types/price.hpp>
#include <ob/types/quantity.hpp>
#include <ob/types/side.hpp>

namespace ob::types {

struct Order {
    OrderId id {};
    Side side {};
    Price price {};
    Quantity quantity {};
};

}
