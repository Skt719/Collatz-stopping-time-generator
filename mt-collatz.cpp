#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

#include "collatz.hpp"

int main()
{
    mutex mtx;
    Collatz c;

    c.tFunction(10, ref(mtx)); // Example call to tFunction with N=10

    return 0;
}