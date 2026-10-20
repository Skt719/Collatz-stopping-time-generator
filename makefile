CC = g++

# C++17 standard, enable all warnings, enable extra warnings, enable debugging information,
# enable pedantic warnings, and link with pthread library
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Wpedantic -pthread

TARGET = mt-collatz


mt-collatz: mt-collatz.cpp 
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: clean

# Build the program.
#make

# Run with N=100 and 4 threads.
# Save the histogram on stdout to histogram.csv and timing on stderr to timing.csv.
#./mt-collatz 100 4 > histogram.csv 2> timing.csv

# Run one locked benchmark and append its timing row to timings.csv.
#./mt-collatz 1000000 4 > /dev/null 2>> results.csv

# Run one unlocked benchmark and append its timing row to timings.csv.
#./mt-collatz 1000000 4 -nolock > /dev/null 2>> results.csv