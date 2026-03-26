# Coding Vibes Microthon!

## Overview

The goal is for users to write a program that can solve a given problem. In this case, it is to calculate the nth fibonacci number modulo a big number. It is mainly for testing the time complexity of the algorithm used and the efficiency of their implementation. This naturally means that some languages will perform better than others (i.e. python vs c).

This program has a frontend and a backend. The frontend is written in php and is how users submit their code to the backend. The backend is written in C and runs users' submitted code. Right now, the code is given user level access, which means the users' programs can read and write files that belong to the user.

## Algorithms

The fibonacci sequences is defined by the relation `F(n) = F(n-1) + F(n-2)`, where `F(n)` is the nth fibonacci number, `F(0) = 0`, and `F(1) = 1`

There are many different algorithms to calculate fibonacci numbers. I'll give my thoughts on some of them.

### Binet's formula

This is a closed form solution for the nth fibonacci number. However, this method actually isn't very good for very large n since it requires exponentiating floating-point values.

### Recursion

This is one of the first ways people learn how to compute fibonacci numbers. It is also a textbook example of recursion, and I believe many people are familiar with this. The naive implementation of recursion looks like this:

```
def fib(n):
    if n == 0: return 0
    if n == 1: return 1
    return fib(n-1) + fib(n-2)
```

This solution isn't very good because you're doing many computations several time. For example, if `n = 10`, then the first computation would be `fib(10) = fib(9) + fib(8)`, and the second one would be `fib(9) = fib(8) + fib(7)`. The problem here is that `fib(8)` is calculated more than once; once for `fib(10)` and onces for `fib(9)`. A simple way to improve this algorithm is by using a hashmap to cache values, which looks like this:

```
F = {0: 0, 1: 1}
def fib(n):
    if n in F: return F[n]
    F[n] = fib(n-1) + fib(n-2)
    return F[n]
```

This ensures an O(n) time complexity and O(n) space complexity.

A problem people might run into for larger n is recursion limits, which is solved with the next method.

### Iteration

This is an improved way of calculating fibonacci numbers. The naive implementation looks like this:

```
def fib(n):
    fib = [0] * (n+1)
    fib[0] = 0
    fib[1] = 1
    for i in range(2, n+1):
        fib[i] = fib[i-1] + fib[i-2]
    return fib[n]
```

This solution is better than recursion because there is less memory associated with function calls. This method can be further improved with the insight that you only need the previous two fibonacci numbers:

```
def fib(n):
    f1, f2 = 0, 1
    for i in range(2, n+1):
        f1, f2 = f2, f1 + f2
    return f2
```

This improves the algorithm from O(n) to O(1) space complexity, and maintains an O(n) time complexity.

### Matrix Multiplications

The relationship for fibonacci can be expressed as the product of two square matrices like so:

```
    | F(n+1)  F(n)   | * | 1  1 | = | F(n+1) + F(n)  F(n+1) |    
    | F(n)    F(n-1) |   | 1  0 |   | F(n+1)         F(n)   |
```

This allows the `nth` fibonacci to be represented as the power of a matrix:
```
    | 1 1 | ^ n = | F(n+2)  F(n+1) |
    | 1 0 |       | F(n+1)    F(n) |
```

### Binary exponentiation

You can compute large powers much quicker using binary exponentiation. To get an idea of how this works, take the example of `x^100`

`100 base 10  = 1100100 base 2`

It follows that

`x^4 + x^32 + x^64 = x^100`

The insight here is that you can calculate powers of two from previous powers of two, or more concretely

`x^64 = x^32 * x^32`

That means you need to perform at most `log(n)` multiplications to calculate `x^n`

The same principle can be applied to matrix multiplcation to compute large fibonacci numbers. Here's the pseudocode:

```
def fib(n):
    ans = [[1, 1], 
           [1, 0]]
    cur = [[1, 1], 
           [1, 0]]
    while n > 0:
        if n % 2 == 1:
            ans = matmul(ans, cur)
        cur = matmul(cur, cur)
        n = n / 2
    return ans[1][1]
```

This solution uses O(1) space and O(log(n)) time. Programs can use this algorithm (with some modifications) to easily calculate fibonacci numbers past `10^100000`, which my C program `acm/fib.c` does.

### Implementation

`acm/fib.py` and `acm/fib.c` contain working code examples.

## Setup

- Start the frontend using php or whatever webserver you want.
    - The command I use is `php 0.0.0.0:8000 -t src-web`
- Use the Makefile to build the backend.
- Edit teams in `acm/server.json`
    - I was trying to do it ICPC-style where teams choose their names and are assigned passwords
- Run `bin/server acm/server.json`
- Have users navigate to the ip. If that doesnt work, this works for me: http://{hostname}.resource.campus.njit.edu:8000

!!! The project relies on relative directories, so make sure everything stays consistent in this regard

Users's program should print the correct answer to standard out.

## Standings

After submitting code for a problem, that code is validated for the right answer. If the code exceeds some time or memory threshold, then the run will fail. If the code prints the wrong number to standard out, the run will fail. If the run finishes within the time limit and prints the correct answer, then it is stored as a successful run.

When producing the standings, the program grabs all of the successful runs for each problem and determines the fastest time for that problem. If a user doesn't have a successful run for a problem, then the default `10000 ms` is used (the time limit for running programs). The running time for each problem is used to calculate the total time for a team, which is just the sum of the running times for each problem. The teams are sorted by this total time.
