HEADERS = $(wildcard *.h)

default: osh

osh: osh.c $(HEADERS); gcc $< -o $@

clean:; -rm -f osh;