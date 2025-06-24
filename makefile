CC = c++

##### Handle compiler flags, especially those for debugging.
##### See documentation below this section.

# Basic checks
DEBUG_FLAGS_COMMON := -g -DNOGO_DEBUG -DASSERT_RESTORE_DEBUG -DGAME_UNDO_DEBUG

# Additional checks that are expensive and/or annoying
DEBUG_FLAGS_EXTRA := -DSUMGAME_DEBUG -DTYPE_TABLE_DEBUG -DDEFAULT_IMPL_DEBUG -D_GLIBCXX_DEBUG

DEBUG_FLAGS_ALL := $(DEBUG_FLAGS_COMMON) $(DEBUG_FLAGS_EXTRA)

# Default flags for MCGS and MCGS_test
DEBUG_FLAGS_MCGS := $(DEBUG_FLAGS_COMMON)
DEBUG_FLAGS_MCGS_TEST := $(filter-out -DDEFAULT_IMPL_DEBUG,$(DEBUG_FLAGS_ALL))


### handle DEBUG and ASAN variables
ifneq (,$(filter $(DEBUG),0 false)) # DEBUG=0 or DEBUG=false
	DEBUG_FLAGS_MCGS := -DNDEBUG
endif

ifneq (,$(filter $(DEBUG),1 true)) # DEBUG=1 or DEBUG=true
	DEBUG_FLAGS_MCGS := $(DEBUG_FLAGS_ALL)
	DEBUG_FLAGS_MCGS_TEST := $(DEBUG_FLAGS_ALL)
endif

ifneq (,$(filter $(ASAN),leak address)) # ASAN=leak or ASAN=address
	ASAN_FLAGS := -g -fno-omit-frame-pointer -fsanitize=$(ASAN)
endif

NORMAL_FLAGS_BASE = -Wall --std=c++17 -O3 -pthread $(ASAN_FLAGS) $(DEBUG_FLAGS_MCGS)
TEST_FLAGS_BASE = -Wall --std=c++17 -O3 -pthread $(ASAN_FLAGS) $(DEBUG_FLAGS_MCGS_TEST) -DCLOBBER_SPLIT


#         Makefile Variables
#
#     DEBUG (undefined, 0, 1)
# Affects debugging flags used for MCGS and MCGS_test executables.
#
# Undefined: Use defaults.
# 0: For MCGS use -DNDEBUG and no debug flags. No effect for MCGS_test.
# 1: use all debug flags for both MCGS and MCGS_test.
#
#     ASAN (undefined, "leak", "address")
# Affects MCGS and MCGS_test executables.
#
# Undefined: don't use AddressSanitizer/LeakSanitizer.
# "leak": Link against LeakSanitizer to check for memory leaks.
# "address": Link against LeakSanitizer, and instrument code with
#     AddressSanitizer, to check for leaks and other memory errors.


#         Debugging Flags
#
#     -D_GLIBCXX_DEBUG
# std library debugging. Not supported by all compilers.
#
#     -DNOGO_DEBUG
# Make nogo and nogo_1xn validate the board in their constructors. Without this
# flag, an error will still be thrown on invalid user input, by file_parser.cpp.
#
#     -DASSERT_RESTORE_DEBUG
# Make assert_restore_game and similar classes do their checks.
#
#     -DSUMGAME_DEBUG
# Enable sumgame checks at the start of every minimax search step. Checks that
# all games are unique objects, and uses assert_restore_sumgame.
# Without this flag, assert_restore_sumgame is still used at the root of search.
#
#     -DTYPE_TABLE_DEBUG
# Make the i_type_table::type_table() method check that the cached
# type_table_t* is correct. It could be incorrect if type_table() is called
# before the constructor of a class most derived type runs.
#
#     -DDEFAULT_IMPL_DEBUG
# Print warnings to stderr whenever a game relies on a default implementation.
# i.e. strip::_init_hash(), game::_split_impl().

SRC_DIR = src
TEST_DIR = test

RELEASE_BUILD_DIR = build/release
TEST_BUILD_DIR = build/test

INC = -I. -I$(SRC_DIR)

NORMAL_FLAGS := $(NORMAL_FLAGS_BASE) $(INC)
TEST_FLAGS := $(TEST_FLAGS_BASE) $(INC)

# args: files, directory prefix, file extension
FN_OUTPATH = \
$(addsuffix $(3), \
	$(addprefix $(2)/, \
		$(basename $(1)) \
	) \
)

# args: compiler flags
FN_USE_ALL_DEBUG_FLAGS = $(filter-out $(DEBUG_FLAGS_ALL),$(1)) $(DEBUG_FLAGS_ALL)
FN_TIDY_DEBUG_FLAGS = $(filter-out -D_GLIBCXX_DEBUG,$(call FN_USE_ALL_DEBUG_FLAGS,$(1)))

