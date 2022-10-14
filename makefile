CC=gcc

CFLAGS=-O3 -Wall -Wextra
OFLAGS=-O3 -Wall -Wextra
LFLAGS=

EXEC=evlc
BUILD_DIR=out
SRC_DIR=src/c
INC_DIRS=src/header

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

.PHONY: clean debug

clean:
	rm -f -r $(BUILD_DIR)/*
	rm -f $(EXEC)