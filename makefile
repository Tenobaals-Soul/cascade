CC=gcc

CFLAGS=-O3 -Wall -Wextra
OFLAGS=-O3 -Wall -Wextra
TFLAGS=-Wall -Wextra
LFLAGS=

EXEC=evlc
BUILD_DIR=out
SRC_DIR=src/c
INC_DIRS=src/header
TEST_DIR=tests
TEST_CORE_DIR=test_core

DEBUG_FLAGS = -DDEBUG -g

all: remove_executables $(EXEC)

remove_executables:
	@rm -f $(EXEC)

set_debug: 
	$(eval CFLAGS += $(DEBUG_FLAGS))

set_no_opt: 
	$(eval CFLAGS = -O0 -Wall -Wextra -DDEBUG -g)

debug: set_debug all

no_opt: set_no_opt all

SRCS=$(shell find $(SRC_DIR) -name '*.c')
OBJS=$(SRCS:$(SRC_DIR)/%=$(BUILD_DIR)/%.o)

INC_FLAGS=$(addprefix -I,$(INC_DIRS))

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LFLAGS)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OFLAGS) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

$(TEST_CORE_DIR)/test_core.so: $(TEST_CORE_DIR)/test_core.c
	$(CC) $(CFLAGS) -I$(TEST_CORE_DIR) -c $(TEST_CORE_DIR)/test_core.c -o $@

tests/*: $(OBJS) $(TEST_CORE_DIR)/test_core.so set_debug
	$(CC) $(CFLAGS) $(INC_FLAGS) $(OBJS) $(TEST_CORE_DIR)/test_core.so -I$(TEST_CORE_DIR) $@ -o $(TEST_DIR)/current_test

tests:
	@rm -f $(TEST_DIR)/current_test
	@for file in $(TEST_DIR)/* ; do \
		make $${file} ; \
		valgrind $(TEST_DIR)/current_test ; \
		rm -f $(TEST_DIR)/current_test ; \
	done

.PHONY: clean debug tests

clean:
	rm -f -r $(BUILD_DIR)/*
	rm -f $(EXEC)