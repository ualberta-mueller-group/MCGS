CC = c++
#NORMAL_FLAGS_BASE = -Wall --std=c++17 -O3 -DNDEBUG -DNO_WARN_DEFAULT_IMPL
NORMAL_FLAGS_BASE = -Wall --std=c++17 -O3
TEST_FLAGS_BASE = -Wall --std=c++17 -O3 -g -DSUMGAME_DEBUG_EXTRA

# i.e. "make MCGS LEAKCHECK=1" or "make MCGS LEAKCHECK=true"
ifneq (,$(filter $(LEAKCHECK),1 true))
	NORMAL_FLAGS_BASE := $(NORMAL_FLAGS_BASE) -fsanitize=leak
	TEST_FLAGS_BASE := $(TEST_FLAGS_BASE) -fsanitize=leak
endif

# Valgrind is too slow for even short computations. Instead use: -fsanitize=leak
# as a flag when compiling. Should work for clang++ and g++
# Still slows down executation considerably, but not nearly as much

SRC_DIR = src
TEST_DIR = test

RELEASE_BUILD_DIR = build/release
TEST_BUILD_DIR = build/test

INC = -I. -I$(SRC_DIR)

NORMAL_FLAGS := $(NORMAL_FLAGS_BASE) $(INC)
TEST_FLAGS := $(TEST_FLAGS_BASE) $(INC)

# args: files, directory prefix, file extension
OUTPATH = \
$(addsuffix $(3), \
	$(addprefix $(2)/, \
		$(basename $(1)) \
	) \
)

##### Target: MCGS
MCGS_SRC = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/main/*.cpp)
MCGS_SRC_H = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/main/*.h)
MCGS_OBJS := $(call OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.o)
MCGS_DEPS := $(call OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.d)

##### Target: MCGS_test
MCGS_TEST_SRC = $(wildcard $(SRC_DIR)/*.cpp $(TEST_DIR)/*.cpp)
MCGS_TEST_SRC_H = $(wildcard $(SRC_DIR)/*.h $(TEST_DIR)/*.h)
MCGS_TEST_OBJS := $(call OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.o)
MCGS_TEST_DEPS := $(call OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.d)

#### For linter tooling
ALL_SRC_FILES := $(MCGS_SRC) $(MCGS_SRC_H) $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H) 
ALL_SRC_FILES := $(sort $(ALL_SRC_FILES))

ALL_CPP_FILES := $(MCGS_SRC) $(MCGS_TEST_SRC)
ALL_CPP_FILES := $(sort $(ALL_CPP_FILES))


TIDY_CONFIG := .clang-tidy
TIDY_CONFIG_HEADERS := .clang-tidy-headers
FORMAT_SCRIPT := utils/format-files.py



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


# NOTE: clang-tidy should be invoked on cpp files

# Tidy targets
#$(eval LINT_FILES ?= $(ALL_SRC_FILES))
tidy:
	$(eval LINT_FILES ?= $(ALL_CPP_FILES))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

#$(eval LINT_FILES ?= $(MCGS_SRC) $(MCGS_SRC_H))
tidy_release:
	$(eval LINT_FILES ?= $(MCGS_SRC))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

#$(eval LINT_FILES ?= $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H))
tidy_test:
	$(eval LINT_FILES ?= $(MCGS_TEST_SRC))
	@clang-tidy --config-file=$(TIDY_CONFIG) $(LINT_FILES) -- $(TEST_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt

tidy_header_functions:
	$(eval LINT_FILES ?= $(ALL_SRC_FILES))
	$(eval LINT_FILES := $(filter %.h, $(LINT_FILES)))
	@clang-tidy --config-file=$(TIDY_CONFIG_HEADERS) $(LINT_FILES) -- $(NORMAL_FLAGS)  -x c++ 2>&1 | tee tidy_result.txt


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


ifeq ($(CAN_BUILD), 1)
MCGS: $(MCGS_OBJS)
	$(CC) $(USE_FLAGS) $^ -o $@

MCGS_test: $(MCGS_TEST_OBJS)
	$(CC) $(USE_FLAGS) $^ -o $@

else
.PHONY: MCGS MCGS_test

MCGS:
	make $@ USE_FLAGS="$(NORMAL_FLAGS)" DEPS="$(MCGS_DEPS)" BUILD_DIR="$(RELEASE_BUILD_DIR)"

MCGS_test:
	make $@ USE_FLAGS="$(TEST_FLAGS)" DEPS="$(MCGS_TEST_DEPS)" BUILD_DIR="$(TEST_BUILD_DIR)"

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

