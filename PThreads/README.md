<!-- PS1='${PWD##*/} $ ' -->

# Assignment 1 - PThreads

## Authors
- `sdi2000154`: Giorgos Nikolaou
- `sdi2100203`: Helen Fili

## Exercise 1.1

### Description of the problem
In this problem, the task is to use the **Monte Carlo** method to approximate `pi` by sampling random points in a `2x2` square (centered at `(0, 0)`) and observing how many of them fall within the unit circle. We approach the problem with both serial and parallel implementations (using `PThreads`). As expected, the method produces better approximations as we increase the number of iterations, but that is not efficient when using the serial algorithm. To accelerate the computation, we utilize PThreads to parallelize the random sampling among them.

### Brief description of the solution
- **Serial**: 
The serial version of the algorithm is executed when the command line argument `<threads>` is set to `1`.
- **Parallel**:
The parallel version of the algorithm is executed when the command line argument `<threads>` is greater than `1`. In this version, the samples of random points from arrow throwing are divided equally among the threads.

`approx_percentage(LONG n)`: In this function, we calculate the number of arrows that landed in the critical area (inside the unit circle). For the point sampling, a thread-safe version of `rand()` is used (`rand_r()`).

`worker(void* args)`: This function utilizes `approx_percentage()` to compute the number of arrows -that landed in the critical area- for the corresponding thread and then adds the result in the global variable `arrows`, which is guarded by a mutex.

`threaded_pi(size_t threads, LONG n)`: This functions is responsible for dividing the total number of arrows among the threads, creating and joining the threads, initializing and destroying the mutex used for sychronization and returning the approximation of `pi`. 

**Implmentation Details**: type `long long` is macro-defined as `LONG`.

**Important**: The number of sampling points `<n>`, given from command line, must be divisible by the number of `<threads>`.

### Presentation of experiments and analysis


## Exercise 1.2

### Description of the problem
The goal of this exercise is to implement a parallel **matrix multiplication** program using `PThreads`. An observation made after implementing the initial version of the exercise (`matmul_false_sharing.c`) was the presence of *false sharing*. **False sharing** occurs when multiple threads, running on separate cores, access different variables that happen to reside on the same cache line. To address that issue, we implemented three alternative programs (`matmul_2d.c`, `matmul_pad_var.c`, `matmul_private_var.c`).