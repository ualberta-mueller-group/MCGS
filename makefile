CC = c++
FLAGS = -Wall --std=c++17 -O3
INC = -I.

MCGS: *.cpp *.h main/*.cpp
	$(CC) $(FLAGS) $(INC) *cpp main/main.cpp -o MCGS

test: *.cpp *.h test/*.cpp test/*.h
	$(CC) $(FLAGS) $(INC) *cpp test/*cpp -o MCGS_test
	./MCGS_test

clean:
	-rm *.o main/*.o test/*.o MCGS
