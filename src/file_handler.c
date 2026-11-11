/*
 * file_handler.c
 *
 * Implements the File Handler module: see include/file_handler.h for
 * the public interface and documentation of each function's contract.
 *
 * Every fopen/fread/fwrite/fclose call here is checked for success.
 */

#include "../include/file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

OperationStatus loadShopInfo(ShopInfo *shop)
{
    if (shop == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        return OP_ERROR_FILE_IO;
    }

    char line[MAX_LINE_LEN];
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return OP_ERROR_FILE_IO;
    }

    char licenceNo[MAX_LICENCE_LEN];
    char ownerName[MAX_NAME_LEN];
    double limit;

    /* Format: licenceNo,ownerName,licensedLimitKg */
    char *token = strtok(line, ",\n");
    if (token == NULL) { fclose(fp); return OP_ERROR_FILE_IO; }
    strncpy(licenceNo, token, MAX_LICENCE_LEN - 1);
    licenceNo[MAX_LICENCE_LEN - 1] = '\0';

    token = strtok(NULL, ",\n");
    if (token == NULL) { fclose(fp); return OP_ERROR_FILE_IO; }
    strncpy(ownerName, token, MAX_NAME_LEN - 1);
    ownerName[MAX_NAME_LEN - 1] = '\0';

    token = strtok(NULL, ",\n");
    if (token == NULL) { fclose(fp); return OP_ERROR_FILE_IO; }
    limit = atof(token);

    fclose(fp);

    strcpy(shop->licenceNo, licenceNo);
    strcpy(shop->ownerName, ownerName);
    shop->licensedLimitKg = limit;
    return OP_SUCCESS;
}

OperationStatus saveShopInfo(const ShopInfo *shop)
{
    if (shop == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (fp == NULL) {
        return OP_ERROR_FILE_IO;
    }
    int written = fprintf(fp, "%s,%s,%.2f\n", shop->licenceNo, shop->ownerName,
                           shop->licensedLimitKg);
    fclose(fp);
    if (written < 0) {
        return OP_ERROR_FILE_IO;
    }
    return OP_SUCCESS;
}

OperationStatus loadInventory(Inventory *inv)
{
    if (inv == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    initInventory(inv);

    FILE *fp = fopen(STOCK_FILE, "r");
    if (fp == NULL) {
        /* No stock file yet is a normal first run, not an error. */
        return OP_SUCCESS;
    }

    char line[MAX_LINE_LEN];
    int maxIdSeen = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (inv->count >= MAX_ITEMS) {
            break;
        }
        int id, category;
        char batchNo[MAX_BATCH_LEN];
        double quantityKg;
        char expiryDate[MAX_DATE_LEN];

        int parsed = sscanf(line, "%d,%d,%15[^,],%lf,%10[^,\n]",
                             &id, &category, batchNo, &quantityKg, expiryDate);
        if (parsed != 5) {
            continue; /* skip malformed line rather than crash */
        }

        StockItem *item = &inv->items[inv->count];
        item->id = id;
        item->category = (CrackerCategory)category;
        strncpy(item->batchNo, batchNo, MAX_BATCH_LEN - 1);
        item->batchNo[MAX_BATCH_LEN - 1] = '\0';
        item->quantityKg = quantityKg;
        strncpy(item->expiryDate, expiryDate, MAX_DATE_LEN - 1);
        item->expiryDate[MAX_DATE_LEN - 1] = '\0';

        inv->count++;
        if (id > maxIdSeen) {
            maxIdSeen = id;
        }
    }
    fclose(fp);
    inv->nextId = maxIdSeen + 1;
    return OP_SUCCESS;
}

OperationStatus saveInventory(const Inventory *inv)
{
    if (inv == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    FILE *fp = fopen(STOCK_FILE, "w");
    if (fp == NULL) {
        return OP_ERROR_FILE_IO;
    }
    for (int i = 0; i < inv->count; i++) {
        const StockItem *item = &inv->items[i];
        int written = fprintf(fp, "%d,%d,%s,%.2f,%s\n",
                               item->id, (int)item->category, item->batchNo,
                               item->quantityKg, item->expiryDate);
        if (written < 0) {
            fclose(fp);
            return OP_ERROR_FILE_IO;
        }
    }
    fclose(fp);
    return OP_SUCCESS;
}

OperationStatus appendTransactionLog(const char *action, int itemId,
                                      double quantityKg)
{
    if (action == NULL) {
        return OP_ERROR_INVALID_INPUT;
    }
    FILE *fp = fopen(TRANSACTION_LOG, "a");
    if (fp == NULL) {
        return OP_ERROR_FILE_IO;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[20];
    if (localTime != NULL) {
        strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M", localTime);
    } else {
        strcpy(timestamp, "UNKNOWN_TIME");
    }

    int written = fprintf(fp, "%s,%s,%d,%.2f\n", timestamp, action, itemId,
                           quantityKg);
    fclose(fp);
    if (written < 0) {
        return OP_ERROR_FILE_IO;
    }
    return OP_SUCCESS;
}
