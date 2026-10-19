#include <iostream>
#include <mutex>
#include <thread>
#include <charconv>
#include <string_view>
#include <cstdint>
#include <array>
#include <atomic>
#include <chrono>
#include <exception>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Converts a string argument to a positive integer
//STORES result as N
// Returns false if the text is not a valid positive integer
bool parsePositive(string_view text, uint64_t& value)
{
    const char* start = text.data(); //text.data() returns a pointer to the underlying character array of the string_view
    const char* end = start + text.size();

    // from_chars parses the number without creating a temporary string
    const auto [ptr, error] = from_chars(start, end, value);

    return error == errc{} && ptr == end && value > 0; //return bool if positive
}


// Calculate how many steps it takes for value to reach 1 using the Collatz conjecture rules
uint64_t stoppingTime(uint64_t value)
{
    uint64_t steps = 0;

    // Apply the Collatz rule until the value reaches 1.
    while (value != 1)
    {
        if (value % 2 == 0)
        {
            value /= 2; // If even, divide by 2
        } else
        {
            // If odd, multiply by 3 and add 1
            //check first to avoid overflowing the integer type
            if (value > (numeric_limits<uint64_t>::max() - 1) / 3)
            {
                throw overflow_error("Overflow occurred during Collatz computation.");
            }
            value = value * 3 + 1;
        }
        ++steps; // Increment the step count
    }
    return steps;
}


int main(int argc, char* argv[])
{
    // Require N and T, with and optional -nolock flag
    //N is the upper limit of the range of numbers to process
    //T is the number of threads to use
    //-nolock is an optional flag that disables locking for performance testing
    if (argc != 3 && argc != 4) {
        cerr << "Usage: ./mt-collatz <N> <T> [-nolock]\n";
        return 1;
    }

    uint64_t N = 0;
    uint64_t T = 0;

    //set N and T (check errors)
    if (!parsePositive(argv[1], N) || !parsePositive(argv[2], T))
    {
        cerr << "N and T must be positive integers.\n";
        return 1;
    }


    // The shared counter increments after assigning each number
    // Reject the maximum value to prevent that increment from wrapping around
    if (N == numeric_limits<uint64_t>::max())
    {
        cerr << "N is too large.\n";
        return 1;
    }

    // Enable unlocked mode only when the optimal argument is exactly -nolock
    const bool noLock = argc == 4 && string(argv[3]) == "-nolock";
    if (argc == 4 && !noLock)
    {
        cerr << "The only supported option is -nolock.\n";
        return 1;
    }

    // Counter that tells the threads which number to process next
    uint64_t counter = 1;

    // Each array index represents a stopping time; its value is the frequency of that stopping time. The array is initialized to zero.
    array<uint64_t, 1001> histogram{}; // Histogram to count stopping times, size 1001 for stopping times 0-1000

    // These mutexes protect the shared counter, histogram, and error flag from concurrent access by multiple threads
    mutex counterMutex; // Mutex to protect access to the counter
    mutex histogramMutex; // Mutex to protect access to the histogram
    mutex errorMutex; // Mutex to protect access to the error flag

    // Set to true if any thread encounters an error, allowing the main thread to terminate all threads smoothly
    atomic<bool> failed{false}; // Atomic flag to indicate if any thread encountered an error
    string errorMessage; // String to hold the error message if an error occurs

    // Each thread repeatedly claims a number, computes its stopping time,
    // and adds the result to the histogram
    auto threadFunction = [&]()
    {
        while (!failed.load())
        {
            uint64_t value = 0;

            // Claim the next number to process. In locked mode, only one thread
            // can access and increment counter at a time. In unlocked mode, multiple threads can access the counter concurrently.
            {
                unique_lock<mutex> lock(counterMutex, defer_lock);
                if (!noLock)
                {
                    lock.lock(); // Lock the mutex to safely access the counter
                }
                
                if (counter > N)
                {
                    return; // All numbers have been processed, exit the thread
                }

                value = counter++; // Claim the next number then increment the counter
                
            }

            try
            {
                // Calculate how many Collatz steps this number takes to reach 1
                uint64_t steps = stoppingTime(value);

                // The required histogram only has indexes 0-1000.
                if (steps >= histogram.size())
                {
                    throw out_of_range("Stopping time exceeds histogram range 0-1000.");
                }

                // record this numbers stopping time. The mutex prevents
                // multiple threads from writing to the same histogram index at the same time.
                
                unique_lock<mutex> lock(histogramMutex, defer_lock);
                if (!noLock)
                {
                    lock.lock(); // Lock the mutex to safely access the histogram
                }
                ++histogram[steps]; // Increment the histogram count for this stopping time
                
            } catch (const exception& error)
            {
                // Save the first error message while protecting it from
                // simultaneous writes by multiple threads.
                {
                    lock_guard<mutex> lock(errorMutex);
                    if (errorMessage.empty())
                    {
                        errorMessage = error.what(); // Store the error message
                    }
                }

                failed.store(true); // Set the failed flag to true to signal other threads to stop
                return;
            }
        }

    };

    // Start timing immediately before creating and running the threads
    const auto start = chrono::steady_clock::now();

    vector<thread> threads; // Vector to hold the thread objects
    try
    {
        // Reserve space for all thread objects before starting the workers
        threads.reserve(static_cast<size_t>(T));

        // Start the requested number of threads
        for (uint64_t i = 0; i < T; ++i)
        {
            threads.emplace_back(threadFunction); // Create and start a new thread
        }
    } catch (const exception& error)
    {
        // If thread creation fails, set the failed flag and store the error message
        failed.store(true);

        // Join every started thread so none are left running when main exits
        for (auto& t : threads)
        {
            if (t.joinable())
            {
                t.join(); // Wait for the thread to finish
            }
        }

        cerr << "Could not create worker threads: " << error.what() << '\n'; // Output the error message
        return 1;
    }

    // wait for all threads to finish before reading the final histogram
    for (auto& t : threads)
    {
        t.join(); // Wait for each thread to finish
    }

    // stop the timer after all worker threads have finished
    const auto end = chrono::steady_clock::now();

    //Report an error instead of printing incomplete results
    if (failed.load())
    {
        cerr << "Computation failed: " << errorMessage << '\n'; // Output the error message
        return 1;
    }

    // Print every histogram entry, including entries whose frequency is zero
    // This ouput goes to stdout so it can be redirected to a file if desired
    for (size_t i = 0; i < histogram.size(); ++i)
    {
        cout << i << ',' << histogram[i] << '\n'; // Output the stopping
    }

    // Convert the elapsed time to seconds and print the timing CSV row to stderr
    const chrono::duration<double> elapsed = end - start;
    cerr << N << ',' << T << ',' << fixed << setprecision(9) << elapsed.count() << '\n'; // Output the timing information


    return 0;
}




