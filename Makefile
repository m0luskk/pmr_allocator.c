CC = gcc
CFLAGS = -g -std=c99 -Wall -Wextra -pedantic -Iinclude \
         -fsanitize=undefined -fsanitize=address -fsanitize=leak \
         -fstrict-aliasing -Wstrict-aliasing
LDFLAGS = -fsanitize=undefined -fsanitize=address -fsanitize=leak -lcheck -lsubunit -lm

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

SOURCES = #$(wildcard $(SRC_DIR)/*.c)

LIB_OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_TARGETS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

.PHONY: all tests clean

all: $(LIB_OBJECTS) tests

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%: $(TEST_DIR)/%.c $(LIB_OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(LIB_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $@

tests: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do \
		echo "Запуск $$test..."; \
		$$test; \
	done

clean:
	rm -rf $(BUILD_DIR)