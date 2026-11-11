/*
 * main.c
 *
 * Firecracker Shop Licence and Stock Compliance Tracker.
 *
 * Responsibilities of this file (per project guidelines, main.c should
 * only orchestrate, not implement business logic):
 *   - Initialise the application
 *   - Load existing shop configuration and stock data
 *   - Display the main menu and read the user's choice
 *   - Call functions from the other modules to do the real work
 *   - Save data when required
 *   - Perform final cleanup on exit
 */

#include "../include/common.h"
#include "../include/stock.h"
#include "../include/validation.h"
#include "../include/compliance.h"
#include "../include/file_handler.h"
#include "../include/report.h"

#include <stdio.h>
#include <string.h>

#define MENU_MIN 1
#define MENU_MAX 8

static void printMenu(void)
{
    printf("\n============ Firecracker Stock Compliance Tracker ============\n");
    printf("1. Add new stock batch\n");
    printf("2. Record a sale\n");
    printf("3. Restock an existing batch\n");
    printf("4. Remove a batch (e.g. fully sold / expired)\n");
    printf("5. View stock report\n");
    printf("6. View compliance report\n");
    printf("7. View category breakdown\n");
    printf("8. Save and exit\n");
    printf("Enter choice [%d-%d]: ", MENU_MIN, MENU_MAX);
}

/* Return codes for the read* helpers below, so callers can tell a closed
 * input stream (EOF) apart from merely malformed input. Treating the two
 * the same was the cause of an earlier bug: on EOF the menu loop kept
 * re-prompting forever instead of shutting down, which is exactly the
 * "uncontrolled state" the project guidelines warn against. */
typedef enum { READ_OK, READ_INVALID, READ_EOF } ReadStatus;

/*
 * readRawLine
 * Purpose: Read one line of console input into buffer, regardless of
 *          whether it is longer than the buffer can hold.
 *
 * Every other read* helper is built on this one function because of a
 * subtle fgets pitfall: if a line is exactly (size - 1) characters long,
 * fgets fills the buffer completely WITHOUT consuming the trailing '\n'.
 * That leftover newline then sits in stdin and is silently picked up by
 * the *next* read as a spurious empty line, desynchronising every prompt
 * that follows it. This function detects that case (no '\n' found, but
 * not EOF either) and flushes the remainder of the real line from stdin
 * so the stream always stays aligned to actual user input, one line per
 * read, no matter how long the typed line was.
 *
 * Returns READ_EOF if the stream closed with no data read, READ_OK
 * otherwise (buffer always holds a null-terminated string on READ_OK,
 * truncated to size - 1 characters if the typed line was longer).
 */
static ReadStatus readRawLine(char *buffer, size_t size)
{
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return READ_EOF;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else if (len == size - 1) {
        /* Buffer filled exactly without a newline: either the real line
         * was longer than our buffer, or it was exactly (size - 1) chars
         * long. Either way, discard everything up to the real end of
         * line so the next read starts cleanly on the next line. */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* discard */
        }
    }
    return READ_OK;
}

static ReadStatus readInt(int *outValue)
{
    char line[MAX_LINE_LEN];
    ReadStatus status = readRawLine(line, sizeof(line));
    if (status == READ_EOF) {
        return READ_EOF;
    }
    return (sscanf(line, "%d", outValue) == 1) ? READ_OK : READ_INVALID;
}

static ReadStatus readDouble(double *outValue)
{
    char line[MAX_LINE_LEN];
    ReadStatus status = readRawLine(line, sizeof(line));
    if (status == READ_EOF) {
        return READ_EOF;
    }
    return (sscanf(line, "%lf", outValue) == 1) ? READ_OK : READ_INVALID;
}

static void readLine(char *buffer, int size)
{
    readRawLine(buffer, (size_t)size);
}

/* Each handler returns READ_EOF if input closed partway through, so the
 * main loop can shut down immediately instead of looping on a dead stream;
 * READ_OK is returned otherwise, whether or not the operation itself
 * succeeded (operation failures are reported to the user, not to main). */
