# Detailed Software Design
## Firecracker Shop Licence and Stock Compliance Tracker

## 1. Engineering Scenario and Problem

Licensed firecracker retailers in India operate under PESO (Petroleum
and Explosives Safety Organisation) rules that cap the total explosive
stock a shop may hold at any time. In practice this limit is tracked on
paper or not at all, so shop owners often discover they are close to
(or over) the limit only when it is too late — after a fresh delivery
has already arrived, or during an inspection.

**Problem:** there is no simple, immediate way for a shop owner to know,
at the moment of adding or selling stock, whether the shop's current
holdings are safely within its licensed limit.

**Proposed solution:** a console application that records every stock
batch (category, quantity, batch number, expiry), checks every addition
or restock against the licensed limit *before* it is committed, and
reports the shop's compliance status (SAFE / NEAR_LIMIT / EXCEEDED) on
demand.

**Target users:** the shop owner or a staff member operating the till,
entering stock arrivals and sales as they happen.

## 2. Software Architecture

See `architecture.png` for the block diagram. The application uses a
layered structure:

| Layer | Module(s) | Responsibility |
|---|---|---|
| Control | `main.c` | Menu loop, wiring the other modules together |
| Validation | `validation.c/h` | Pure checks on user/file input — no I/O |
| Business logic | `stock.c/h` | Inventory CRUD and totals |
| Decision logic | `compliance.c/h` | Stock-vs-licence-limit decisions |
| Presentation | `report.c/h` | Formatted console reports |
| Persistence | `file_handler.c/h` | All disk reads/writes, each checked |

`main.c` contains no business logic: it loads data, shows the menu,
dispatches to the other modules, and saves on exit.

## 3. Module Design

### `stock.c` / `stock.h`
- **Purpose:** owns the `Inventory` and `StockItem`/`ShopInfo` data types
  and the operations on them.
- **Public functions:** `initInventory`, `addStockItem`, `findItemById`,
  `findItemByBatch`, `sellStock`, `restockItem`, `removeItem`,
  `calculateTotalStock`, `calculateCategoryStock`, `categoryName`.
- **Depends on:** `common.h`.
- **Status codes returned:** `OP_SUCCESS`, `OP_ERROR_NOT_FOUND`,
  `OP_ERROR_INVALID_INPUT`, `OP_ERROR_DUPLICATE`,
  `OP_ERROR_CAPACITY_FULL`, `OP_ERROR_INSUFFICIENT_STOCK`.

### `validation.c` / `validation.h`
- **Purpose:** pure, side-effect-free checks used before any stock
  operation is attempted.
- **Public functions:** `isValidQuantity`, `isValidCategory`,
  `isValidBatchNo`, `isValidDate`, `isValidMenuChoice`,
  `isDuplicateBatch`.
- **Depends on:** `common.h`, `stock.h`.

### `compliance.c` / `compliance.h`
- **Purpose:** the project's core decision-making logic — classifying
  stock levels against the licensed limit.
- **Public functions:** `determineStockStatus`, `willExceedLimit`,
  `remainingCapacityKg`, `stockStatusLabel`.
- **Depends on:** `common.h`, `stock.h`.

### `file_handler.c` / `file_handler.h`
- **Purpose:** all file I/O, with every `fopen`/`fread`/`fwrite`
  checked for success.
- **Public functions:** `loadShopInfo`, `saveShopInfo`, `loadInventory`,
  `saveInventory`, `appendTransactionLog`.
- **Depends on:** `common.h`, `stock.h`.

### `report.c` / `report.h`
- **Purpose:** formats and prints stock/compliance/category reports.
- **Public functions:** `printStockReport`, `printComplianceReport`,
  `printCategoryBreakdown`.
- **Depends on:** `stock.h`, `compliance.h`.

### `main.c`
- **Purpose:** menu loop, input prompts, and wiring calls between the
  modules above. Contains no business rules of its own.

## 4. Data Design

### Enumerations
- `CrackerCategory`: `CAT_SPARKLERS, CAT_FLOWERPOTS, CAT_CHAKRIS, CAT_ROCKETS, CAT_BOMBS, CAT_OTHER`
- `OperationStatus`: generic success/error codes shared across modules
- `StockStatus`: `STATUS_SAFE, STATUS_NEAR_LIMIT, STATUS_EXCEEDED`

