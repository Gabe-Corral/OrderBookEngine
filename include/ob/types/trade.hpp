#pragma once

#include <ob/types/order_id.hpp>
#include <ob/types/price.hpp>
#include <ob/types/quantity.hpp>

namespace ob::types {

struct Trade {
    OrderId incoming_order_id {};
    OrderId resting_order_id {};
    Price price {};
    Quantity quantity {};
};

}
