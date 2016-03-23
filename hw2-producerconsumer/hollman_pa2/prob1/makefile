HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

default: pro_con

pro_con: pro_con.c $(HEADERS); gcc -o pro_con $(SOURCES) -lm -std=c99

clean:;-rm pro_con;