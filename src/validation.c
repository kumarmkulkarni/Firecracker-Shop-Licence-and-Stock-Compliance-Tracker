/*
 * validation.c
 *
 * Implements the Validation module: see include/validation.h for the
 * public interface and documentation of each function's contract.
 *
 * All functions here are pure (no I/O, no side effects) so they can be
 * unit tested directly and independently of file or console input.
 */

#include "../include/validation.h"
#include <string.h>
#include <ctype.h>
#include <math.h>

bool isValidQuantity(double quantityKg)
{
    if (isnan(quantityKg) || isinf(quantityKg)) {
        return false;
    }
    return quantityKg > 0.0;
}

bool isValidCategory(int categoryValue)
{
    return categoryValue >= 0 && categoryValue < CATEGORY_COUNT;
}

bool isValidBatchNo(const char *batchNo)
{
    if (batchNo == NULL) {
        return false;
    }
    size_t len = strlen(batchNo);
    if (len == 0 || len >= MAX_BATCH_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)batchNo[i];
        if (!isalnum(c) && c != '-') {
            return false;
        }
    }
    return true;
}

/* Days in each month for a non-leap year; February is handled separately. */
static int daysInMonth(int month, int year)
{
    static const int DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    return DAYS[month - 1];
}

bool isValidDate(const char *date)
{
    if (date == NULL) {
        return false;
    }
    /* Expect exactly "DD-MM-YYYY" -> length 10, hyphens at positions 2 and 5. */
    if (strlen(date) != 10) {
        return false;
    }
    if (date[2] != '-' || date[5] != '-') {
        return false;
    }
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            continue;
        }
        if (!isdigit((unsigned char)date[i])) {
            return false;
        }
    }

    int day = (date[0] - '0') * 10 + (date[1] - '0');
    int month = (date[3] - '0') * 10 + (date[4] - '0');
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
               (date[8] - '0') * 10 + (date[9] - '0');

    if (year < 2000 || year > 2100) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    if (day < 1 || day > daysInMonth(month, year)) {
        return false;
    }
    return true;
}

bool isValidMenuChoice(int choice, int minOpt, int maxOpt)
{
    if (minOpt > maxOpt) {
        return false;
    }
    return choice >= minOpt && choice <= maxOpt;
}

bool isDuplicateBatch(const Inventory *inv, const char *batchNo)
{
    if (inv == NULL || batchNo == NULL) {
        return false;
    }
    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].batchNo, batchNo) == 0) {
            return true;
        }
    }
    return false;
}
