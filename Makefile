# Makefile for the Firecracker Shop Licence and Stock Compliance Tracker
#
# Targets:
#   make all    - build the application (bin/tracker)
#   make run    - build (if needed) and run the application
#   make test   - build (if needed) and run the gTest unit tests
#   make clean  - remove all generated files

CC       := gcc
CXX      := g++
CFLAGS   := -Wall -Wextra -Wpedantic -std=c11 -Iinclude
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++14 -Iinclude
LDFLAGS  :=
GTEST_LIBS := -lgtest -lgtest_main -pthread

SRC_DIR   := src
OBJ_DIR   := build
BIN_DIR   := bin
TEST_DIR  := tests

# Application sources/objects (everything in src/ except main.c is also
# needed by the test binary, so split main out separately).
APP_SRCS    := $(wildcard $(SRC_DIR)/*.c)
LOGIC_SRCS  := $(filter-out $(SRC_DIR)/main.c,$(APP_SRCS))
APP_OBJS    := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(APP_SRCS))
LOGIC_OBJS  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LOGIC_SRCS))

TARGET      := $(BIN_DIR)/tracker

TEST_SRCS   := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS   := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TEST_TARGET := $(BIN_DIR)/run_tests

.PHONY: all run test clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) data

$(TARGET): $(APP_OBJS)
	$(CC) $(APP_OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

test: dirs $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(LOGIC_OBJS) $(TEST_OBJS)
	$(CXX) $(LOGIC_OBJS) $(TEST_OBJS) -o $@ $(GTEST_LIBS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