static ReadStatus handleAddStock(Inventory *inv, const ShopInfo *shop)
{
    int categoryValue;
    char batchNo[MAX_BATCH_LEN];
    double quantityKg;
    char expiryDate[MAX_DATE_LEN];
    ReadStatus rs;

    printf("Category (0=Sparklers 1=Flowerpots 2=Chakris 3=Rockets 4=Bombs 5=Other): ");
    rs = readInt(&categoryValue);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK || !isValidCategory(categoryValue)) {
        printf("Error: invalid category.\n");
        return READ_OK;
    }

    printf("Batch number (alphanumeric/-, max %d chars): ", MAX_BATCH_LEN - 1);
    readLine(batchNo, sizeof(batchNo));
    if (!isValidBatchNo(batchNo)) {
        printf("Error: invalid batch number.\n");
        return READ_OK;
    }
    if (isDuplicateBatch(inv, batchNo)) {
        printf("Error: a batch with this number already exists.\n");
        return READ_OK;
    }

    printf("Quantity (kg): ");
    rs = readDouble(&quantityKg);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK || !isValidQuantity(quantityKg)) {
        printf("Error: quantity must be a positive number.\n");
        return READ_OK;
    }

    double currentTotal = calculateTotalStock(inv);
    if (willExceedLimit(currentTotal, quantityKg, shop->licensedLimitKg)) {
        printf("Error: adding %.2f kg would EXCEED the licensed limit of %.2f kg ",
               quantityKg, shop->licensedLimitKg);
        printf("(current: %.2f kg, remaining capacity: %.2f kg). Entry rejected.\n",
               currentTotal, remainingCapacityKg(currentTotal, shop->licensedLimitKg));
        return READ_OK;
    }

    printf("Expiry date (DD-MM-YYYY): ");
    readLine(expiryDate, sizeof(expiryDate));
    if (!isValidDate(expiryDate)) {
        printf("Error: invalid date.\n");
        return READ_OK;
    }

    OperationStatus status = addStockItem(inv, (CrackerCategory)categoryValue,
                                           batchNo, quantityKg, expiryDate);
    if (status == OP_SUCCESS) {
        StockItem *added = findItemByBatch(inv, batchNo);
        appendTransactionLog("ADD", added != NULL ? added->id : -1, quantityKg);
        printf("Stock batch added successfully.\n");
    } else if (status == OP_ERROR_CAPACITY_FULL) {
        printf("Error: inventory has reached its maximum number of tracked batches.\n");
    } else {
        printf("Error: could not add stock batch.\n");
    }
    return READ_OK;
}

static ReadStatus handleSale(Inventory *inv)
{
    int id;
    double quantityKg;
    ReadStatus rs;

    printf("Item ID to sell from: ");
    rs = readInt(&id);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK) {
        printf("Error: invalid ID.\n");
        return READ_OK;
    }
    printf("Quantity sold (kg): ");
    rs = readDouble(&quantityKg);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK) {
        printf("Error: invalid quantity.\n");
        return READ_OK;
    }

    OperationStatus status = sellStock(inv, id, quantityKg);
    switch (status) {
        case OP_SUCCESS:
            appendTransactionLog("SALE", id, quantityKg);
            printf("Sale recorded.\n");
            break;
        case OP_ERROR_NOT_FOUND:
            printf("Error: no item with that ID.\n");
            break;
        case OP_ERROR_INVALID_INPUT:
            printf("Error: quantity must be a positive number.\n");
            break;
        case OP_ERROR_INSUFFICIENT_STOCK:
            printf("Error: insufficient stock for that sale.\n");
            break;
        default:
            printf("Error: could not record sale.\n");
            break;
    }
    return READ_OK;
}

static ReadStatus handleRestock(Inventory *inv, const ShopInfo *shop)
{
    int id;
    double quantityKg;
    ReadStatus rs;

    printf("Item ID to restock: ");
    rs = readInt(&id);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK) {
        printf("Error: invalid ID.\n");
        return READ_OK;
    }
    StockItem *item = findItemById(inv, id);
    if (item == NULL) {
        printf("Error: no item with that ID.\n");
        return READ_OK;
    }
    printf("Quantity to add (kg): ");
    rs = readDouble(&quantityKg);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK || !isValidQuantity(quantityKg)) {
        printf("Error: quantity must be a positive number.\n");
        return READ_OK;
    }

    double currentTotal = calculateTotalStock(inv);
    if (willExceedLimit(currentTotal, quantityKg, shop->licensedLimitKg)) {
        printf("Error: restocking %.2f kg would EXCEED the licensed limit of %.2f kg. ",
               quantityKg, shop->licensedLimitKg);
        printf("Remaining capacity is %.2f kg. Entry rejected.\n",
               remainingCapacityKg(currentTotal, shop->licensedLimitKg));
        return READ_OK;
    }

    OperationStatus status = restockItem(inv, id, quantityKg);
    if (status == OP_SUCCESS) {
        appendTransactionLog("RESTOCK", id, quantityKg);
        printf("Restock recorded.\n");
    } else {
        printf("Error: could not restock item.\n");
    }
    return READ_OK;
}

