GIT_VERSION := $(shell git describe --abbrev=16 --dirty="-uncommitted" --always --tags)
CFLAGS=-Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -DGIT_VERSION=\"$(GIT_VERSION)\" -std=gnu99 -g -O0
CC = gcc

OBJS = otar \

all: otar

otar: otar.c
	$(CC) $(CFLAGS) otar.c -o otar

clean:
	rm -f $(OBJS)
