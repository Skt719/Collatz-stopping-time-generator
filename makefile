CC = g++
CFLAGS = -Wall -Wextra -g

TARGET = mt-collatz

OBJS = mt-collatz.o collatz.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

mt-collatz.o: mt-collatz.cpp collatz.hpp
	$(CC) $(CFLAGS) -c mt-collatz.cpp

collatz.o: collatz.cpp collatz.hpp
	$(CC) $(CFLAGS) -c collatz.cpp

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

	.PHONY: all run clean

