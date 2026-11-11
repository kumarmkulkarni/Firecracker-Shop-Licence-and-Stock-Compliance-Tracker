# Firecracker Shop Licence and Stock Compliance Tracker

## Engineering Domain
Retail / regulatory compliance — firecracker ("cracker") retail under
India's PESO (Petroleum and Explosives Safety Organisation) licensing
rules.

## Problem Statement
Licensed firecracker shops must keep their total explosive stock under
a fixed limit set by their PESO licence. In practice this is tracked on
paper, if at all, so a shop owner often only discovers they are near or
over the limit during an inspection — after the fact. There is no
simple way to check, at the moment stock is added, whether the shop is
still within its legal limit.

## Proposed Solution
A menu-driven C console application that:
- Records every stock batch (category, batch number, quantity, expiry).
- Rejects any addition or restock that would push total stock **over**
  the licensed limit, before it is committed.
- Classifies current stock as **SAFE**, **NEAR_LIMIT** (≥ 90% of the
  limit), or **EXCEEDED**.
- Generates stock, compliance and category-breakdown reports.
- Keeps an append-only transaction log of every add/sale/restock/removal.

## Features
- Add a stock batch, with duplicate-batch and licence-limit checks.
- Record a sale (reduces quantity, rejects over-selling).
- Restock an existing batch, with the same licence-limit check as adding.
- Remove a batch (e.g. fully sold or expired).
- Stock report, compliance report, and per-category breakdown.
- Data persists to disk between runs; every change is logged.

## Functional Requirements Summary
| ID | Requirement |
|---|---|
| FR-01 | The system shall add a new stock batch with category, batch number, quantity and expiry date. |
| FR-02 | The system shall reject a batch number that already exists. |
| FR-03 | The system shall reject an addition or restock that would exceed the licensed limit. |
| FR-04 | The system shall record a sale, reducing the item's quantity, and reject a sale larger than the remaining quantity. |
| FR-05 | The system shall restock an existing batch. |
| FR-06 | The system shall remove a batch by id. |
| FR-07 | The system shall report total stock, remaining capacity and compliance status (SAFE / NEAR_LIMIT / EXCEEDED). |
| FR-08 | The system shall report stock totals broken down by category. |
| FR-09 | The system shall persist stock data between runs and append every transaction to a log file. |
| FR-10 | The system shall validate all user input and reject invalid values without crashing. |

## Software Architecture
See `docs/architecture.png` and `docs/detailed_design.md` for the full
design. In short: `main.c` orchestrates a menu loop; `validation.c`,
`stock.c`, `compliance.c` and `report.c` are independent logic modules;
`file_handler.c` is the only module that touches disk.

## Module Descriptions
| Module | Responsibility |
|---|---|
| `main.c` | Menu loop and control flow only |
| `stock.c` / `stock.h` | Inventory data structures and CRUD operations |
| `validation.c` / `validation.h` | Pure input/data validation functions |
| `compliance.c` / `compliance.h` | Stock-vs-licence-limit decision logic |
| `file_handler.c` / `file_handler.h` | Checked file I/O (load/save/append) |
| `report.c` / `report.h` | Formatted console reports |

## Folder Structure
```
cracker-stock-compliance/
├── src/            C source files
├── include/        Header files
├── tests/          gTest unit tests
├── data/           Runtime data + sample_data.txt reference
├── docs/           detailed_design.md, architecture.png
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
```

## C Concepts Used
Structures (`StockItem`, `ShopInfo`, `Inventory`), enumerations
(`CrackerCategory`, `StockStatus`, `OperationStatus`), functions with
clear parameters/return values, fixed-size arrays, string handling,
constants (no magic numbers), file handling (`fopen`/`fgets`/`fprintf`/
`fclose`), and status/error codes instead of ignored return values.

## Coding Standards Followed
4-space indentation, one statement per line, consistent brace
placement, `UPPER_CASE` constants, `camelCase` functions/variables
describing an action (`addStockItem`, `determineStockStatus`), header
guards, no magic numbers, every file/function documented with a header
comment. Compiles cleanly with `-Wall -Wextra -Wpedantic`.

## Prerequisites
- `gcc` and `g++` (C11 / C++14)
- `make`
- Google Test (`libgtest-dev`, or any setup providing `gtest/gtest.h`
  and `-lgtest -lgtest_main`) — **only needed for `make test`**; the
  application itself (`make all` / `make run`) needs no extra
  dependencies.

