HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

default: osh

osh: osh.c $(HEADERS); gcc -o osh $(SOURCES) -lm

clean:;-rm osh;