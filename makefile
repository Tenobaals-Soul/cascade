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

$(TEST_CORE_DIR)/test_core.so: $(TEST_CORE_DIR)/test_core.c set_debug
	$(CC) $(CFLAGS) -I$(TEST_CORE_DIR) -shared -fPIC $(TEST_CORE_DIR)/*.c -o $@

$(TEST_DIR)/%.elf: $(TEST_DIR)/%.c $(OBJS) $(TEST_CORE_DIR)/test_core.c set_debug set_no_opt
	$(CC) $(CFLAGS) $(INC_FLAGS) $(OBJS) $(TEST_CORE_DIR)/test_core.c -I$(TEST_CORE_DIR) $< -o $@

test:
	@for file in $(TEST_DIR)/*.c ; do \
		target="$${file%%.*}".elf ; \
		echo $${target} ; \
		make $${target} ; \
		./$${target} ; \
		rm -f /$${target} ; \
	done

test-valgrind:
	@for file in $(TEST_DIR)/*.c ; do \
		target="$${file%%.*}".elf ; \
		make $${target} ; \
		valgrind ./$${target} ; \
		rm -f /$${target} ; \
	done

.PHONY: clean debug test test-valgrind

clean:
	rm -f -r $(BUILD_DIR)/*
	rm -f $(EXEC)
	rm -f tests/*.elf