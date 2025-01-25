
CC = c++
NORMAL_FLAGS = -Wall --std=c++17 -O3
TEST_FLAGS = -Wall --std=c++17 -O3 -g

SRC_DIR = src
TEST_DIR = test

RELEASE_BUILD_DIR = build/release
TEST_BUILD_DIR = build/test


INC = -I. -I$(SRC_DIR)

# TODO use a function for the following lines, they're very error prone

##### Target: MCGS

# .cpp files
MCGS_SRC = $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/main/*.cpp)

# .o files
MCGS_OBJS := $(basename $(MCGS_SRC))
MCGS_OBJS := $(addsuffix .o, $(MCGS_OBJS))
MCGS_OBJS := $(addprefix $(RELEASE_BUILD_DIR)/, $(MCGS_OBJS))

# .d files
MCGS_DEPS := $(basename $(MCGS_SRC))
MCGS_DEPS := $(addsuffix .d, $(MCGS_DEPS))
MCGS_DEPS := $(addprefix $(RELEASE_BUILD_DIR)/, $(MCGS_DEPS))

##### Target: MCGS_test

# .cpp files
MCGS_TEST_SRC = $(wildcard $(SRC_DIR)/*.cpp $(TEST_DIR)/*.cpp)

# .o files
MCGS_TEST_OBJS := $(basename $(MCGS_TEST_SRC))
MCGS_TEST_OBJS := $(addsuffix .o, $(MCGS_TEST_OBJS))
MCGS_TEST_OBJS := $(addprefix $(TEST_BUILD_DIR)/, $(MCGS_TEST_OBJS))

# .d files
MCGS_TEST_DEPS := $(basename $(MCGS_TEST_SRC))
MCGS_TEST_DEPS := $(addsuffix .d, $(MCGS_TEST_DEPS))
MCGS_TEST_DEPS := $(addprefix $(TEST_BUILD_DIR)/, $(MCGS_TEST_DEPS))


.DEFAULT_GOAL := MCGS
.PHONY: clean test leakcheck leakcheck_test


CAN_BUILD=0

ifdef USE_FLAGS
	ifdef DEPS
		ifdef BUILD_DIR
			CAN_BUILD=1
		endif
	endif
endif




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

leakcheck: MCGS
	clear
	valgrind --leak-check=full ./MCGS "[clobber_1xn] XOXOXO {B}"

leakcheck_test: MCGS_test
	clear
	valgrind --leak-check=full ./MCGS_test


#%.d: %.cpp
#	$(CC) -MM $(USE_FLAGS) $(INC) $< > $@

#%.o: %.cpp %.h
#	$(CC) $(USE_FLAGS) $(INC) $< -o $@ -c



# TODO should this call mkdir like this? There's probably a better way
$(BUILD_DIR)/%.o: %.cpp
	-mkdir -p $(dir $@)
	$(CC) $(USE_FLAGS) $(INC) -MMD -MP -c $< -o $@


-include $(DEPS)

