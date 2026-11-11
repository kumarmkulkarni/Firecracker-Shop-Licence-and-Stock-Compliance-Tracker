/*
 * test_compliance.cpp
 *
 * Unit tests for the Compliance module (include/compliance.h), the
 * project's core decision-making logic.
 */

extern "C" {
#include "../include/compliance.h"
}

#include <gtest/gtest.h>

/* ---------- determineStockStatus ---------- */

TEST(ComplianceTest, WellBelowLimitIsSafe) {
    EXPECT_EQ(determineStockStatus(50.0, 100.0), STATUS_SAFE);
}

TEST(ComplianceTest, ExactlyAtNearLimitRatioIsNearLimit) {
    // Boundary case: exactly at NEAR_LIMIT_RATIO (0.90) of the limit.
    EXPECT_EQ(determineStockStatus(90.0, 100.0), STATUS_NEAR_LIMIT);
}

TEST(ComplianceTest, JustBelowNearLimitRatioIsSafe) {
    // Boundary case: just under the near-limit threshold.
    EXPECT_EQ(determineStockStatus(89.99, 100.0), STATUS_SAFE);
}

TEST(ComplianceTest, ExactlyAtLimitIsNearLimitNotExceeded) {
    // Boundary case: exactly at the licensed limit should not count as
    // exceeding it (only going OVER the limit does).
    EXPECT_EQ(determineStockStatus(100.0, 100.0), STATUS_NEAR_LIMIT);
}

TEST(ComplianceTest, JustOverLimitIsExceeded) {
    // Boundary case: the smallest amount over the limit.
    EXPECT_EQ(determineStockStatus(100.01, 100.0), STATUS_EXCEEDED);
}

TEST(ComplianceTest, WellOverLimitIsExceeded) {
    EXPECT_EQ(determineStockStatus(150.0, 100.0), STATUS_EXCEEDED);
}

TEST(ComplianceTest, ZeroStockIsSafe) {
    EXPECT_EQ(determineStockStatus(0.0, 100.0), STATUS_SAFE);
}

/* ---------- willExceedLimit ---------- */

TEST(ComplianceTest, AdditionWithinLimitDoesNotExceed) {
    EXPECT_FALSE(willExceedLimit(80.0, 10.0, 100.0));
}

TEST(ComplianceTest, AdditionExactlyToLimitDoesNotExceed) {
    // Boundary case: landing exactly on the limit is allowed.
    EXPECT_FALSE(willExceedLimit(90.0, 10.0, 100.0));
}

TEST(ComplianceTest, AdditionPastLimitExceeds) {
    EXPECT_TRUE(willExceedLimit(95.0, 10.0, 100.0));
}

/* ---------- remainingCapacityKg ---------- */

TEST(ComplianceTest, RemainingCapacityNormalCase) {
    EXPECT_DOUBLE_EQ(remainingCapacityKg(60.0, 100.0), 40.0);
}

TEST(ComplianceTest, RemainingCapacityWhenAlreadyOverLimitIsZero) {
    // Boundary case: current stock already past the limit should report
    // zero remaining capacity, not a negative number.
    EXPECT_DOUBLE_EQ(remainingCapacityKg(120.0, 100.0), 0.0);
}

TEST(ComplianceTest, RemainingCapacityWhenExactlyAtLimitIsZero) {
    EXPECT_DOUBLE_EQ(remainingCapacityKg(100.0, 100.0), 0.0);
}
