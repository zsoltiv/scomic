CC = gcc
CFLAGS = -O2

LDFLAGS = -lSDL2 -lSDL2_image -lzip

SOURCES := $(wildcard ./*.c)
OBJECTS := $(patsubst ./%.c, ./%.o, $(SOURCES))

scomic: $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -rf scomic *.o
