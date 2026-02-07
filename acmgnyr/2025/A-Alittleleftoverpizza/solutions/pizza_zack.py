import math

np = int(input().strip())
counts = {'S': 0, 'M': 0, 'L': 0}
for _ in range(np):
    sz, slices = input().strip().split()
    counts[sz] += int(slices)
tot = int(math.ceil(counts['S']/6) +math.ceil(counts['M']/8) +math.ceil(counts['L']/12))

print(tot)

