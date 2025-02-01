
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
MCGS_OBJS := $(call OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.o)
MCGS_DEPS := $(call OUTPATH,$(MCGS_SRC),$(RELEASE_BUILD_DIR),.d)

##### Target: MCGS_test
MCGS_TEST_SRC = $(wildcard $(SRC_DIR)/*.cpp $(TEST_DIR)/*.cpp)
MCGS_TEST_SRC_H = $(wildcard $(SRC_DIR)/*.h $(TEST_DIR)/*.h)
MCGS_TEST_OBJS := $(call OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.o)
MCGS_TEST_DEPS := $(call OUTPATH,$(MCGS_TEST_SRC),$(TEST_BUILD_DIR),.d)



.DEFAULT_GOAL := MCGS
.PHONY: clean test leakcheck leakcheck_test style format


CAN_BUILD=0

ifdef USE_FLAGS
	ifdef DEPS
		ifdef BUILD_DIR
			CAN_BUILD=1
		endif
	endif
endif



#STYLE_TEST_FILES = $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H)
#STYLE_TEST_FILES = src/main/main.cpp src/file_parser.* src/cli_options.* src/game.* style_test.cpp
STYLE_TEST_FILES = style_test.cpp

style:
	clang-tidy --config-file=clangTidyConfig $(STYLE_TEST_FILES) -- $(NORMAL_FLAGS) $(INC) -x c++


# TODO use diff and tr to make sure only whitespace has changed
format: $(MCGS_TEST_SRC) $(MCGS_TEST_SRC_H)
	-rm diffResult.txt
	- ! for x in $^ ; do \
		! echo "============"$$x"===============" ; \
		! clang-format --style="file:clangFormatConfig" "backup/"$$x > $$x || true ; \
		! diff "backup/"$$x $$x >> diffResult.txt || true ; \
	done


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

