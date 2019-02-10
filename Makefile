MAIN_SRC = $(wildcard src/*.cpp src/elements/*.cpp src/pipeline/*.cpp)
TEST_SRC = $(wildcard test/pipeline/*.cpp)

TO_OBJS = $(patsubst %.cpp, obj/%.o, $(1))
MAIN_OBJS = $(call TO_OBJS, $(MAIN_SRC))
TEST_OBJS = $(call TO_OBJS, $(TEST_SRC) $(filter-out src/main.cpp, $(MAIN_SRC)))

MAIN_LIBS = -lasound -lm -pthread 
TEST_LIBS = $(MAIN_LIBS) -lgtest -lgtest_main

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -g -O3 -Wall -Wextra -Werror -Wfatal-errors -c -std=c++11 -I src -o $@ $^

z-synth: $(MAIN_OBJS)
	g++ -std=c++11 $^ $(MAIN_LIBS) -o $@

z-synth-test: $(TEST_OBJS)
	g++ -std=c++11 $^ $(TEST_LIBS) -o $@

.PHONY: all
all: z-synth z-synth-test

.PHONY: clean
clean:
	rm -rf obj
	rm -f z-synth z-synth-test
