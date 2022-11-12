CC = g++
CFLAGS = -c -Wall -g -O3 -lSDL2
LD = $(CC)
LDFLAGS = -lSDL2

TARGET = ./bin/kmeans

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard ./**/*.cpp))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)
