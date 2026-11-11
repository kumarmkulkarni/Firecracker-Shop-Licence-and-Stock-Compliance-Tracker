/*
 * stock.h
 *
 * Module: Stock
 * Purpose: Defines the core data structures for firecracker stock items
 *          and the shop's licence information, and declares the
 *          functions used to add, sell, restock, search and summarise
 *          stock.
 *
 * Depends on: common.h
 */

#ifndef STOCK_H
#define STOCK_H

#include "common.h"

/* Category of firecracker. Kept small and representative rather than
 * exhaustive, per project scope. */
typedef enum {
    CAT_SPARKLERS = 0,
    CAT_FLOWERPOTS,
    CAT_CHAKRIS,
    CAT_ROCKETS,
    CAT_BOMBS,
    CAT_OTHER
} CrackerCategory;

#define CATEGORY_COUNT (CAT_OTHER + 1)

/* A single stock item (one batch of one category). */
typedef struct {
    int id;                          /* unique identifier, assigned on add */
    CrackerCategory category;
    char batchNo[MAX_BATCH_LEN];
    double quantityKg;                /* current quantity held, in kilograms */
    char expiryDate[MAX_DATE_LEN];    /* format: DD-MM-YYYY */
} StockItem;

/* The shop's licence details, loaded once at startup. */
typedef struct {
    char licenceNo[MAX_LICENCE_LEN];
    char ownerName[MAX_NAME_LEN];
    double licensedLimitKg;           /* maximum total explosive stock permitted */
} ShopInfo;

/* The full in-memory inventory: a fixed-capacity array plus a count. */
typedef struct {
    StockItem items[MAX_ITEMS];
    int count;
    int nextId;                       /* next id to assign to a new item */
} Inventory;

/*
 * initInventory
 * Purpose: Initialise an empty inventory.
 * Parameters: inv - pointer to the Inventory to initialise.
 * Returns: void.
 * Preconditions: inv is not NULL.
 * Postconditions: inv->count == 0, inv->nextId == 1.
 */
void initInventory(Inventory *inv);

/*
 * addStockItem
 * Purpose: Add a new stock item (initial entry of a batch) to the inventory.
 * Parameters:
 *   inv        - pointer to the Inventory to modify.
 *   category   - the cracker category.
 *   batchNo    - batch identifier string (already validated by caller).
 *   quantityKg - quantity to add, in kilograms (already validated by caller).
 *   expiryDate - expiry date string "DD-MM-YYYY" (already validated by caller).
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_CAPACITY_FULL if inv->count == MAX_ITEMS.
 *   OP_ERROR_DUPLICATE if an item with the same batchNo already exists.
 * Preconditions: inv, batchNo, expiryDate are not NULL; quantityKg > 0.
 * Postconditions: on success, a new StockItem is appended and inv->count
 *                 is incremented.
 */
OperationStatus addStockItem(Inventory *inv, CrackerCategory category,
                              const char *batchNo, double quantityKg,
                              const char *expiryDate);

/*
 * findItemById
 * Purpose: Locate a stock item by its unique id.
 * Parameters:
 *   inv - pointer to the Inventory to search.
 *   id  - the id to search for.
 * Returns: pointer to the matching StockItem, or NULL if not found.
 * Preconditions: inv is not NULL.
 * Postconditions: none (read-only).
 */
StockItem *findItemById(Inventory *inv, int id);

/*
 * findItemByBatch
 * Purpose: Locate a stock item by its batch number.
 * Parameters:
 *   inv     - pointer to the Inventory to search.
 *   batchNo - batch number to search for.
 * Returns: pointer to the matching StockItem, or NULL if not found.
 * Preconditions: inv and batchNo are not NULL.
 * Postconditions: none (read-only).
 */
StockItem *findItemByBatch(Inventory *inv, const char *batchNo);

/*
 * sellStock
 * Purpose: Reduce the quantity of a stock item to record a sale.
 * Parameters:
 *   inv        - pointer to the Inventory to modify.
 *   id         - id of the item being sold from.
 *   quantityKg - quantity sold, in kilograms.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_NOT_FOUND if no item with that id exists.
 *   OP_ERROR_INVALID_INPUT if quantityKg <= 0.
 *   OP_ERROR_INSUFFICIENT_STOCK if quantityKg exceeds the item's current quantity.
 * Preconditions: inv is not NULL.
 * Postconditions: on success, the item's quantityKg is reduced.
 */
OperationStatus sellStock(Inventory *inv, int id, double quantityKg);

/*
 * restockItem
 * Purpose: Increase the quantity of an existing stock item.
 * Parameters:
 *   inv        - pointer to the Inventory to modify.
 *   id         - id of the item being restocked.
 *   quantityKg - quantity to add, in kilograms.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_NOT_FOUND if no item with that id exists.
 *   OP_ERROR_INVALID_INPUT if quantityKg <= 0.
 * Preconditions: inv is not NULL.
 * Postconditions: on success, the item's quantityKg is increased.
 */
OperationStatus restockItem(Inventory *inv, int id, double quantityKg);

/*
 * removeItem
 * Purpose: Remove a stock item entirely (e.g. a batch fully sold or expired).
 * Parameters:
 *   inv - pointer to the Inventory to modify.
 *   id  - id of the item to remove.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_NOT_FOUND if no item with that id exists.
 * Preconditions: inv is not NULL.
 * Postconditions: on success, the item is removed and inv->count decremented;
 *                 remaining items are shifted to keep the array contiguous.
 */
OperationStatus removeItem(Inventory *inv, int id);

/*
 * calculateTotalStock
 * Purpose: Sum the quantity (kg) of every item currently in the inventory.
 * Parameters: inv - pointer to the Inventory (read-only).
 * Returns: total quantity in kilograms across all items.
 * Preconditions: inv is not NULL.
 * Postconditions: none (read-only). Returns 0.0 for an empty inventory.
 */
double calculateTotalStock(const Inventory *inv);

/*
 * calculateCategoryStock
 * Purpose: Sum the quantity (kg) of items in a single category.
 * Parameters:
 *   inv      - pointer to the Inventory (read-only).
 *   category - the category to total.
 * Returns: total quantity in kilograms for that category.
 * Preconditions: inv is not NULL.
 * Postconditions: none (read-only).
 */
double calculateCategoryStock(const Inventory *inv, CrackerCategory category);

/*
 * categoryName
 * Purpose: Convert a CrackerCategory value to a human-readable string.
 * Parameters: category - the category to describe.
 * Returns: a pointer to a static, read-only string naming the category.
 * Preconditions: none.
 * Postconditions: none.
 */
const char *categoryName(CrackerCategory category);

#endif /* STOCK_H */
