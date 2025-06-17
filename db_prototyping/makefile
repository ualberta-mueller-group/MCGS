.PHONY: run, clean


run:
	-@rm test
	#g++ serialization_test.cpp -o test --std=c++17 -O3 -DNDEBUG
	g++ serialization_test.cpp -o test --std=c++17 -O3
	./test

clean:
	-rm test data.bin
