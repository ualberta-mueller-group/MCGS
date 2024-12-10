CC = c++
NORMAL_FLAGS = -Wall --std=c++17 -O3
TEST_FLAGS = -Wall --std=c++17 -O3 -g
INC = -I.

MCGS_FILES := *.cpp *.h main/*.cpp
MCGS_TEST_FILES := *.cpp *.h test/*.cpp test/*.h

# Given a string of .h and .cpp files, keep only .cpp files, then
# replace ".cpp" with ".o"
define getObjs
	$(eval DEPS_CPP = $(shell echo $(1) | grep -o "[^ ]*\.cpp"))
	$(eval DEPS_OBJ = $(shell echo $(DEPS_CPP) | sed "s/\.cpp/.o/g"))
	$(DEPS_OBJ)
endef

# TODO: are these evaluated lazily, or are these both evaluated here?
MCGS_OBJS := $(call getObjs, $(MCGS_FILES))
MCGS_TEST_OBJS := $(call getObjs, $(MCGS_TEST_FILES))


MCGS: USE_FLAGS = $(NORMAL_FLAGS)
MCGS: $(MCGS_OBJS)
	$(CC) $(USE_FLAGS) $(INC) $^ -o $@

MCGS_test: USE_FLAGS = $(TEST_FLAGS)
MCGS_test: $(MCGS_TEST_OBJS)
	$(CC) $(USE_FLAGS) $(INC) $^ -o $@

test: MCGS_test
	./MCGS_test



%.o: %.cpp %.h
	$(CC) $(USE_FLAGS) $(INC) $< -o $@ -c

%.o: %.cpp
	$(CC) $(USE_FLAGS) $(INC) $< -o $@ -c



clean:
	-rm -r *.o main/*.o test/*.o MCGS MCGS_test MCGS_test.dSYM
