#include <gtest/gtest.h>

#include <ob/book/order_book.hpp>
#include <ob/types/order.hpp>
#include <ob/types/side.hpp>

TEST(OrderBookTest, AddSingleRestingBuyOrder)
{
    ob::book::OrderBook book;

    const ob::types::Order buy_order{
        .id = 1,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 10
    };

    const auto trades = book.add_order(buy_order);

    EXPECT_TRUE(trades.empty());

    const auto best_bid = book.best_bid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 10125);

    const auto best_ask = book.best_ask();
    EXPECT_FALSE(best_ask.has_value());

    EXPECT_TRUE(book.has_order(1));

    const auto bid_qty = book.bid_quantity_at(10125);
    ASSERT_TRUE(bid_qty.has_value());
    EXPECT_EQ(*bid_qty, 10);

    const auto missing_bid_qty = book.bid_quantity_at(10100);
    EXPECT_FALSE(missing_bid_qty.has_value());

    const auto ask_qty = book.ask_quantity_at(10125);
    EXPECT_FALSE(ask_qty.has_value());
}
