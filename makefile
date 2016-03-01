HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

default: osh

osh: osh.c $(HEADERS); gcc -o osh $(SOURCES) -lm -std=c99

clean:;-rm osh;