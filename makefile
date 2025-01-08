# TODO: do we need to generate .d files every build? 
# 	should we have ".td" dependency files built with testing flags?

CC = c++
NORMAL_FLAGS = -Wall --std=c++17 -O3
TEST_FLAGS = -Wall --std=c++17 -O3 -g
INC = -I.

#MCGS_FILES := *.cpp *.h main/*.cpp
#MCGS_TEST_FILES := *.cpp *.h test/*.cpp test/*.h

MCGS_SRC = $(wildcard *.cpp main/*.cpp)
MCGS_OBJS = $(MCGS_SRC:.cpp=.o)
MCGS_DEPS = $(MCGS_SRC:.cpp=.d)

MCGS_TEST_SRC = $(wildcard *.cpp test/*.cpp)
MCGS_TEST_OBJS = $(MCGS_TEST_SRC:.cpp=.o)
MCGS_TEST_DEPS = $(MCGS_TEST_SRC:.cpp=.d)


.DEFAULT_GOAL := MCGS
.PHONY: lc test clean


CAN_BUILD=0

ifdef USE_FLAGS
	ifdef DEPS
		CAN_BUILD=1
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
	make $@ USE_FLAGS="$(NORMAL_FLAGS)" DEPS="$(MCGS_DEPS)"

MCGS_test:
	make $@ USE_FLAGS="$(TEST_FLAGS)" DEPS="$(MCGS_TEST_DEPS)"
endif





clean:
	-rm -r *.o main/*.o test/*.o MCGS MCGS_test MCGS_test.dSYM *.d main/*.d test/*.d

test: MCGS_test
	./MCGS_test

lc: MCGS
	clear
	valgrind --leak-check=full ./MCGS "[clobber_1xn] XOXOXO {B}"


#%.d: %.cpp
#	$(CC) -MM $(USE_FLAGS) $(INC) $< > $@

#%.o: %.cpp %.h
#	$(CC) $(USE_FLAGS) $(INC) $< -o $@ -c



%.o: %.cpp
	$(CC) $(USE_FLAGS) $(INC) -MMD -MP -c $< -o $@


-include $(DEPS)

