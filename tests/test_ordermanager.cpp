#include <gtest/gtest.h>
#include "OrderManager.hpp"

// Use the namespace to avoid typing TradingEngine:: everywhere
using namespace TradingEngine;

// Test fixture for OrderManager tests
class OrderManagerTest : public ::testing::Test {
protected:
    OrderManager om;
};

// Test case for a simple full buy order
TEST_F(OrderManagerTest, HandlesSingleFullBuyOrder) {
    // 1. Create a new order
    Order buy_order;
    buy_order.symbol = "AAPL";
    buy_order.side = Side::BUY;
    buy_order.quantity = 100;
    buy_order.price = 150.0;
    buy_order.order_type = OrderType::LIMIT;

    // 2. Add it to the OrderManager
    uint64_t order_id = om.add_new_order(buy_order);
    ASSERT_GT(order_id, 0); // Ensure we got a valid ID back

    // 3. Create a fake execution report for a full fill
    ExecutionReport report;
    report.order_id = order_id;
    report.symbol = "AAPL";
    report.new_status = OrderStatus::FILLED;
    report.fill_quantity = 100;
    report.fill_price = 149.95;

    // 4. Update the OrderManager with the report
    om.update_order_status(report);

    // 5. Assert the final state is correct
    ASSERT_EQ(om.get_position("AAPL"), 100.0);
    
    Order final_order_state = om.get_order(order_id);
    ASSERT_EQ(final_order_state.status, OrderStatus::FILLED);
    ASSERT_EQ(final_order_state.filled_quantity, 100.0);
    ASSERT_DOUBLE_EQ(final_order_state.avg_fill_price, 149.95);
}

// Test case for partial fills
TEST_F(OrderManagerTest, HandlesPartialFills) {
    Order buy_order;
    buy_order.symbol = "MSFT";
    buy_order.side = Side::BUY;
    buy_order.quantity = 200;
    uint64_t order_id = om.add_new_order(buy_order);

    // First partial fill
    ExecutionReport report1;
    report1.order_id = order_id;
    report1.new_status = OrderStatus::PARTIALLY_FILLED;
    report1.fill_quantity = 50;
    report1.fill_price = 300.0;
    om.update_order_status(report1);

    // Assert state after first fill
    ASSERT_EQ(om.get_position("MSFT"), 50.0);
    Order order_state1 = om.get_order(order_id);
    ASSERT_EQ(order_state1.status, OrderStatus::PARTIALLY_FILLED);
    ASSERT_EQ(order_state1.filled_quantity, 50.0);
    ASSERT_DOUBLE_EQ(order_state1.avg_fill_price, 300.0);

    // Second partial fill
    ExecutionReport report2;
    report2.order_id = order_id;
    report2.new_status = OrderStatus::FILLED; // Now it's fully filled
    report2.fill_quantity = 150;
    report2.fill_price = 301.0;
    om.update_order_status(report2);

    // Assert final state
    ASSERT_EQ(om.get_position("MSFT"), 200.0);
    Order final_order_state = om.get_order(order_id);
    ASSERT_EQ(final_order_state.status, OrderStatus::FILLED);
    ASSERT_EQ(final_order_state.filled_quantity, 200.0);
    // Expected avg price: (50 * 300 + 150 * 301) / 200 = 300.75
    ASSERT_DOUBLE_EQ(final_order_state.avg_fill_price, 300.75);
}
