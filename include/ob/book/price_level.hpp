#pragma once

#include <ob/types/price.hpp>
#include <ob/types/quantity.hpp>

namespace ob::book {

struct OrderNode;

struct PriceLevel {
  ob::types::Price price {};
  ob::types::Quantity total_quantity {};

  OrderNode* head {nullptr};
  OrderNode* tail {nullptr};
};


}