##### Target: MCGS
MCGS_SRC = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/main/*.cpp)
MCGS_SRC_H = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/main/*.h)
MCGS_OBJS := $(call FN_OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.o)
MCGS_DEPS := $(call FN_OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.d)

##### Target: MCGS_test
MCGS_TEST_SRC = $(wildcard $(SRC_DIR)/*.cpp $(TEST_DIR)/*.cpp)
MCGS_TEST_SRC_H = $(wildcard $(SRC_DIR)/*.h $(TEST_DIR)/*.h)
MCGS_TEST_OBJS := $(call FN_OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.o)
MCGS_TEST_DEPS := $(call FN_OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.d)

#### For linter tooling
ALL_SRC_FILES := $(MCGS_SRC) $(MCGS_SRC_H) $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H) 
ALL_SRC_FILES := $(sort $(ALL_SRC_FILES))

ALL_CPP_FILES := $(MCGS_SRC) $(MCGS_TEST_SRC)
ALL_CPP_FILES := $(sort $(ALL_CPP_FILES))


TIDY_CONFIG := .clang-tidy
TIDY_CONFIG_HEADERS := .clang-tidy-headers
FORMAT_SCRIPT := utils/format-files.py

GREP_FILES_BASE := $(wildcard .*) $(wildcard *)
GREP_FILES_EXCLUDE := . .. .cache .git
GREP_FILES := $(filter-out $(GREP_FILES_EXCLUDE),$(GREP_FILES_BASE))


.DEFAULT_GOAL := MCGS
.PHONY: clean test
.PHONY: tidy tidy_release tidy_test
.PHONY: format format_delete format_replace


CAN_BUILD=0

ifdef USE_FLAGS
	ifdef DEPS
		ifdef BUILD_DIR
			CAN_BUILD=1
		endif
	endif
endif

# Tidy targets
#$(eval LINT_FILES ?= $(ALL_SRC_FILES))

tidy:
	$(eval LINT_FILES ?= $(ALL_CPP_FILES))
	$(eval NORMAL_FLAGS := $(call FN_TIDY_DEBUG_FLAGS,$(NORMAL_FLAGS)))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

#$(eval LINT_FILES ?= $(MCGS_SRC) $(MCGS_SRC_H))
tidy_release:
	$(eval LINT_FILES ?= $(MCGS_SRC))
	$(eval NORMAL_FLAGS := $(call FN_TIDY_DEBUG_FLAGS,$(NORMAL_FLAGS)))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

#$(eval LINT_FILES ?= $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H))
tidy_test:
	$(eval LINT_FILES ?= $(MCGS_TEST_SRC))
	$(eval TEST_FLAGS := $(call FN_TIDY_DEBUG_FLAGS,$(TEST_FLAGS)))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(TEST_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

tidy_headers:
	$(eval LINT_FILES ?= $(ALL_SRC_FILES))
	$(eval LINT_FILES := $(filter %.h, $(LINT_FILES)))
	$(eval NORMAL_FLAGS := $(call FN_TIDY_DEBUG_FLAGS,$(NORMAL_FLAGS)))
	@clang-tidy --config-file=$(TIDY_CONFIG_HEADERS) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++-header 2>&1 | tee tidy_result.txt


# Format targets
format:
	$(eval LINT_FILES ?= $(ALL_SRC_FILES))
	@python3 $(FORMAT_SCRIPT) $(LINT_FILES)

format_delete:
	$(eval LINT_FILES ?= $(ALL_SRC_FILES))
	@python3 $(FORMAT_SCRIPT) --delete $(LINT_FILES)

format_replace:
	$(eval LINT_FILES ?= $(ALL_SRC_FILES))
	@python3 $(FORMAT_SCRIPT) --replace $(LINT_FILES)

# Grep targets
find_todo:
	grep -R -i -n "TODO" $(GREP_FILES)


ifeq ($(CAN_BUILD), 1)
MCGS: $(MCGS_OBJS)
	$(CC) $(USE_FLAGS) $^ -o $@

MCGS_test: $(MCGS_TEST_OBJS)
	$(CC) $(USE_FLAGS) $^ -o $@

else
.PHONY: MCGS MCGS_test

MCGS:
	$(MAKE) $@ USE_FLAGS="$(NORMAL_FLAGS)" DEPS="$(MCGS_DEPS)" BUILD_DIR="$(RELEASE_BUILD_DIR)"

MCGS_test:
	$(MAKE) $@ USE_FLAGS="$(TEST_FLAGS)" DEPS="$(MCGS_TEST_DEPS)" BUILD_DIR="$(TEST_BUILD_DIR)"

endif


# Simple targets
clean:
	-rm -r *.o main/*.o test/*.o MCGS MCGS_test MCGS_test.dSYM *.d main/*.d test/*.d
	-rm -rf build

test: MCGS_test
	./MCGS_test

test-fast: MCGS_test
	./MCGS_test --no-slow-tests


# Object file target
# TODO should this call mkdir like this? There's probably a better way
$(BUILD_DIR)/%.o: %.cpp
	-mkdir -p $(dir $@)
	$(CC) $(USE_FLAGS) -x c++ -MMD -MP -c $< -o $@


-include $(DEPS)

