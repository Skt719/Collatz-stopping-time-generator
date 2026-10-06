# Collatz-stopping-time-generator

The goal of this assignment is to build a multi-threaded C++ program named mt-collatz. The program computes the Collatz stopping time for every integer from 1 to N using T concurrent threads. You will measure execution time to evaluate parallel performance and implement mutex synchronization to handle race conditions on shared data.

Core Program Specifications
1. Input & Command-Line Arguments
Your executable must accept two required arguments and one optional flag:

Bash
./mt-collatz <N> <T> [-nolock]
N: The upper limit of the range of numbers (1 through N) to compute.

T: The number of worker threads to launch.

-nolock (optional): Disables mutex locking when accessing shared memory, allowing you to observe race conditions and measure lock overhead.

2. Execution Logic & Race Conditions
Shared State:

A global COUNTER initialized to 1 (tracks the next number to assign to a thread).

A global histogram array of size 1001 initialized to 0 (stores frequency of stopping times from 0 to 1000).

Thread Job Loop:

Safely retrieve and increment COUNTER until COUNTER > N.

Compute the Collatz stopping time for that number (the number of steps required to reach 1).

Safely increment histogram[stopping_time].

Synchronization: Unless -nolock is passed, you must use C++ <mutex> locks to protect access to shared variables (COUNTER and histogram).

3. Required Output Streams
Standard Output (stdout / cout):
Prints the histogram frequencies line-by-line in CSV format from index 0 to 1000:

Code snippet
0, frequency_of_stopping_time(0)
1, frequency_of_stopping_time(1)
...
1000, frequency_of_stopping_time(1000)
Standard Error (stderr / cerr):
Prints execution metrics as a single CSV line containing N, T, and elapsed time in seconds and nanoseconds:

Code snippet
1000000,4,3.852953000
Experimental Requirements
Thread Benchmark:

Test execution across 1 to 8 threads using a large N (large enough to produce measurable runtimes).

Run each test scenario at least 10 times for both locked and -nolock modes, taking the average runtime.

Spreadsheet Deliverable:

Create an Excel spreadsheet containing:

A bar/column chart of the Collatz stopping time histogram.

A performance graph plotting thread count (1–8) vs. average runtime (with and without locks).

Submission Deliverables Checklist
mt-collatz.cpp: Source code cleanly formatted with proper comments.

Makefile: Must compile with flags -g -Wall with zero warnings/errors, and include a clean target.

README: Notes any challenges encountered during development.

Excel Spreadsheet (.xlsx): Contains data and charts for the histogram and timing experiment.

report.pdf: Report detailing system specs (CPU, cores, RAM), experiment methodologies, speedup analysis, and analysis of thread contention/lock overhead.