### Structures
- `StockItem { id, category, batchNo[16], quantityKg, expiryDate[11] }`
- `ShopInfo { licenceNo[20], ownerName[50], licensedLimitKg }`
- `Inventory { items[MAX_ITEMS], count, nextId }`

### Data dictionary (selected fields)

| Field | Type | Valid range / size | Validation rule |
|---|---|---|---|
| `quantityKg` | double | > 0.0 | `isValidQuantity` |
| `category` | enum (int) | 0–5 | `isValidCategory` |
| `batchNo` | char[16] | 1–15 chars, alnum/`-` | `isValidBatchNo` |
| `expiryDate` | char[11] | `DD-MM-YYYY`, real calendar date, year 2000–2100 | `isValidDate` |
| `licensedLimitKg` | double | > 0.0 | checked at load; a non-positive limit is treated as already exceeded |

### Named constants (`common.h`)
`MAX_ITEMS=200`, `MAX_BATCH_LEN=16`, `MAX_DATE_LEN=11`, `MAX_NAME_LEN=50`,
`MAX_LICENCE_LEN=20`, `MAX_LINE_LEN=256`, `NEAR_LIMIT_RATIO=0.90`.

## 5. Function Design (representative examples)

### `determineStockStatus`
```c
StockStatus determineStockStatus(double totalStockKg, double licensedLimitKg);
```
- **Purpose:** classify current stock against the licensed limit.
- **Returns:** `STATUS_EXCEEDED` if over the limit; `STATUS_NEAR_LIMIT`
  if at or above 90% of the limit; `STATUS_SAFE` otherwise.
- **Precondition:** `licensedLimitKg > 0` (a non-positive limit is
  treated defensively as already exceeded).
- **Module:** `compliance.c`.

### `sellStock`
```c
OperationStatus sellStock(Inventory *inv, int id, double quantityKg);
```
- **Purpose:** reduce an item's quantity to record a sale.
- **Returns:** `OP_SUCCESS`, `OP_ERROR_NOT_FOUND`,
  `OP_ERROR_INVALID_INPUT`, or `OP_ERROR_INSUFFICIENT_STOCK`.
- **Precondition:** `inv` not NULL.
- **Postcondition:** on success, the item's `quantityKg` is reduced;
  state is otherwise unchanged.
- **Module:** `stock.c`.

(Full prototypes and contracts for every function are documented as
header comments in `include/*.h`.)

## 6. Input-Validation Plan

| Input | Rule | Enforced by |
|---|---|---|
| Menu choice | integer within `[MENU_MIN, MENU_MAX]` | `isValidMenuChoice` |
| Category | integer within `[0, CATEGORY_COUNT)` | `isValidCategory` |
| Quantity (kg) | finite, `> 0` | `isValidQuantity` |
| Batch number | 1–15 chars, alphanumeric or `-`, not a duplicate | `isValidBatchNo`, `isDuplicateBatch` |
| Expiry date | `DD-MM-YYYY`, real calendar date incl. leap years | `isValidDate` |
| Non-numeric console input | rejected, re-prompt | `sscanf` result checked in `main.c` |
| End of input (EOF) | application saves and exits cleanly instead of looping | `ReadStatus` / `READ_EOF` handling in `main.c` |

## 7. File-Handling Plan

| File | Format | Written | Read |
|---|---|---|---|
| `data/shop_config.txt` | text, `licenceNo,ownerName,limit` | on first run (defaults) or manual edit | at startup |
| `data/stock.dat` | text, one `id,category,batch,qty,expiry` line per item | on "Save and exit" | at startup |
| `data/transactions.log` | text, append-only, `timestamp,action,id,qty` | after every ADD/SALE/RESTOCK/REMOVE | not read by the app (audit trail for the owner) |

Every `fopen` result is checked; a missing `stock.dat` on first run is
treated as a normal empty inventory, not an error, while a missing
`shop_config.txt` falls back to safe placeholder values the user is
told to edit.

## 8. Error-Handling Strategy

