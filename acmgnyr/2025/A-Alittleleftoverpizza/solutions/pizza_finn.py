from sys import stdin
from collections import defaultdict

n = int(stdin.readline())
counts = defaultdict(int)
for _ in range(n):
    tokens = stdin.readline().strip().split(" ")
    counts[tokens[0]] += int(tokens[1])
boxes = (counts['S'] // 6) + (1 if counts['S'] % 6 != 0 else 0)
boxes += (counts['M'] // 8) + (1 if counts['M'] % 8 != 0 else 0)
boxes += (counts['L'] // 12) + (1 if counts['L'] % 12 != 0 else 0)
print(boxes)
