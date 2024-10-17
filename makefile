CC = c++
FLAGS = -Wall --std=c++17 -O3
TESTFLAGS = -Wall --std=c++17 -O3 -g
INC = -I.

MCGS: *.cpp *.h main/*.cpp
	$(CC) $(FLAGS) $(INC) *cpp main/main.cpp -o MCGS

MCGS_test: *.cpp *.h test/*.cpp test/*.h
	$(CC) $(TESTFLAGS) $(INC) *cpp test/*cpp -o MCGS_test

test: MCGS_test
	./MCGS_test

clean:
	-rm -r *.o main/*.o test/*.o MCGS MCGS_test MCGS_test.dSYM