static ReadStatus handleRemove(Inventory *inv)
{
    int id;
    ReadStatus rs;
    printf("Item ID to remove: ");
    rs = readInt(&id);
    if (rs == READ_EOF) return READ_EOF;
    if (rs != READ_OK) {
        printf("Error: invalid ID.\n");
        return READ_OK;
    }
    OperationStatus status = removeItem(inv, id);
    if (status == OP_SUCCESS) {
        appendTransactionLog("REMOVE", id, 0.0);
        printf("Item removed.\n");
    } else {
        printf("Error: no item with that ID.\n");
    }
    return READ_OK;
}

/* Saves all in-memory data to disk. Used both for a normal "save and
 * exit" (menu choice 8) and for an unexpected end of input, so that no
 * work is lost either way. */
static void saveAndExit(const Inventory *inv)
{
    if (saveInventory(inv) == OP_SUCCESS) {
        printf("Stock data saved. Goodbye.\n");
    } else {
        printf("Warning: could not save stock data.\n");
    }
}

int main(void)
{
    ShopInfo shop;
    Inventory inventory;

    initInventory(&inventory);

    if (loadShopInfo(&shop) != OP_SUCCESS) {
        /* First run: no config file yet. Set up a default licence so the
         * application remains usable; the user can edit data/shop_config.txt
         * directly to match their actual licence. */
        strcpy(shop.licenceNo, "PESO-UNSET");
        strcpy(shop.ownerName, "Unset Owner");
        shop.licensedLimitKg = 100.0;
        printf("No shop configuration found. Using default licence settings.\n");
        printf("Edit %s to set your actual licence details.\n", CONFIG_FILE);
        saveShopInfo(&shop);
    }

    if (loadInventory(&inventory) != OP_SUCCESS) {
        printf("Warning: could not read existing stock file. Starting with empty stock.\n");
    }

    printf("Welcome, %s (Licence: %s). Licensed limit: %.2f kg.\n",
           shop.ownerName, shop.licenceNo, shop.licensedLimitKg);

    int running = 1;
    while (running) {
        printMenu();
        int choice;
        ReadStatus rs = readInt(&choice);
        if (rs == READ_EOF) {
            /* Input stream closed (e.g. piped input ran out, or Ctrl-D).
             * Save whatever has been entered so far and shut down cleanly
             * instead of looping forever on a dead stream. */
            printf("\nInput closed.\n");
            saveAndExit(&inventory);
            break;
        }
        if (rs != READ_OK || !isValidMenuChoice(choice, MENU_MIN, MENU_MAX)) {
            printf("Invalid choice. Please enter a number between %d and %d.\n",
                   MENU_MIN, MENU_MAX);
            continue;
        }

        ReadStatus handlerStatus = READ_OK;
        switch (choice) {
            case 1: handlerStatus = handleAddStock(&inventory, &shop); break;
            case 2: handlerStatus = handleSale(&inventory); break;
            case 3: handlerStatus = handleRestock(&inventory, &shop); break;
            case 4: handlerStatus = handleRemove(&inventory); break;
            case 5: printStockReport(&inventory); break;
            case 6: printComplianceReport(&inventory, &shop); break;
            case 7: printCategoryBreakdown(&inventory); break;
            case 8:
                saveAndExit(&inventory);
                running = 0;
                break;
            default:
                /* unreachable due to isValidMenuChoice check above */
                break;
        }

        if (handlerStatus == READ_EOF) {
            printf("\nInput closed.\n");
            saveAndExit(&inventory);
            break;
        }
    }

    return 0;
}
