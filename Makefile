CC = gcc
CFLAGS = 

LDFLAGS = -lSDL2 -lSDL2_image -lzip

all: clean scomic

clean:
	rm -rf scomic *.o

scomic: scomic.o
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

scomic.o: scomic.c
	$(CC) $< $(CFLAGS) -c -o $@
