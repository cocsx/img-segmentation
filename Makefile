CC = g++
CFLAGS = -c -Wall -g -O3 -fopt-info-vec-all
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
