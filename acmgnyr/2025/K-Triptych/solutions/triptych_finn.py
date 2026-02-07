#!/usr/bin/python3

"""
Use dynamic programming. First count all spotlight sequences ignoring the palindrome rule.
Then count the number of palindromes separately that satisfy all rules.
The dynamic programming state consists of:
    position <= 52
    a count <= 24
    b count <= 24
    c count <= 24
    last character: 3
    previous character: 3
Number of states is approximately 6.5 million.


Author: Finn Lidbetter
"""


from collections import defaultdict
from sys import stdin

tokens = stdin.readline().split(' ')
w = int(tokens[0])
d = int(tokens[1])

count_max = (w+2*d)//3


dp = defaultdict(int)
dp[(0, 0, 0, 0, -1, -1)] = 1
for i in range(1,w+1):
    for a in range(0, count_max+1):
        for b in range(0, count_max+1):
            for c in range(0, count_max+1):
                prevs = [0,1,2]
                if i == 1:
                    prevs.append(-1)
                for prev in prevs:
                    prev_prevs = [0,1,2]
                    if i <= 2:
                        prev_prevs.append(-1)
                    for prev_prev in prev_prevs:
                        # print(f"Considering state {(i-1,a,b,c,prev,prev_prev)}")
                        if prev == prev_prev and abs(prev) != 1:
                            # print(f"\tskipping because prev==prev_prev and abs(prev)!=1")
                            continue
                        for choice in (0,1,2):
                            # print(f"\tConsidering choice {choice}")
                            if prev == prev_prev and prev == 1 and choice == 1:
                                # print("\t\tskipping because prev==prev_prev and prev==1 and choice==1")
                                continue
                            if prev == choice and choice != 1:
                                # print("\t\tskipping because prev==choice and choice!=1")
                                continue
                            counts = [a,b,c]
                            counts[choice] += 1
                            if counts[choice] > count_max:
                                # print(f"\t\tskipping because {counts[choice]}=counts[choice] > count_max = {count_max}")
                                continue
                            state = (i-1, a, b, c, prev_prev, prev)
                            if state in dp:
                                next_state = (i, *counts, prev, choice)
                                dp[next_state] += dp[state]
                            # else:
                                # print(f"\t\tprev state {state} is not in dp")

# print(dp)
total_count = 0
for a in range(0, count_max+1):
    for b in range(0, count_max+1):
        if abs(a-b) > d:
            continue
        for c in range(0, count_max+1):
            if abs(a-c) > d or abs(b-c) > d:
                continue
            for prev in (-1,0,1,2):
                for prev_prev in (-1,0,1,2):
                    state = (w,a,b,c,prev,prev_prev)
                    if state in dp:
                        total_count += dp[state]
palindrome_count = 0
middle = w//2
for a in range(0, count_max+1):
    for b in range(0, count_max+1):
        for c in range(0, count_max+1):
            for prev in (0,1,2):
                if w%2 == 0:
                    if prev != 1:
                        continue
                    if 2*abs(a-b) > d or 2*abs(a-c) > d or 2*abs(b-c) > d:
                        continue
                    for prev_prev in (-1,0,1,2):
                        if prev == prev_prev:
                            continue
                        state = (middle, a, b, c, prev_prev, prev)
                        palindrome_count += dp[state]
                else:
                    for next_ch in (0,1,2):
                        if next_ch == prev:
                            continue
                        palindrome_counts = [2*a,2*b,2*c]
                        palindrome_counts[next_ch] += 1
                        pa,pb,pc = palindrome_counts
                        if abs(pa-pb) > d or abs(pa-pc) > d or abs(pb-pc) > d:
                            continue
                        for prev_prev in (-1,0,1,2):
                            state = (middle,a,b,c,prev_prev,prev)
                            palindrome_count += dp[state]
# print(palindrome_count)
print(total_count - palindrome_count)

