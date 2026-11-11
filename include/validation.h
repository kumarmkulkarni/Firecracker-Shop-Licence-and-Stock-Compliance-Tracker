/*
 * validation.h
 *
 * Module: Validation
 * Purpose: Declares pure, independently-testable functions that validate
 *          user and file input before it is acted upon by other modules.
 *          None of these functions perform I/O; they only check data.
 *
 * Depends on: common.h, stock.h
 */

#ifndef VALIDATION_H
#define VALIDATION_H

#include "common.h"
#include "stock.h"
#include <stdbool.h>

/*
 * isValidQuantity
 * Purpose: Check whether a quantity value is usable for stock operations.
 * Parameters: quantityKg - the quantity to check.
 * Returns: true if quantityKg > 0.0 and is a finite number, false otherwise.
 * Preconditions: none.
 * Postconditions: none (read-only).
 */
bool isValidQuantity(double quantityKg);

/*
 * isValidCategory
 * Purpose: Check whether an integer value is a valid CrackerCategory.
 * Parameters: categoryValue - the raw integer read from user/file input.
 * Returns: true if 0 <= categoryValue < CATEGORY_COUNT, false otherwise.
 * Preconditions: none.
 * Postconditions: none (read-only).
 */
bool isValidCategory(int categoryValue);

/*
 * isValidBatchNo
 * Purpose: Check whether a batch number string is non-empty and within
 *          the allowed length, and contains only alphanumeric or '-'
 *          characters.
 * Parameters: batchNo - the string to check (may be NULL).
 * Returns: true if batchNo is a well-formed batch identifier, false otherwise.
 * Preconditions: none.
 * Postconditions: none (read-only).
 */
bool isValidBatchNo(const char *batchNo);

/*
 * isValidDate
 * Purpose: Check whether a date string is in DD-MM-YYYY format and
 *          represents a calendar date with a plausible year.
 * Parameters: date - the string to check (may be NULL).
 * Returns: true if the string is a well-formed, plausible date, false otherwise.
 * Preconditions: none.
 * Postconditions: none (read-only).
 */
bool isValidDate(const char *date);

/*
 * isValidMenuChoice
 * Purpose: Check whether a menu selection is within the supported range.
 * Parameters:
 *   choice  - the value entered by the user.
 *   minOpt  - the smallest valid menu option.
 *   maxOpt  - the largest valid menu option.
 * Returns: true if minOpt <= choice <= maxOpt, false otherwise.
 * Preconditions: minOpt <= maxOpt.
 * Postconditions: none (read-only).
 */
bool isValidMenuChoice(int choice, int minOpt, int maxOpt);

/*
 * isDuplicateBatch
 * Purpose: Check whether a batch number already exists in the inventory.
 * Parameters:
 *   inv     - pointer to the Inventory to search.
 *   batchNo - batch number to check.
 * Returns: true if an item with that batch number exists, false otherwise.
 * Preconditions: inv and batchNo are not NULL.
 * Postconditions: none (read-only).
 */
bool isDuplicateBatch(const Inventory *inv, const char *batchNo);

#endif /* VALIDATION_H */
