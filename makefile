
CC = c++
NORMAL_FLAGS = -Wall --std=c++17 -O3
TEST_FLAGS = -Wall --std=c++17 -O3 -g


# Valgrind is too slow for even short computations. Instead add: -fsanitize=leak
# as a flag when compiling. Should work for clang++ and g++
# Still slows down executation considerably, but not nearly as much

SRC_DIR = src
TEST_DIR = test

RELEASE_BUILD_DIR = build/release
TEST_BUILD_DIR = build/test


INC = -I. -I$(SRC_DIR)

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

#### For linter tools
ALL_SRC_FILES := $(MCGS_SRC) $(MCGS_SRC_H) $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H) 
ALL_SRC_FILES := $(sort $(ALL_SRC_FILES))

LINT_FILES ?= $(ALL_SRC_FILES)



.DEFAULT_GOAL := MCGS
.PHONY: clean test leakcheck leakcheck_test tidy format


CAN_BUILD=0

ifdef USE_FLAGS
	ifdef DEPS
		ifdef BUILD_DIR
			CAN_BUILD=1
		endif
	endif
endif


tidy:
	clang-tidy --config-file=clangTidyConfig $(LINT_FILES) -- $(NORMAL_FLAGS) $(INC) -x c++ 2>&1 | tee tidy_result.txt

format:
	@python3 format_files.py $(LINT_FILES)

format_delete:
	@python3 format_files.py --delete $(LINT_FILES)

format_apply:
	@python3 format_files.py --apply $(LINT_FILES)

ifeq ($(CAN_BUILD), 1)

MCGS: $(MCGS_OBJS)
	$(CC) $(USE_FLAGS) $(INC) $^ -o $@

MCGS_test: $(MCGS_TEST_OBJS)
	$(CC) $(USE_FLAGS) $(INC) $^ -o $@



else
.PHONY: MCGS MCGS_test

MCGS:
	make $@ USE_FLAGS="$(NORMAL_FLAGS)" DEPS="$(MCGS_DEPS)" BUILD_DIR="$(RELEASE_BUILD_DIR)"

MCGS_test:
	make $@ USE_FLAGS="$(TEST_FLAGS)" DEPS="$(MCGS_TEST_DEPS)" BUILD_DIR="$(TEST_BUILD_DIR)"
endif



clean:
	-rm -r *.o main/*.o test/*.o MCGS MCGS_test MCGS_test.dSYM *.d main/*.d test/*.d
	-rm -rf build

test: MCGS_test
	./MCGS_test

test-fast: MCGS_test
	./MCGS_test --no-slow-tests


#%.d: %.cpp
#	$(CC) -MM $(USE_FLAGS) $(INC) $< > $@

#%.o: %.cpp %.h
#	$(CC) $(USE_FLAGS) $(INC) $< -o $@ -c



# TODO should this call mkdir like this? There's probably a better way
$(BUILD_DIR)/%.o: %.cpp
	-mkdir -p $(dir $@)
	$(CC) $(USE_FLAGS) -x c++ $(INC) -MMD -MP -c $< -o $@


-include $(DEPS)

