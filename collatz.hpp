#ifndef COLLATZ_HPP
#define COLLATZ_HPP

#include <iostream>
#include <thread>

class Collatz 
{
    public:
        Collatz() = default;
        ~Collatz() = default;

        void tFunction(int *N);
        //timer function
        //collatz math

    private:
        //int stopping_time_helper(int n, int count);
};



#endif // COLLATZ_HPP