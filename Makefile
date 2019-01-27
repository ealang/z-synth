SRC = $(wildcard src/*.cpp src/*.h)

TO_OBJS = $(patsubst %.cpp, obj/%.o, $(1))
OBJS = $(call TO_OBJS, $(SRC))

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -g -Wall -Wextra -Werror -Wfatal-errors -c -std=c++11 -I src -o $@ $^

# main: main.cpp
#	g++ -std=c++11 main.cpp -lasound -lm -pthread -o main
main: $(OBJS)
	g++ -std=c++11 -lasound -lm -pthread -o $@ $^

.PHONY: clean
clean:
	rm -rf obj
	rm -f main
