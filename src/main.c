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

/* Reads one line of input and parses an integer from it.
 * Returns true and sets *outValue on success, false on bad input
 * (so the caller can re-prompt instead of crashing on non-numeric input). */
static int readInt(int *outValue)
{
    char line[MAX_LINE_LEN];
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }
    return sscanf(line, "%d", outValue) == 1;
}

static int readDouble(double *outValue)
{
    char line[MAX_LINE_LEN];
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }
    return sscanf(line, "%lf", outValue) == 1;
}

static void readLine(char *buffer, int size)
{
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    } else {
        buffer[0] = '\0';
    }
}

static void handleAddStock(Inventory *inv, const ShopInfo *shop)
{
    int categoryValue;
    char batchNo[MAX_BATCH_LEN];
    double quantityKg;
    char expiryDate[MAX_DATE_LEN];

    printf("Category (0=Sparklers 1=Flowerpots 2=Chakris 3=Rockets 4=Bombs 5=Other): ");
    if (!readInt(&categoryValue) || !isValidCategory(categoryValue)) {
        printf("Error: invalid category.\n");
        return;
    }

    printf("Batch number (alphanumeric/-, max %d chars): ", MAX_BATCH_LEN - 1);
    readLine(batchNo, sizeof(batchNo));
    if (!isValidBatchNo(batchNo)) {
        printf("Error: invalid batch number.\n");
        return;
    }
    if (isDuplicateBatch(inv, batchNo)) {
        printf("Error: a batch with this number already exists.\n");
        return;
    }

    printf("Quantity (kg): ");
    if (!readDouble(&quantityKg) || !isValidQuantity(quantityKg)) {
        printf("Error: quantity must be a positive number.\n");
        return;
    }

    double currentTotal = calculateTotalStock(inv);
    if (willExceedLimit(currentTotal, quantityKg, shop->licensedLimitKg)) {
        printf("Error: adding %.2f kg would EXCEED the licensed limit of %.2f kg ",
               quantityKg, shop->licensedLimitKg);
        printf("(current: %.2f kg, remaining capacity: %.2f kg). Entry rejected.\n",
               currentTotal, remainingCapacityKg(currentTotal, shop->licensedLimitKg));
        return;
    }

    printf("Expiry date (DD-MM-YYYY): ");
    readLine(expiryDate, sizeof(expiryDate));
    if (!isValidDate(expiryDate)) {
        printf("Error: invalid date.\n");
        return;
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
}

static void handleSale(Inventory *inv)
{
    int id;
    double quantityKg;

    printf("Item ID to sell from: ");
    if (!readInt(&id)) {
        printf("Error: invalid ID.\n");
        return;
    }
    printf("Quantity sold (kg): ");
    if (!readDouble(&quantityKg)) {
        printf("Error: invalid quantity.\n");
        return;
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
}

static void handleRestock(Inventory *inv, const ShopInfo *shop)
{
    int id;
    double quantityKg;

    printf("Item ID to restock: ");
    if (!readInt(&id)) {
        printf("Error: invalid ID.\n");
        return;
    }
    StockItem *item = findItemById(inv, id);
    if (item == NULL) {
        printf("Error: no item with that ID.\n");
        return;
    }
    printf("Quantity to add (kg): ");
    if (!readDouble(&quantityKg) || !isValidQuantity(quantityKg)) {
        printf("Error: quantity must be a positive number.\n");
        return;
    }

    double currentTotal = calculateTotalStock(inv);
    if (willExceedLimit(currentTotal, quantityKg, shop->licensedLimitKg)) {
        printf("Error: restocking %.2f kg would EXCEED the licensed limit of %.2f kg. ",
               quantityKg, shop->licensedLimitKg);
        printf("Remaining capacity is %.2f kg. Entry rejected.\n",
               remainingCapacityKg(currentTotal, shop->licensedLimitKg));
        return;
    }

    OperationStatus status = restockItem(inv, id, quantityKg);
    if (status == OP_SUCCESS) {
        appendTransactionLog("RESTOCK", id, quantityKg);
        printf("Restock recorded.\n");
    } else {
        printf("Error: could not restock item.\n");
    }
}

static void handleRemove(Inventory *inv)
{
    int id;
    printf("Item ID to remove: ");
    if (!readInt(&id)) {
        printf("Error: invalid ID.\n");
        return;
    }
    OperationStatus status = removeItem(inv, id);
    if (status == OP_SUCCESS) {
        appendTransactionLog("REMOVE", id, 0.0);
        printf("Item removed.\n");
    } else {
        printf("Error: no item with that ID.\n");
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
        if (!readInt(&choice) || !isValidMenuChoice(choice, MENU_MIN, MENU_MAX)) {
            printf("Invalid choice. Please enter a number between %d and %d.\n",
                   MENU_MIN, MENU_MAX);
            continue;
        }

        switch (choice) {
            case 1: handleAddStock(&inventory, &shop); break;
            case 2: handleSale(&inventory); break;
            case 3: handleRestock(&inventory, &shop); break;
            case 4: handleRemove(&inventory); break;
            case 5: printStockReport(&inventory); break;
            case 6: printComplianceReport(&inventory, &shop); break;
            case 7: printCategoryBreakdown(&inventory); break;
            case 8:
                if (saveInventory(&inventory) == OP_SUCCESS) {
                    printf("Stock data saved. Goodbye.\n");
                } else {
                    printf("Warning: could not save stock data.\n");
                }
                running = 0;
                break;
            default:
                /* unreachable due to isValidMenuChoice check above */
                break;
        }
    }

    return 0;
}
