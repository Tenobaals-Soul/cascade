#
# Directories
#
SRC_DIR = src/c
INC_DIR = src/header

#
# Compiler flags
#
CC     = gcc
CFLAGS = -Wall -Wextra -Werror

#
# Project files
#
SRCS=$(notdir $(shell find $(SRC_DIR) -name '*.c'))
OBJS = $(SRCS:.c=.o)
EXE  = cascadec

#
# Debug build settings
#
DBGDIR = debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#
RELDIR = release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O3 -DNDEBUG

#
# Linker and macro Settings
#
LFLAGS = 
METAFLAGS = $(addprefix -I, $(INC_DIR)) $(addprefix -D, $(DEFINES)) $(addprefix -l, $(LFLAGS))

.PHONY: all clean debug prep release remake

# Default build
all: prep release

#
# Debug rules
#
debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJS)
	$(CC) $(CFLAGS) $(METAFLAGS) $(DBGCFLAGS) -o $(DBGEXE) $^
	@mv $(DBGEXE) ./$(EXE)

$(DBGDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $(METAFLAGS) $(DBGCFLAGS) -o $@ $<

#
# Release rules
#
release: $(RELEXE)

$(RELEXE): $(RELOBJS)
	$(CC) $(CFLAGS) $(METAFLAGS) $(RELCFLAGS) -o $(RELEXE) $^

$(RELDIR)/%.o: $(SRC_DIR)%.c
	$(CC) -c $(CFLAGS) $(METAFLAGS) $(RELCFLAGS) -o $@ $<
	@mv $(DBGEXE) ./$(EXE)

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR) $(RELDIR)

#
# test rules
#

TEST_DIR = tests
TEST_CORE_DIR = test_core

$(TEST_DIR)/%.elf: $(TEST_DIR)/%.c $(DBGOBJS) $(TEST_CORE_DIR)/test_core.c
	$(CC) $(CFLAGS) $(METAFLAGS) $(DBGCFLAGS) $(DBGOBJS) $(TEST_CORE_DIR)/test_core.c -I$(TEST_CORE_DIR) $< -o $@

test:
	@for file in $(TEST_DIR)/*.c ; do \
		target="$${file%%.*}".elf ; \
		make $${target} && \
		./$${target} ; \
		rm -f /$${target} ; \
	done

test-valgrind:
	@for file in $(TEST_DIR)/*.c ; do \
		target="$${file%%.*}".elf ; \
		make $${target} && \
		valgrind ./$${target} ; \
		rm -f /$${target} ; \
	done

remake: clean all

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS) $(TEST_DIR)/*.test