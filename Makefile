main: main.cpp
	g++ -std=c++11 main.cpp -lasound -lm -pthread -o main

.PHONY: clean
clean:
	rm -f main
