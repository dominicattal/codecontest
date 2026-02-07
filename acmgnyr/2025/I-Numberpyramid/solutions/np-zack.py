import sys
h = int(input())
p = []
for r in range(h):
    p.append([int(x) for x in input().strip().split()])
going = True
while going:
    hasun = False
    going = False
    for r in range(1,h):
        for c in range(r):
            # triple is p[r-1][c], p[r][c], p[r][c+1]
            if p[r-1][c] != 100 and p[r][c] != 100 and p[r][c+1] != 100:
                if p[r-1][c] != p[r][c] + p[r][c+1]:
                    print('contradiction')
                    sys.exit()
            else:
                if not hasun:
                    hasun = True
                if p[r][c] != 100 and p[r][c+1] != 100:
                    p[r-1][c] = p[r][c] + p[r][c+1]
                    going = True
                elif p[r-1][c] != 100 and p[r][c+1] != 100:
                    p[r][c] = p[r-1][c] - p[r][c+1]
                    going = True
                elif p[r-1][c] != 100 and p[r][c] != 100:
                    p[r][c+1] = p[r-1][c] - p[r][c]
                    going = True
    
if hasun:
    print('ambiguous')
else:
    print('solvable')
    for r in range(h):
        print(' '.join([str(x) for x in p[r]]))
        
    
    
    
