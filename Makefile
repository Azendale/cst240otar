CFLAGS=-Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
CC = gcc

OBJS = otar \

all: otar

otar: otar.c
	$(CC) $(CFLAGS) otar.c -o otar

clean:
	rm -f $(OBJS)
