.PHONY: run, clean


FLAGS := -I.

run:
	-rm test
	g++ serialization_test.cpp -o test --std=c++17 -O3 -DNDEBUG $(FLAGS)
	#g++ serialization_test.cpp -o test --std=c++17 -O3
	./test

clean:
	-rm test data.bin
