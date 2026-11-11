/*
 * file_handler.h
 *
 * Module: File Handler
 * Purpose: Declares functions that load and save the shop's licence
 *          configuration and stock inventory, and append entries to the
 *          transaction log. All file operations are checked for success
 *          or failure by the caller via the returned OperationStatus.
 *
 * File formats (plain text, one record per line, comma-separated):
 *   shop_config.txt   : licenceNo,ownerName,licensedLimitKg
 *   stock.dat         : id,category,batchNo,quantityKg,expiryDate
 *   transactions.log  : timestamp,action,itemId,quantityKg   (append-only)
 *
 * Depends on: common.h, stock.h
 */

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "common.h"
#include "stock.h"

/*
 * loadShopInfo
 * Purpose: Read the shop's licence configuration from CONFIG_FILE.
 * Parameters: shop - pointer to a ShopInfo struct to populate.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_FILE_IO if the file cannot be opened or its contents are malformed.
 * Preconditions: shop is not NULL.
 * Postconditions: on success, *shop is fully populated.
 */
OperationStatus loadShopInfo(ShopInfo *shop);

/*
 * saveShopInfo
 * Purpose: Write the shop's licence configuration to CONFIG_FILE.
 * Parameters: shop - pointer to the ShopInfo struct to persist.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_FILE_IO if the file cannot be opened for writing.
 * Preconditions: shop is not NULL.
 * Postconditions: on success, CONFIG_FILE contains shop's data.
 */
OperationStatus saveShopInfo(const ShopInfo *shop);

/*
 * loadInventory
 * Purpose: Read all stock items from STOCK_FILE into memory.
 * Parameters: inv - pointer to the Inventory to populate.
 * Returns:
 *   OP_SUCCESS if the file was read successfully (including if it does
 *              not yet exist, which is treated as an empty inventory).
 *   OP_ERROR_FILE_IO if the file exists but cannot be read.
 * Preconditions: inv is not NULL and has been initialised.
 * Postconditions: on success, inv contains every item from the file.
 */
OperationStatus loadInventory(Inventory *inv);

/*
 * saveInventory
 * Purpose: Write all stock items in memory to STOCK_FILE, overwriting it.
 * Parameters: inv - pointer to the Inventory to persist.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_FILE_IO if the file cannot be opened for writing.
 * Preconditions: inv is not NULL.
 * Postconditions: on success, STOCK_FILE reflects the current in-memory state.
 */
OperationStatus saveInventory(const Inventory *inv);

/*
 * appendTransactionLog
 * Purpose: Append one line describing a stock operation to TRANSACTION_LOG.
 * Parameters:
 *   action     - short description of the action, e.g. "SALE", "RESTOCK", "ADD".
 *   itemId     - id of the item involved.
 *   quantityKg - quantity involved in the operation, in kilograms.
 * Returns:
 *   OP_SUCCESS on success.
 *   OP_ERROR_FILE_IO if the file cannot be opened for appending.
 * Preconditions: action is not NULL.
 * Postconditions: on success, one new line is appended to TRANSACTION_LOG.
 */
OperationStatus appendTransactionLog(const char *action, int itemId,
                                      double quantityKg);

#endif /* FILE_HANDLER_H */