All functions that can fail return an `OperationStatus` (or the more
specific `ReadStatus` for console input), and `main.c` checks every
return value and prints a specific message — no return value is ever
ignored. Categories handled: invalid/duplicate/missing records,
insufficient stock, capacity full, file-open/read/write failure, and
closed input streams (EOF), which are treated as a trigger to save and
exit cleanly rather than loop indefinitely.

## 9. Application Flow

1. Start → initialise inventory → load shop config (or create
   defaults) → load stock file (or start empty).
2. Loop: show menu → read choice → validate choice →
   - **1–4:** collect and validate the operation's inputs, check
     against the licensed limit where relevant, apply the change,
     append a transaction log entry.
   - **5–7:** print the requested report (read-only).
   - **8 / EOF:** save inventory to disk, print a goodbye message, exit.
3. On any invalid input: print an error, return to the menu without
   crashing or losing existing data.

## 10. Unit-Test Plan

| Function | Positive | Negative | Boundary |
|---|---|---|---|
| `isValidQuantity` | 5.0 → true | -3.5 → false | 0.0 → false; 0.01 → true |
| `isValidCategory` | 0 → true | -1 → false | `CATEGORY_COUNT` → false |
| `isValidBatchNo` | "BATCH-01" → true | "" / NULL / "BAD BATCH" → false | exactly `MAX_BATCH_LEN-1` chars → true; one char over → false |
| `isValidDate` | "25-12-2026" → true | wrong format / invalid month → false | 29-02 on non-leap vs leap year |
| `isValidMenuChoice` | mid-range → true | out of range → false | exactly min/max → true |
| `determineStockStatus` | well below limit → SAFE | well above → EXCEEDED | exactly at 90% → NEAR_LIMIT; exactly at limit → NEAR_LIMIT; just over → EXCEEDED |
| `willExceedLimit` | within limit → false | — | landing exactly on limit → false; one unit over → true |
| `remainingCapacityKg` | normal subtraction | — | already over limit → 0.0, never negative |
| `addStockItem` | unique batch → success | duplicate batch → `OP_ERROR_DUPLICATE` | inventory exactly full (`MAX_ITEMS`) → `OP_ERROR_CAPACITY_FULL` |
| `sellStock` | partial sale → success | sell more than held → `OP_ERROR_INSUFFICIENT_STOCK`; unknown id → `OP_ERROR_NOT_FOUND`; negative qty → `OP_ERROR_INVALID_INPUT` | sell exactly the remaining quantity → success, leaves 0 |
| `removeItem` | existing id → success | unknown id → `OP_ERROR_NOT_FOUND` | removing the middle item of three keeps the others intact |
| `calculateTotalStock` / `calculateCategoryStock` | sums across items | — | empty inventory → 0.0 |

Implemented in `tests/test_validation.cpp`, `tests/test_compliance.cpp`
and `tests/test_stock.cpp` (44 test cases in total). Menu display and
simple print functions (`report.c`) are intentionally not the focus of
unit testing, per the project guidelines.

## 11. A Note on a Bug Found and Fixed During Development

While manually testing the console menu with scripted (piped) input, I
found that a line read with `fgets` into a buffer sized *exactly* to
the longest valid input (e.g. the 10-character date buffer) would fill
completely without consuming the trailing newline. That leftover `\n`
then leaked into the next prompt as a spurious blank line, silently
shifting every subsequent read by one. A second, separate issue
appeared when the input stream closed (EOF): the menu loop did not
distinguish "EOF" from "merely invalid input" and looped forever
printing "Invalid choice."

Both were fixed in `main.c`:
- A single `readRawLine` helper now detects the no-newline-found case
  and flushes any remaining characters of the real line from `stdin`,
  so every read stays aligned to exactly one line of real input.
- A three-way `ReadStatus` (`READ_OK` / `READ_INVALID` / `READ_EOF`)
  lets every prompt distinguish a closed stream from bad input, and the
  main loop now saves data and exits cleanly on EOF instead of looping.

This is documented here because it is exactly the class of defect the
Input-Validation and Error-Handling sections above are meant to guard
against, and because the fix (and the manual test sequences that
exposed it) are themselves part of the engineering record for this
project.
