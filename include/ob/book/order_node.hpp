#pragma once

#include <ob/types/order.hpp>

namespace ob::book {

struct PriceLevel;

struct OrderNode {
    ob::types::Order order {};

    OrderNode* previous {nullptr};
    OrderNode* next {nullptr};

    PriceLevel* parent_level {nullptr};
};

}
