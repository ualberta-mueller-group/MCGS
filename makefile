CC = c++
FLAGS = -Wall --std=c++17 -O3

MCGS:
	$(CC) $(FLAGS) *cpp main/main.cpp -o MCGS

test:
	$(CC) $(FLAGS) *cpp test/*cpp -o MCGS_test

clean:
	-rm *.o test/*.o MCGS
