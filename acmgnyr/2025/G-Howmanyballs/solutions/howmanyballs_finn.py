#!/bin/python3

"""
Try every value for r from 1 to 1,000,000 (the maximum possible value for r).
For each r, we can rearrange the equation 2rg/((r+g)(r+g-1)) = p/q to get a quadratic in g:
0 = pg^2 + (2pr - 2rq - p)g + (pr^2 - pr).
We can use the quadratic formula to find possible values for g.
Calculate the two possible values for g, rounded to the nearest integer.
Then check if they satisfy the original equation.

Let N = rMax.
Time complexity: O(N)
author: Finn Lidbetter
"""

from sys import stdin


def main():
    p, q = map(int, stdin.readline().strip().split(' '))
    for r in range(1, 1000001):
        a = p
        b = 2*p*r - 2*r*q - p
        c = p*r*r - p*r
        disc = b*b - 4*a*c
        if disc < 0:
            continue
        sqrt_disc = disc ** 0.5
        g1 = round((-b + sqrt_disc) / (2*a))
        g2 = round((-b - sqrt_disc) / (2*a))
        if g1 > 0 and 2*r*g1 * q == p*(g1 + r)*(g1 + r - 1):
            print(f"{r} {g1}")
            return
        if g2 > 0 and 2*r*g2 * q == p*(g2 + r)*(g2 + r - 1):
            print(f"{r} {g2}")
            return
    print("impossible")

if __name__ == "__main__":
    main()
