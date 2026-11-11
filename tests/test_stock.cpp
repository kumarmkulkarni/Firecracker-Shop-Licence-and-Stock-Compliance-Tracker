/*
 * test_stock.cpp
 *
 * Unit tests for the Stock module (include/stock.h): inventory
 * manipulation, searching, and totals.
 */

extern "C" {
#include "../include/stock.h"
}

#include <gtest/gtest.h>

// Fixture: gives every test a freshly initialised Inventory, so tests
// do not depend on execution order or leak state between each other.
class StockTest : public ::testing::Test {
protected:
    Inventory inv;

    void SetUp() override {
        initInventory(&inv);
    }
};

/* ---------- addStockItem ---------- */

TEST_F(StockTest, AddSingleItemSucceeds) {
    OperationStatus status = addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    EXPECT_EQ(status, OP_SUCCESS);
    EXPECT_EQ(inv.count, 1);
}

TEST_F(StockTest, AddDuplicateBatchFails) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = addStockItem(&inv, CAT_ROCKETS, "B1", 5.0, "01-01-2027");
    EXPECT_EQ(status, OP_ERROR_DUPLICATE);
    EXPECT_EQ(inv.count, 1); // the duplicate must not have been added
}

TEST_F(StockTest, AddToFullInventoryFails) {
    // Boundary case: filling the inventory to MAX_ITEMS, then trying one more.
    char batch[MAX_BATCH_LEN];
    for (int i = 0; i < MAX_ITEMS; i++) {
        snprintf(batch, sizeof(batch), "B%d", i);
        ASSERT_EQ(addStockItem(&inv, CAT_OTHER, batch, 1.0, "01-01-2027"), OP_SUCCESS);
    }
    OperationStatus status = addStockItem(&inv, CAT_OTHER, "OVERFLOW", 1.0, "01-01-2027");
    EXPECT_EQ(status, OP_ERROR_CAPACITY_FULL);
    EXPECT_EQ(inv.count, MAX_ITEMS);
}

/* ---------- findItemById / findItemByBatch ---------- */

TEST_F(StockTest, FindItemByIdReturnsCorrectItem) {
    addStockItem(&inv, CAT_FLOWERPOTS, "B1", 10.0, "01-01-2027");
    StockItem *found = findItemById(&inv, 1);
    ASSERT_NE(found, nullptr);
    EXPECT_STREQ(found->batchNo, "B1");
}

TEST_F(StockTest, FindItemByIdNotFoundReturnsNull) {
    addStockItem(&inv, CAT_FLOWERPOTS, "B1", 10.0, "01-01-2027");
    StockItem *found = findItemById(&inv, 999);
    EXPECT_EQ(found, nullptr);
}

TEST_F(StockTest, FindItemByBatchNotFoundReturnsNull) {
    StockItem *found = findItemByBatch(&inv, "NOPE");
    EXPECT_EQ(found, nullptr);
}

/* ---------- sellStock ---------- */

TEST_F(StockTest, SellValidQuantitySucceeds) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = sellStock(&inv, 1, 4.0);
    EXPECT_EQ(status, OP_SUCCESS);
    EXPECT_DOUBLE_EQ(findItemById(&inv, 1)->quantityKg, 6.0);
}

TEST_F(StockTest, SellExactRemainingQuantitySucceeds) {
    // Boundary case: selling exactly what remains should succeed and
    // leave the item at zero, not be rejected as "insufficient".
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = sellStock(&inv, 1, 10.0);
    EXPECT_EQ(status, OP_SUCCESS);
    EXPECT_DOUBLE_EQ(findItemById(&inv, 1)->quantityKg, 0.0);
}

TEST_F(StockTest, SellMoreThanAvailableFails) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = sellStock(&inv, 1, 10.01);
    EXPECT_EQ(status, OP_ERROR_INSUFFICIENT_STOCK);
}

TEST_F(StockTest, SellFromNonexistentItemFails) {
    OperationStatus status = sellStock(&inv, 42, 5.0);
    EXPECT_EQ(status, OP_ERROR_NOT_FOUND);
}

TEST_F(StockTest, SellNegativeQuantityFails) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = sellStock(&inv, 1, -1.0);
    EXPECT_EQ(status, OP_ERROR_INVALID_INPUT);
}

/* ---------- restockItem ---------- */

TEST_F(StockTest, RestockValidQuantitySucceeds) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = restockItem(&inv, 1, 5.0);
    EXPECT_EQ(status, OP_SUCCESS);
    EXPECT_DOUBLE_EQ(findItemById(&inv, 1)->quantityKg, 15.0);
}

/* ---------- removeItem ---------- */

TEST_F(StockTest, RemoveExistingItemSucceeds) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    OperationStatus status = removeItem(&inv, 1);
    EXPECT_EQ(status, OP_SUCCESS);
    EXPECT_EQ(inv.count, 0);
}

TEST_F(StockTest, RemoveNonexistentItemFails) {
    OperationStatus status = removeItem(&inv, 99);
    EXPECT_EQ(status, OP_ERROR_NOT_FOUND);
}

TEST_F(StockTest, RemoveMiddleItemKeepsOthersIntact) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    addStockItem(&inv, CAT_ROCKETS, "B2", 20.0, "01-01-2027");
    addStockItem(&inv, CAT_BOMBS, "B3", 30.0, "01-01-2027");
    removeItem(&inv, 2); // remove the middle one
    EXPECT_EQ(inv.count, 2);
    EXPECT_NE(findItemById(&inv, 1), nullptr);
    EXPECT_EQ(findItemById(&inv, 2), nullptr);
    EXPECT_NE(findItemById(&inv, 3), nullptr);
}

/* ---------- calculateTotalStock / calculateCategoryStock ---------- */

TEST_F(StockTest, TotalStockOfEmptyInventoryIsZero) {
    // Boundary case: an empty inventory should report 0.0, not crash.
    EXPECT_DOUBLE_EQ(calculateTotalStock(&inv), 0.0);
}

TEST_F(StockTest, TotalStockSumsAllItems) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    addStockItem(&inv, CAT_ROCKETS, "B2", 25.5, "01-01-2027");
    EXPECT_DOUBLE_EQ(calculateTotalStock(&inv), 35.5);
}

TEST_F(StockTest, CategoryStockOnlySumsMatchingCategory) {
    addStockItem(&inv, CAT_SPARKLERS, "B1", 10.0, "01-01-2027");
    addStockItem(&inv, CAT_ROCKETS, "B2", 25.0, "01-01-2027");
    addStockItem(&inv, CAT_SPARKLERS, "B3", 5.0, "01-01-2027");
    EXPECT_DOUBLE_EQ(calculateCategoryStock(&inv, CAT_SPARKLERS), 15.0);
    EXPECT_DOUBLE_EQ(calculateCategoryStock(&inv, CAT_ROCKETS), 25.0);
}