On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y build-essential libgtest-dev
```
(On some distributions `libgtest-dev` ships source only; if `make test`
can't find `-lgtest`, build it once with
`cd /usr/src/googletest && cmake . && make && sudo make install`.)

## Build Instructions
```bash
make all
```
Builds the application with strict warnings (`-Wall -Wextra -Wpedantic`)
into `bin/tracker`. The build produced **zero warnings** in development.

## Run Instructions
```bash
make run
```
or directly:
```bash
./bin/tracker
```
On first run, no licence is configured yet, so the app creates
`data/shop_config.txt` with placeholder values and tells you to edit it
with your actual licence number, owner name and licensed limit (kg).

## Test Instructions
```bash
make test
```
Builds `bin/run_tests` (linking the real `src/*.c` logic, excluding
`main.c`, against the gTest files in `tests/`) and runs it.

> **Development note:** this project was built in a sandboxed session
> without network access, so `libgtest-dev` could not be installed
> there to execute `make test` directly. The 44 test cases were instead
> verified by compiling the same assertions against the real
> implementation with a plain-assert harness (all 68 individual checks
> passed) before being written as proper `TEST`/`TEST_F` gTest cases.
> `make test` will run them as-is on any machine with gTest installed.

## Sample Input / Output
```
$ make run
No shop configuration found. Using default licence settings.
Edit data/shop_config.txt to set your actual licence details.
Welcome, Unset Owner (Licence: PESO-UNSET). Licensed limit: 100.00 kg.

============ Firecracker Stock Compliance Tracker ============
1. Add new stock batch
2. Record a sale
3. Restock an existing batch
4. Remove a batch (e.g. fully sold / expired)
5. View stock report
6. View compliance report
7. View category breakdown
8. Save and exit
Enter choice [1-8]: 1
Category (0=Sparklers 1=Flowerpots 2=Chakris 3=Rockets 4=Bombs 5=Other): 0
Batch number (alphanumeric/-, max 15 chars): SPK-001
Quantity (kg): 40
Expiry date (DD-MM-YYYY): 20-12-2026
Stock batch added successfully.
...
Enter choice [1-8]: 6

=================== COMPLIANCE REPORT ===================
Licence No.       : PESO-UNSET
Owner             : Unset Owner
Licensed Limit    : 100.00 kg
Current Stock     : 85.50 kg
Remaining Capacity: 14.50 kg
Compliance Status : SAFE
==========================================================
```
A licensed limit of 250 kg with ~108 kg held (per `data/sample_data.txt`)
reports **SAFE**; pushing stock to ≥ 90% of the limit reports
**NEAR_LIMIT**; any addition that would exceed the limit is rejected
outright with an error, before it is saved.

## Unit-Testing Summary
44 gTest cases across three suites (`tests/test_validation.cpp`,
`tests/test_compliance.cpp`, `tests/test_stock.cpp`), each covering
positive, negative and boundary cases — e.g. a quantity of exactly
`0.0` (invalid), the licence limit's exact NEAR_LIMIT threshold (90%),
selling exactly the remaining stock (valid), and filling the inventory
to exactly `MAX_ITEMS` before the next add is rejected. See
`docs/detailed_design.md` §10 for the full test plan and §11 for an
input-handling bug found and fixed during development.

## Error-Handling Summary
Every file operation and every stock operation returns an
`OperationStatus`, checked by the caller. Console input is read line by
line and validated before use; non-numeric or out-of-range input is
rejected with a message and re-prompted rather than crashing. A closed
input stream (EOF) causes the application to save and exit cleanly
instead of looping. See `docs/detailed_design.md` §8 and §11.

## Student Information
- **Student name:** Kumar Kulkarni
- **Roll number:** 612509024
- **Division/Branch:** Instrumentation and Control
- **Project title:** Firecracker Shop Licence and Stock Compliance Tracker
- **Email:** kulkarnikm25.instru@coeptech.ac.in
- **GitHub repository link:** https://github.com/kumarmkulkarni/Firecracker-Shop-Licence-and-Stock-Compliance-Tracker

## Known Limitations
- Single shop / single licence per running instance (no multi-shop support).
- Fixed-capacity inventory (`MAX_ITEMS = 200` batches at a time).
- No authentication — anyone running the binary has full access.
- Transaction log is append-only with no built-in rotation or report view.

## Future Enhancements
- Multi-shop support with a shop selector at startup.
- Searching/filtering stock by category or near-expiry date.
- CSV export of reports for external record-keeping.
- A configurable alert threshold instead of the fixed 90% NEAR_LIMIT ratio.

## GitHub Repository Link
https://github.com/kumarmkulkarni/Firecracker-Shop-Licence-and-Stock-Compliance-Tracker