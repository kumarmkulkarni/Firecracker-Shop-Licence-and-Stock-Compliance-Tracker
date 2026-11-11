/*
 * test_validation.cpp
 *
 * Unit tests for the Validation module (include/validation.h).
 * C headers are included with extern "C" linkage so these C++ test
 * files can link directly against the C object files.
 */

extern "C" {
#include "../include/validation.h"
#include "../include/stock.h"
}

#include <gtest/gtest.h>
#include <string>

/* ---------- isValidQuantity ---------- */

TEST(ValidationTest, QuantityPositiveIsValid) {
    EXPECT_TRUE(isValidQuantity(5.0));
}

TEST(ValidationTest, QuantityZeroIsInvalid) {
    // Boundary case: zero is not a valid quantity to add/sell/restock.
    EXPECT_FALSE(isValidQuantity(0.0));
}

TEST(ValidationTest, QuantityNegativeIsInvalid) {
    EXPECT_FALSE(isValidQuantity(-3.5));
}

TEST(ValidationTest, QuantityJustAboveZeroIsValid) {
    // Boundary case: smallest positive value should still be accepted.
    EXPECT_TRUE(isValidQuantity(0.01));
}

/* ---------- isValidCategory ---------- */

TEST(ValidationTest, CategoryLowerBoundIsValid) {
    EXPECT_TRUE(isValidCategory(CAT_SPARKLERS)); // 0
}

TEST(ValidationTest, CategoryUpperBoundIsValid) {
    EXPECT_TRUE(isValidCategory(CAT_OTHER)); // last valid value
}

TEST(ValidationTest, CategoryNegativeIsInvalid) {
    EXPECT_FALSE(isValidCategory(-1));
}

TEST(ValidationTest, CategoryJustPastUpperBoundIsInvalid) {
    // Boundary case: one past the last valid enum value.
    EXPECT_FALSE(isValidCategory(CATEGORY_COUNT));
}

/* ---------- isValidBatchNo ---------- */

TEST(ValidationTest, BatchNoNormalIsValid) {
    EXPECT_TRUE(isValidBatchNo("BATCH-01"));
}

TEST(ValidationTest, BatchNoEmptyIsInvalid) {
    EXPECT_FALSE(isValidBatchNo(""));
}

TEST(ValidationTest, BatchNoWithSpaceIsInvalid) {
    EXPECT_FALSE(isValidBatchNo("BAD BATCH"));
}

TEST(ValidationTest, BatchNoNullIsInvalid) {
    EXPECT_FALSE(isValidBatchNo(NULL));
}

TEST(ValidationTest, BatchNoAtMaxLengthIsValid) {
    // Boundary case: exactly MAX_BATCH_LEN - 1 characters (the longest
    // batch number that fits with a null terminator).
    std::string batch(MAX_BATCH_LEN - 1, 'A');
    EXPECT_TRUE(isValidBatchNo(batch.c_str()));
}

TEST(ValidationTest, BatchNoOverMaxLengthIsInvalid) {
    // Boundary case: one character past the allowed length.
    std::string batch(MAX_BATCH_LEN, 'A');
    EXPECT_FALSE(isValidBatchNo(batch.c_str()));
}

/* ---------- isValidDate ---------- */

TEST(ValidationTest, DateWellFormedIsValid) {
    EXPECT_TRUE(isValidDate("25-12-2026"));
}

TEST(ValidationTest, DateWrongFormatIsInvalid) {
    EXPECT_FALSE(isValidDate("2026-12-25"));
}

TEST(ValidationTest, DateInvalidMonthIsInvalid) {
    EXPECT_FALSE(isValidDate("15-13-2026"));
}

TEST(ValidationTest, DateFeb29NonLeapYearIsInvalid) {
    // Boundary case: 2025 is not a leap year, so Feb 29 does not exist.
    EXPECT_FALSE(isValidDate("29-02-2025"));
}

TEST(ValidationTest, DateFeb29LeapYearIsValid) {
    // Boundary case: 2028 is a leap year, so Feb 29 is valid.
    EXPECT_TRUE(isValidDate("29-02-2028"));
}

/* ---------- isValidMenuChoice ---------- */

TEST(ValidationTest, MenuChoiceWithinRangeIsValid) {
    EXPECT_TRUE(isValidMenuChoice(4, 1, 8));
}

TEST(ValidationTest, MenuChoiceLowerBoundIsValid) {
    EXPECT_TRUE(isValidMenuChoice(1, 1, 8));
}

TEST(ValidationTest, MenuChoiceUpperBoundIsValid) {
    EXPECT_TRUE(isValidMenuChoice(8, 1, 8));
}

TEST(ValidationTest, MenuChoiceBelowRangeIsInvalid) {
    EXPECT_FALSE(isValidMenuChoice(0, 1, 8));
}

TEST(ValidationTest, MenuChoiceAboveRangeIsInvalid) {
    EXPECT_FALSE(isValidMenuChoice(9, 1, 8));
}
