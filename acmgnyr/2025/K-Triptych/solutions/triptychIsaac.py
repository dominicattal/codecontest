#!/usr/bin/python3

# Isaac Lee
# 2025 East Division
# Triptych
# Accepted submission

# This solution uses dynamic programming to count sequences satisfying the first rule. Specifically,
# it keeps track of the number of sequences satisfying the first rule which use a certain number of
# As, a certain number of Bs and certain number of Cs, and which end in A, B, BB or C. Working from
# shorter to longer sequences, we can determine when appending an A, B or C will result in a longer
# sequence still satisfying the first rule and which ending it will have, as well as its letter counts.
# The number of sequences satisfying the first 2 rules will then be the sum of entries in the dp table
# over all (a,b,c) letter counts such that a + b + c = W and max(a,b,c) - min(a,b,c) <= D, over all
# endings. Finally, we need to subtract the number of palindromes satisfying the first 2 rules, which
# is done by considering dp entries corresponding to sequences of length W/2. It turns out that there
# are 26235 triplets of nonnegative integers (a,b,c) such that a + b + c <= 52, and since for each
# such triplet, there are 4 possible endings to associate with it, there will be a bit over 10^5
# states in the worst case.

W,D = map(int,input().split())
Wpo = W + 1
fourWpo = Wpo << 2
fourWposq = fourWpo * Wpo
halfW = Wpo >> 1
ways = [0] * (fourWposq * Wpo)
ways[fourWposq] = 1
ways[fourWpo + 1] = 1
ways[7] = 1
result = 0
for length in range(1,Wpo):
	for a in range(length + 1):
		for b in range(length - a + 1):
			c = length - a - b
			index = ((a * Wpo + b) * Wpo + c) << 2
			for jindex in range(4):
				way = ways[index + jindex]
				if way:
					ending = ["A","B","BB","C"][jindex]
					if length == halfW:
						if W & 1:
							if ending != "BB":
								newa = a << 1
								newb = b << 1
								newc = c << 1
								if ending == "A":
									newa -= 1
								if ending == "B":
									newb -= 1
								if ending == "C":
									newc -= 1
								if max(newa,newb,newc) - min(newa,newb,newc) <= D:
									result -= way
						elif ending == "B" and (max(a,b,c) - min(a,b,c)) << 1 <= D:
							result -= way
					if length == W:
						if max(a,b,c) - min(a,b,c) <= D:
							result += way
					else:
						for kindex in range(3):
							newcar = "ABC"[kindex]
							if ending[-1] != newcar or ending == "B":
								lindex = index + [fourWposq,fourWpo,4][kindex]
								if newcar == "B":
									if ending == "B":
										lindex += 2
									else:
										lindex += 1
								if newcar == "C":
									lindex += 3
								ways[lindex] += way
print(result)

