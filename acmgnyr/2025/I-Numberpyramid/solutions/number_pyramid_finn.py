#!/bin/python3

from sys import stdin

def main():
    n = int(stdin.readline().strip())
    pyramid = []
    remaining = 0
    for _ in range(n):
        row = list(map(int, stdin.readline().strip().split(' ')))
        for i in row:
            if i == 100:
                remaining += 1
        pyramid.append(row)
    prev_remaining = -1
    while remaining != prev_remaining:
        prev_remaining = remaining
        for i in range(n):
            for j in range(i+1):
                if i > 0 and j > 0:
                    above_left = pyramid[i-1][j-1]
                    left = pyramid[i][j-1]
                    if above_left != 100 and left != 100:
                        if pyramid[i][j] == 100:
                            pyramid[i][j] = above_left - left
                            if abs(pyramid[i][j]) >= 100:
                                print("no solution")
                                return
                            remaining -= 1
                        elif pyramid[i][j] != above_left - left:
                            print("no solution")
                            return
                if i > 0 and j < i:
                    above_right = pyramid[i-1][j]
                    right = pyramid[i][j+1]
                    if above_right != 100 and right != 100:
                        if pyramid[i][j] == 100:
                            pyramid[i][j] = above_right - right
                            if abs(pyramid[i][j]) >= 100:
                                print("no solution")
                                return
                            remaining -= 1
                        elif pyramid[i][j] != above_right - right:
                            print("no solution")
                            return
                if i < n - 1:
                    below_left = pyramid[i+1][j]
                    below_right = pyramid[i+1][j+1]
                    if below_left != 100 and below_right != 100:
                        if pyramid[i][j] == 100:
                            pyramid[i][j] = below_left + below_right
                            remaining -= 1
                            if abs(pyramid[i][j]) >= 100:
                                print("no solution")
                                return
                        elif pyramid[i][j] != below_left + below_right:
                            print("no solution")
                            return
    if remaining > 0:
        print("ambiguous")
    else:
        print("solvable")
        for row in pyramid:
            print(' '.join(map(str, row)))

if __name__ == "__main__":
    main()
