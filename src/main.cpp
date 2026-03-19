#include <ob/book/order_book.hpp>
#include <ob/types/order.hpp>
#include <ob/types/order_id.hpp>
#include <ob/types/price.hpp>
#include <ob/types/side.hpp>
#include <ob/types/trade.hpp>
#include <ob/util/logger.hpp>

#include <cstdio>
#include <vector>

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

void log_order(const ob::types::Order& order, const char* prefix)
{
    char buffer[160]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s id=%llu side=%s price=%lld qty=%u",
        prefix,
        static_cast<unsigned long long>(order.id),
        to_string(order.side),
        static_cast<long long>(order.price),
        static_cast<unsigned int>(order.quantity)
    );

    ob::util::Logger::info(buffer);
}

void log_trade(const ob::types::Trade& trade)
{
    char buffer[192]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "TRADE incoming_id=%llu resting_id=%llu price=%lld qty=%u",
        static_cast<unsigned long long>(trade.incoming_order_id),
        static_cast<unsigned long long>(trade.resting_order_id),
        static_cast<long long>(trade.price),
        static_cast<unsigned int>(trade.quantity)
    );

    ob::util::Logger::info(buffer);
}

void log_trades(const std::vector<ob::types::Trade>& trades)
{
    if (trades.empty()) {
        ob::util::Logger::info("No trades generated");
        return;
    }

    for (const auto& trade : trades) {
        log_trade(trade);
    }
}

void log_cancel_result(ob::types::OrderId id, bool cancelled)
{
    char buffer[128]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "CANCEL id=%llu result=%s",
        static_cast<unsigned long long>(id),
        cancelled ? "SUCCESS" : "FAIL"
    );

    ob::util::Logger::info(buffer);
}

void log_top_of_book(const ob::book::OrderBook& book)
{
    const auto best_bid = book.best_bid();
    const auto best_ask = book.best_ask();

    char bid_buf[32]{};
    char ask_buf[32]{};

    if (best_bid) {
        std::snprintf(
            bid_buf,
            sizeof(bid_buf),
            "%lld",
            static_cast<long long>(*best_bid)
        );
    } else {
        std::snprintf(bid_buf, sizeof(bid_buf), "NONE");
    }

    if (best_ask) {
        std::snprintf(
            ask_buf,
            sizeof(ask_buf),
            "%lld",
            static_cast<long long>(*best_ask)
        );
    } else {
        std::snprintf(ask_buf, sizeof(ask_buf), "NONE");
    }

    char buffer[128]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "TOP bid=%s ask=%s",
        bid_buf,
        ask_buf
    );

    ob::util::Logger::info(buffer);
}

void log_has_order(const ob::book::OrderBook& book, ob::types::OrderId id)
{
    char buffer[128]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "HAS_ORDER id=%llu present=%s",
        static_cast<unsigned long long>(id),
        book.has_order(id) ? "true" : "false"
    );

    ob::util::Logger::info(buffer);
}

void log_bid_quantity_at(const ob::book::OrderBook& book, ob::types::Price price)
{
    const auto qty = book.bid_quantity_at(price);

    char qty_buf[32]{};
    if (qty) {
        std::snprintf(
            qty_buf,
            sizeof(qty_buf),
            "%u",
            static_cast<unsigned int>(*qty)
        );
    } else {
        std::snprintf(qty_buf, sizeof(qty_buf), "NONE");
    }

    char buffer[128]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "BID_QTY price=%lld qty=%s",
        static_cast<long long>(price),
        qty_buf
    );

    ob::util::Logger::info(buffer);
}

void log_ask_quantity_at(const ob::book::OrderBook& book, ob::types::Price price)
{
    const auto qty = book.ask_quantity_at(price);

    char qty_buf[32]{};
    if (qty) {
        std::snprintf(
            qty_buf,
            sizeof(qty_buf),
            "%u",
            static_cast<unsigned int>(*qty)
        );
    } else {
        std::snprintf(qty_buf, sizeof(qty_buf), "NONE");
    }

    char buffer[128]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "ASK_QTY price=%lld qty=%s",
        static_cast<long long>(price),
        qty_buf
    );

    ob::util::Logger::info(buffer);
}

void log_book_snapshot(const ob::book::OrderBook& book)
{
    log_top_of_book(book);

    log_bid_quantity_at(book, 10150);
    log_bid_quantity_at(book, 10125);
    log_bid_quantity_at(book, 10100);

    log_ask_quantity_at(book, 10150);
    log_ask_quantity_at(book, 10175);
}

void add_and_log(ob::book::OrderBook& book, const ob::types::Order& order)
{
    log_order(order, "ADD");
    const auto trades = book.add_order(order);
    log_trades(trades);
    log_book_snapshot(book);
}

} // namespace

int main()
{
    ob::util::Logger::info("Starting order book scenario harness");

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

    const ob::types::Order sell_1{
        .id = 3,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 8
    };

    const ob::types::Order sell_2{
        .id = 4,
        .side = ob::types::Side::Sell,
        .price = 10150,
        .quantity = 7
    };

    const ob::types::Order aggressive_buy{
        .id = 5,
        .side = ob::types::Side::Buy,
        .price = 10150,
        .quantity = 12
    };

    ob::util::Logger::info("Scenario 1: add first resting bid");
    add_and_log(book, buy_1);
    log_has_order(book, 1);

    ob::util::Logger::info("Scenario 2: add second bid at same price");
    add_and_log(book, buy_2);
    log_has_order(book, 2);

    ob::util::Logger::info("Scenario 3: add resting ask");
    add_and_log(book, sell_1);
    log_has_order(book, 3);

    ob::util::Logger::info("Scenario 4: add second ask at same price");
    add_and_log(book, sell_2);
    log_has_order(book, 4);

    ob::util::Logger::info("Scenario 5: aggressive buy matches asks");
    add_and_log(book, aggressive_buy);
    log_has_order(book, 3);
    log_has_order(book, 4);
    log_has_order(book, 5);

    ob::util::Logger::info("Scenario 6: cancel remaining resting bid");
    log_cancel_result(1, book.cancel_order(1));
    log_book_snapshot(book);
    log_has_order(book, 1);

    ob::util::Logger::info("Scenario 7: cancel second resting bid");
    log_cancel_result(2, book.cancel_order(2));
    log_book_snapshot(book);
    log_has_order(book, 2);

    ob::util::Logger::info("Scenario 8: cancel already removed order");
    log_cancel_result(999, book.cancel_order(999));
    log_book_snapshot(book);

    ob::util::Logger::info("Scenario harness complete");

    return 0;
}
