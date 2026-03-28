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

TEST(OrderBookTest, MultipleBuysSamePriceAggregateQuantity)
{
    ob::book::OrderBook book;

    const ob::types::Order buy_1{
        .id = 1,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 10
    };

    const ob::types::Order buy_2{
        .id = 2,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 5
    };

    const auto trades1 = book.add_order(buy_1);
    const auto trades2 = book.add_order(buy_2);

    EXPECT_TRUE(trades1.empty());
    EXPECT_TRUE(trades2.empty());

    const auto best_bid = book.best_bid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 10125);

    const auto bid_qty = book.bid_quantity_at(10125);
    ASSERT_TRUE(bid_qty.has_value());
    EXPECT_EQ(*bid_qty, 15);

    EXPECT_TRUE(book.has_order(1));
    EXPECT_TRUE(book.has_order(2));

    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(OrderBookTest, BuyMatchesRestingAsksWithPartialFill)
{
    ob::book::OrderBook book;

    const ob::types::Order sell_1{
        .id = 10,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 8
    };

    const ob::types::Order sell_2{
        .id = 11,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 7
    };

    const ob::types::Order aggressive_buy{
        .id = 20,
        .side = ob::types::Side::Buy,
        .price = 10150,
        .quantity = 12
    };

    const auto trades_1 = book.add_order(sell_1);
    const auto trades_2 = book.add_order(sell_2);

    EXPECT_TRUE(trades_1.empty());
    EXPECT_TRUE(trades_2.empty());

    const auto trades_3 = book.add_order(aggressive_buy);

    ASSERT_EQ(trades_3.size(), 2U);

    EXPECT_EQ(trades_3[0].incoming_order_id, 20);
    EXPECT_EQ(trades_3[0].resting_order_id, 10);
    EXPECT_EQ(trades_3[0].price, 10150);
    EXPECT_EQ(trades_3[0].quantity, 8);

    EXPECT_EQ(trades_3[1].incoming_order_id, 20);
    EXPECT_EQ(trades_3[1].resting_order_id, 11);
    EXPECT_EQ(trades_3[1].price, 10150);
    EXPECT_EQ(trades_3[1].quantity, 4);

    EXPECT_FALSE(book.has_order(10));
    EXPECT_TRUE(book.has_order(11));
    EXPECT_FALSE(book.has_order(20));

    const auto ask_qty = book.ask_quantity_at(10150);
    ASSERT_TRUE(ask_qty.has_value());
    EXPECT_EQ(*ask_qty, 3);

    EXPECT_FALSE(book.best_bid().has_value());

    const auto best_ask = book.best_ask();
    ASSERT_TRUE(best_ask.has_value());
    EXPECT_EQ(*best_ask, 10150);
}

TEST(OrderBookTest, AggressiveBuyMatchesRestingAsksWithPartialFill)
{
    ob::book::OrderBook book;

    const ob::types::Order sell_1{
        .id = 10,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 8
    };

    const ob::types::Order sell_2{
        .id = 11,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 7
    };

    const ob::types::Order aggressive_buy{
        .id = 20,
        .side = ob::types::Side::Buy,
        .price = 10150,
        .quantity = 12
    };

    const auto trades_1 = book.add_order(sell_1);
    const auto trades_2 = book.add_order(sell_2);

    EXPECT_TRUE(trades_1.empty());
    EXPECT_TRUE(trades_2.empty());

    const auto trades_3 = book.add_order(aggressive_buy);

    ASSERT_EQ(trades_3.size(), 2U);

    EXPECT_EQ(trades_3[0].incoming_order_id, 20);
    EXPECT_EQ(trades_3[0].resting_order_id, 10);
    EXPECT_EQ(trades_3[0].price, 10150);
    EXPECT_EQ(trades_3[0].quantity, 8);

    EXPECT_EQ(trades_3[1].incoming_order_id, 20);
    EXPECT_EQ(trades_3[1].resting_order_id, 11);
    EXPECT_EQ(trades_3[1].price, 10150);
    EXPECT_EQ(trades_3[1].quantity, 4);

    EXPECT_FALSE(book.has_order(10));
    EXPECT_TRUE(book.has_order(11));
    EXPECT_FALSE(book.has_order(20));

    const auto ask_qty = book.ask_quantity_at(10150);
    ASSERT_TRUE(ask_qty.has_value());
    EXPECT_EQ(*ask_qty, 3);

    EXPECT_FALSE(book.best_bid().has_value());

    const auto best_ask = book.best_ask();
    ASSERT_TRUE(best_ask.has_value());
    EXPECT_EQ(*best_ask, 10150);
}

TEST(OrderBookTest, CancelOnlyOrderRemovesWholePriceLevel)
{
    ob::book::OrderBook book;

    const ob::types::Order buy_order{
        .id = 1,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 10
    };

    book.add_order(buy_order);

    EXPECT_TRUE(book.cancel_order(1));

    EXPECT_FALSE(book.has_order(1));
    EXPECT_FALSE(book.bid_quantity_at(10125).has_value());
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST(OrderBookTest, CancelMissingOrderReturnsFalseAndDoesNotChangeBook)
{
    ob::book::OrderBook book;

    const ob::types::Order buy_order{
        .id = 1,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 10
    };

    book.add_order(buy_order);

    EXPECT_FALSE(book.cancel_order(999));

    EXPECT_TRUE(book.has_order(1));

    const auto best_bid = book.best_bid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 10125);

    const auto bid_qty = book.bid_quantity_at(10125);
    ASSERT_TRUE(bid_qty.has_value());
    EXPECT_EQ(*bid_qty, 10);
}

TEST(OrderBookTest, SellMatchesRestingBidsInFifoOrder)
{
    ob::book::OrderBook book;

    const ob::types::Order buy_1{
        .id = 10,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 8
    };

    const ob::types::Order buy_2{
        .id = 11,
        .side = ob::types::Side::Buy,
        .price = 10125,
        .quantity = 7
    };

    const ob::types::Order aggressive_sell{
        .id = 20,
        .side = ob::types::Side::Sell,
        .price = 10125,
        .quantity = 12
    };

    const auto trades_1 = book.add_order(buy_1);
    const auto trades_2 = book.add_order(buy_2);

    EXPECT_TRUE(trades_1.empty());
    EXPECT_TRUE(trades_2.empty());

    const auto trades_3 = book.add_order(aggressive_sell);

    ASSERT_EQ(trades_3.size(), 2U);

    EXPECT_EQ(trades_3[0].incoming_order_id, 20);
    EXPECT_EQ(trades_3[0].resting_order_id, 10);
    EXPECT_EQ(trades_3[0].price, 10125);
    EXPECT_EQ(trades_3[0].quantity, 8);

    EXPECT_EQ(trades_3[1].incoming_order_id, 20);
    EXPECT_EQ(trades_3[1].resting_order_id, 11);
    EXPECT_EQ(trades_3[1].price, 10125);
    EXPECT_EQ(trades_3[1].quantity, 4);

    EXPECT_FALSE(book.has_order(10));
    EXPECT_TRUE(book.has_order(11));
    EXPECT_FALSE(book.has_order(20));

    const auto bid_qty = book.bid_quantity_at(10125);
    ASSERT_TRUE(bid_qty.has_value());
    EXPECT_EQ(*bid_qty, 3);

    EXPECT_FALSE(book.best_ask().has_value());

    const auto best_bid = book.best_bid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_EQ(*best_bid, 10125);
}
