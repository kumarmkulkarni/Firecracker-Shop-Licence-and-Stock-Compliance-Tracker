/*
 * report.h
 *
 * Module: Report
 * Purpose: Declares functions that print human-readable reports of the
 *          current stock and compliance position to stdout.
 *
 * Depends on: common.h, stock.h, compliance.h
 */

#ifndef REPORT_H
#define REPORT_H

#include "stock.h"
#include "compliance.h"

/*
 * printStockReport
 * Purpose: Print a formatted table of every stock item currently held,
 *          including per-item quantity and category.
 * Parameters: inv - pointer to the Inventory to report on (read-only).
 * Returns: void.
 * Preconditions: inv is not NULL.
 * Postconditions: none (output only).
 */
void printStockReport(const Inventory *inv);

/*
 * printComplianceReport
 * Purpose: Print the shop's total stock, licensed limit, remaining
 *          capacity and compliance status.
 * Parameters:
 *   inv  - pointer to the Inventory to report on (read-only).
 *   shop - pointer to the ShopInfo holding the licensed limit (read-only).
 * Returns: void.
 * Preconditions: inv and shop are not NULL.
 * Postconditions: none (output only).
 */
void printComplianceReport(const Inventory *inv, const ShopInfo *shop);

/*
 * printCategoryBreakdown
 * Purpose: Print the total quantity held per cracker category.
 * Parameters: inv - pointer to the Inventory to report on (read-only).
 * Returns: void.
 * Preconditions: inv is not NULL.
 * Postconditions: none (output only).
 */
void printCategoryBreakdown(const Inventory *inv);

#endif /* REPORT_H */
