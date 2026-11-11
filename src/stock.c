/*
 * stock.c
 *
 * Implements the Stock module: see include/stock.h for the public
 * interface and documentation of each function's contract.
 */

#include "../include/stock.h"
#include <string.h>
#include <stdio.h>

static const char *CATEGORY_NAMES[CATEGORY_COUNT] = {
    "Sparklers",
    "Flowerpots",
    "Chakris",
    "Rockets",
    "Bombs",
    "Other"
};

void initInventory(Inventory *inv)
{
    if (inv == NULL) {
        return;
    }
    inv->count = 0;
    inv->nextId = 1;
}

OperationStatus addStockItem(Inventory *inv, CrackerCategory category,
                              const char *batchNo, double quantityKg,
                              const char *expiryDate)
{
    if (inv == NULL || batchNo == NULL || expiryDate == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    if (inv->count >= MAX_ITEMS) {
        return OP_ERROR_CAPACITY_FULL;
    }
    if (findItemByBatch(inv, batchNo) != NULL) {
        return OP_ERROR_DUPLICATE;
    }

    StockItem *newItem = &inv->items[inv->count];
    newItem->id = inv->nextId;
    newItem->category = category;
    strncpy(newItem->batchNo, batchNo, MAX_BATCH_LEN - 1);
    newItem->batchNo[MAX_BATCH_LEN - 1] = '\0';
    newItem->quantityKg = quantityKg;
    strncpy(newItem->expiryDate, expiryDate, MAX_DATE_LEN - 1);
    newItem->expiryDate[MAX_DATE_LEN - 1] = '\0';

    inv->count++;
    inv->nextId++;
    return OP_SUCCESS;
}

StockItem *findItemById(Inventory *inv, int id)
{
    if (inv == NULL) {
        return NULL;
    }
    for (int i = 0; i < inv->count; i++) {
        if (inv->items[i].id == id) {
            return &inv->items[i];
        }
    }
    return NULL;
}

StockItem *findItemByBatch(Inventory *inv, const char *batchNo)
{
    if (inv == NULL || batchNo == NULL) {
        return NULL;
    }
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].batchNo, batchNo) == 0) {
            return &inv->items[i];
        }
    }
    return NULL;
}

OperationStatus sellStock(Inventory *inv, int id, double quantityKg)
{
    if (inv == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    if (quantityKg <= 0.0) {
        return OP_ERROR_INVALID_INPUT;
    }
    StockItem *item = findItemById(inv, id);
    if (item == NULL) {
        return OP_ERROR_NOT_FOUND;
    }
    if (quantityKg > item->quantityKg) {
        return OP_ERROR_INSUFFICIENT_STOCK;
    }
    item->quantityKg -= quantityKg;
    return OP_SUCCESS;
}

OperationStatus restockItem(Inventory *inv, int id, double quantityKg)
{
    if (inv == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    if (quantityKg <= 0.0) {
        return OP_ERROR_INVALID_INPUT;
    }
    StockItem *item = findItemById(inv, id);
    if (item == NULL) {
        return OP_ERROR_NOT_FOUND;
    }
    item->quantityKg += quantityKg;
    return OP_SUCCESS;
}

OperationStatus removeItem(Inventory *inv, int id)
{
    if (inv == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    int foundIndex = -1;
    for (int i = 0; i < inv->count; i++) {
        if (inv->items[i].id == id) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex == -1) {
        return OP_ERROR_NOT_FOUND;
    }
    for (int i = foundIndex; i < inv->count - 1; i++) {
        inv->items[i] = inv->items[i + 1];
    }
    inv->count--;
    return OP_SUCCESS;
}

double calculateTotalStock(const Inventory *inv)
{
    if (inv == NULL) {
        return 0.0;
    }
    double total = 0.0;
    for (int i = 0; i < inv->count; i++) {
        total += inv->items[i].quantityKg;
    }
    return total;
}

double calculateCategoryStock(const Inventory *inv, CrackerCategory category)
{
    if (inv == NULL) {
        return 0.0;
    }
    double total = 0.0;
    for (int i = 0; i < inv->count; i++) {
        if (inv->items[i].category == category) {
            total += inv->items[i].quantityKg;
        }
    }
    return total;
}

const char *categoryName(CrackerCategory category)
{
    if (category < 0 || category >= CATEGORY_COUNT) {
        return "Unknown";
    }
    return CATEGORY_NAMES[category];
}
