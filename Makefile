main: main.cpp
	g++ -std=c++11 main.cpp -lasound -lm -o main

pcm: pcm.c
	gcc pcm.c -lasound -lm -o pcm

midi: midi.c
	gcc midi.c -lasound -o midi

.PHONY: clean
clean:
	rm -f pcm midi main

