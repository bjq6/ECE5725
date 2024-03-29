//
//  jfs9, 10/16/16   Performance test on C array
//  
// Modified:
// Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
// Mar 20, 2017
//
// ================================================

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// compare subroutine checks for arguments f > s
int comp (const void * elem1, const void * elem2)
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

int main()
{

// Generate data
     const unsigned arraySize = 32768;
    int data[arraySize];

    // fill the array with random integers of the form 0 <= value <= 255
    for (unsigned c = 0; c < arraySize; ++c) {
        /* data[c] = std::rand() % 256; */
        data[c] = rand() % 256;
    }
    // sort the array in numerical order
    qsort(data, sizeof(data)/sizeof(*data), sizeof(*data), comp);
    printf ("size of data = %d, size of *data = %d\n", sizeof(data), sizeof(*data) );
    //
    clock_t start = clock();  // start a timer
    long long sum = 0;

    for (unsigned i = 0; i < 20000; ++i)  //  For a bunch of iterations
    {
        for (unsigned c = 0; c < arraySize; ++c) // for the entire integer array
        {
            if (data[c] >= 128)   //  add to sum only if element > 128
                sum += data[c];
        }
    }
    double elapsedTime = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf ("elapsed time = %3.5f\n", elapsedTime);
}
