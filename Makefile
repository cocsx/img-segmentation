CC = g++
CFLAGS = -c -Wall -g -Os
LD = $(CC)
LDFLAGS =

TARGET = ./bin/kmeans

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard ./**/*.cpp))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)
