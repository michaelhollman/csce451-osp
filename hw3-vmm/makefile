HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

default: a

a: a.c $(HEADERS); gcc -pthread -o a.out $(SOURCES) -lm -std=c99

clean:;-rm a.out;