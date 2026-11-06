LIB_NAME = demantoid
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build

LIBPQ_INC := $(shell ./scripts/get-libpg-path.sh | grep INCLUDEDIR | cut -d= -f2)
LIBPQ_LIB := $(shell ./scripts/get-libpg-path.sh | grep LIBDIR | cut -d= -f2)

CFLAGS += -I$(LIBPQ_INC)
LDFLAGS += -L$(LIBPQ_LIB) -lpq

CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
PIC_FLAGS = -fPIC

TEST_C = tests/test.c
TEST_DE_C = tests/de.c
TEST_CPP = tests/test.cpp
TEST_PY = tests/test.py

OBJ = $(BUILD_DIR)/$(LIB_NAME).o
LIB_A = $(BUILD_DIR)/lib$(LIB_NAME).a
LIB_SO = $(BUILD_DIR)/lib$(LIB_NAME).so
HEADER = $(INCLUDE_DIR)/$(LIB_NAME).h

# Library
$(LIB_A): $(OBJ)
	@$(AR) rcs $@ $^

$(LIB_SO): $(OBJ)
	@$(CC) -shared -o $@ $^ $(LDFLAGS)

$(OBJ): $(SRC_DIR)/$(LIB_NAME).c $(HEADER) | $(BUILD_DIR)
	@$(CC) $(CFLAGS) $(PIC_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

.PHONY: lib lib-static lib-shared
lib: lib-static lib-shared
lib-static: $(LIB_A)
lib-shared: $(LIB_SO)

# Tests
.PHONY: test test-c test-cpp test-python
.NOTPARALLEL: test

$(BUILD_DIR)/test_c: $(TEST_C) $(BUILD_DIR)/de_c.o $(LIB_A)
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $(TEST_C) $(BUILD_DIR)/de_c.o $(LIB_A) $(LDFLAGS)

$(BUILD_DIR)/de_c.o: $(TEST_DE_C) $(HEADER) | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/test_cpp: $(TEST_CPP) $(BUILD_DIR)/de_c.o $(LIB_A)
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $(TEST_CPP) $(BUILD_DIR)/de_c.o $(LIB_A) $(LDFLAGS)

test-c: $(BUILD_DIR)/test_c
	@./$(BUILD_DIR)/test_c
test-cpp: $(BUILD_DIR)/test_cpp
	@./$(BUILD_DIR)/test_cpp
test-python: $(TEST_PY) $(LIB_SO)
	@PYTHONPATH="$(CURDIR)/python" LD_LIBRARY_PATH="$(CURDIR)/$(BUILD_DIR)" python3 "$(TEST_PY)"

test: test-c test-cpp test-python

# Scripts
.PHONY: run-timescaledb install-dependencies generate-compile-flags
run-timescaledb: scripts/timescaledb.sh
	@./scripts/timescaledb.sh

install-dependencies: scripts/install-dependencies.sh
	@./scripts/install-dependencies.sh

generate-compile-flags: scripts/generate-compile-flags.sh
	@./scripts/generate-compile-flags.sh
