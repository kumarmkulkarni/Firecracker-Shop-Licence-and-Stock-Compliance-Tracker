/*
 * compliance.c
 *
 * Implements the Compliance module: see include/compliance.h for the
 * public interface and documentation of each function's contract.
 *
 * This module contains the project's core decision-making logic: how
 * the shop's stock compares with its PESO-licensed limit.
 */

#include "../include/compliance.h"

StockStatus determineStockStatus(double totalStockKg, double licensedLimitKg)
{
    if (licensedLimitKg <= 0.0) {
        /* Defensive default: treat a misconfigured limit as already exceeded,
         * since no amount of stock can be considered safe against it. */
        return STATUS_EXCEEDED;
    }
    if (totalStockKg > licensedLimitKg) {
        return STATUS_EXCEEDED;
    }
    if (totalStockKg >= NEAR_LIMIT_RATIO * licensedLimitKg) {
        return STATUS_NEAR_LIMIT;
    }
    return STATUS_SAFE;
}

bool willExceedLimit(double currentTotalKg, double proposedAddKg,
                      double licensedLimitKg)
{
    return (currentTotalKg + proposedAddKg) > licensedLimitKg;
}

double remainingCapacityKg(double currentTotalKg, double licensedLimitKg)
{
    double remaining = licensedLimitKg - currentTotalKg;
    return remaining > 0.0 ? remaining : 0.0;
}

const char *stockStatusLabel(StockStatus status)
{
    switch (status) {
        case STATUS_SAFE:       return "SAFE";
        case STATUS_NEAR_LIMIT: return "NEAR_LIMIT";
        case STATUS_EXCEEDED:   return "EXCEEDED";
        default:                return "UNKNOWN";
    }
}
