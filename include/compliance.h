/*
 * compliance.h
 *
 * Module: Compliance
 * Purpose: Declares the functions that decide how the shop's current
 *          stock stands against its PESO-licensed limit, which is the
 *          core decision-making logic of this project.
 *
 * Depends on: common.h, stock.h
 */

#ifndef COMPLIANCE_H
#define COMPLIANCE_H

#include "common.h"
#include "stock.h"
#include <stdbool.h>

/* Status of the shop's total stock relative to its licensed limit. */
typedef enum {
    STATUS_SAFE = 0,        /* below NEAR_LIMIT_RATIO of the licensed limit */
    STATUS_NEAR_LIMIT,      /* at or above NEAR_LIMIT_RATIO but <= limit */
    STATUS_EXCEEDED         /* above the licensed limit */
} StockStatus;

/*
 * determineStockStatus
 * Purpose: Classify total stock held against the licensed limit.
 * Parameters:
 *   totalStockKg    - the shop's current total stock, in kilograms.
 *   licensedLimitKg - the licensed maximum, in kilograms.
 * Returns:
 *   STATUS_EXCEEDED   if totalStockKg > licensedLimitKg.
 *   STATUS_NEAR_LIMIT if totalStockKg >= NEAR_LIMIT_RATIO * licensedLimitKg
 *                     (and <= licensedLimitKg).
 *   STATUS_SAFE       otherwise.
 * Preconditions: licensedLimitKg > 0.
 * Postconditions: none (read-only, pure function).
 */
StockStatus determineStockStatus(double totalStockKg, double licensedLimitKg);

/*
 * willExceedLimit
 * Purpose: Predict whether adding a proposed quantity would push total
 *          stock over the licensed limit, before the addition is made.
 * Parameters:
 *   currentTotalKg  - the shop's current total stock, in kilograms.
 *   proposedAddKg   - the quantity proposed to be added, in kilograms.
 *   licensedLimitKg - the licensed maximum, in kilograms.
 * Returns: true if (currentTotalKg + proposedAddKg) > licensedLimitKg.
 * Preconditions: licensedLimitKg > 0.
 * Postconditions: none (read-only, pure function).
 */
bool willExceedLimit(double currentTotalKg, double proposedAddKg,
                      double licensedLimitKg);

/*
 * remainingCapacityKg
 * Purpose: Compute how much more stock (in kg) can legally be added
 *          before the licensed limit is reached.
 * Parameters:
 *   currentTotalKg  - the shop's current total stock, in kilograms.
 *   licensedLimitKg - the licensed maximum, in kilograms.
 * Returns: licensedLimitKg - currentTotalKg, or 0.0 if that would be negative
 *          (i.e. the shop is already at or over the limit).
 * Preconditions: licensedLimitKg > 0.
 * Postconditions: none (read-only, pure function).
 */
double remainingCapacityKg(double currentTotalKg, double licensedLimitKg);

/*
 * stockStatusLabel
 * Purpose: Convert a StockStatus value to a human-readable string.
 * Parameters: status - the status to describe.
 * Returns: a pointer to a static, read-only string naming the status.
 * Preconditions: none.
 * Postconditions: none.
 */
const char *stockStatusLabel(StockStatus status);

#endif /* COMPLIANCE_H */
