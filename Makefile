CC = gcc
CFLAGS = 

LDFLAGS = -lSDL2 -lSDL2_image -lzip

OBJS = scomic.o draw.o file.o common.o

all: clean scomic

clean:
	rm -rf scomic *.o

scomic: $(OBJS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

scomic.o: scomic.c
	$(CC) $< $(CFLAGS) -c -o $@

draw.o: draw.c
	$(CC) $< $(CFLAGS) -c -o $@

file.o: file.c
	$(CC) $< $(CFLAGS) -c -o $@

common.o: common.c
	$(CC) $< $(CFLAGS) -c -o $@
