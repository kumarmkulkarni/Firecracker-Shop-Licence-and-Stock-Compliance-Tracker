/*
 * report.c
 *
 * Implements the Report module: see include/report.h for the public
 * interface and documentation of each function's contract.
 */

#include "../include/report.h"
#include "../include/common.h"
#include <stdio.h>

void printStockReport(const Inventory *inv)
{
    if (inv == NULL) {
        return;
    }
    printf("\n===================== STOCK REPORT =====================\n");
    if (inv->count == 0) {
        printf("No stock items recorded.\n");
    } else {
        printf("%-4s %-12s %-10s %-10s %-12s\n",
               "ID", "Category", "Batch", "Qty(kg)", "Expiry");
        printf("----------------------------------------------------------\n");
        for (int i = 0; i < inv->count; i++) {
            const StockItem *item = &inv->items[i];
            printf("%-4d %-12s %-10s %-10.2f %-12s\n",
                   item->id, categoryName(item->category), item->batchNo,
                   item->quantityKg, item->expiryDate);
        }
    }
    printf("==========================================================\n");
}

void printComplianceReport(const Inventory *inv, const ShopInfo *shop)
{
    if (inv == NULL || shop == NULL) {
        return;
    }
    double total = calculateTotalStock(inv);
    StockStatus status = determineStockStatus(total, shop->licensedLimitKg);
    double remaining = remainingCapacityKg(total, shop->licensedLimitKg);

    printf("\n=================== COMPLIANCE REPORT ===================\n");
    printf("Licence No.       : %s\n", shop->licenceNo);
    printf("Owner             : %s\n", shop->ownerName);
    printf("Licensed Limit    : %.2f kg\n", shop->licensedLimitKg);
    printf("Current Stock     : %.2f kg\n", total);
    printf("Remaining Capacity: %.2f kg\n", remaining);
    printf("Compliance Status : %s\n", stockStatusLabel(status));
    if (status == STATUS_NEAR_LIMIT) {
        printf("Warning: stock is approaching the licensed limit.\n");
    } else if (status == STATUS_EXCEEDED) {
        printf("ALERT: stock EXCEEDS the licensed limit. Stop further restocking.\n");
    }
    printf("==========================================================\n");
}

void printCategoryBreakdown(const Inventory *inv)
{
    if (inv == NULL) {
        return;
    }
    printf("\n================= CATEGORY BREAKDOWN ====================\n");
    for (int c = 0; c < CATEGORY_COUNT; c++) {
        double catTotal = calculateCategoryStock(inv, (CrackerCategory)c);
        printf("%-12s : %8.2f kg\n", categoryName((CrackerCategory)c), catTotal);
    }
    printf("==========================================================\n");
}
