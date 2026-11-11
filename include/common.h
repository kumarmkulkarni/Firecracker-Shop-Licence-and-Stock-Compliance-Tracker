/*
 * common.h
 *
 * Shared constants and limits used across the Firecracker Shop Licence
 * and Stock Compliance Tracker. Centralising these avoids magic numbers
 * scattered through the modules.
 */

#ifndef COMMON_H
#define COMMON_H

/* ---- Sizing limits ---- */
#define MAX_ITEMS          200    /* maximum distinct stock items the shop can hold */
#define MAX_BATCH_LEN      16     /* includes null terminator */
#define MAX_DATE_LEN       11     /* "DD-MM-YYYY" + null terminator */
#define MAX_NAME_LEN       50
#define MAX_LICENCE_LEN    20
#define MAX_LINE_LEN       256    /* buffer size for reading a line from a data file */

/* ---- Compliance thresholds ---- */
/* Stock is "near limit" once it reaches this fraction of the licensed limit. */
#define NEAR_LIMIT_RATIO   0.90

/* ---- File names ---- */
#define STOCK_FILE         "data/stock.dat"
#define CONFIG_FILE        "data/shop_config.txt"
#define TRANSACTION_LOG    "data/transactions.log"

/* ---- General status codes returned by functions in this project ---- */
typedef enum {
    OP_SUCCESS = 0,
    OP_ERROR_NOT_FOUND,
    OP_ERROR_INVALID_INPUT,
    OP_ERROR_DUPLICATE,
    OP_ERROR_CAPACITY_FULL,
    OP_ERROR_FILE_IO,
    OP_ERROR_INSUFFICIENT_STOCK
} OperationStatus;

#endif /* COMMON_H */
