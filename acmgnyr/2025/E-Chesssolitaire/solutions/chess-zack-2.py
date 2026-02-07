from collections import deque
from copy import deepcopy #ugh
import sys

n,m = [int(x) for x in input().strip().split()]
cb = [['' for _ in range(n)] for _ in range(n)]
for _ in range(m):
    pc = input().strip().split()
    loc = (ord(pc[1][0])-ord('A'),int(pc[1][1])-1)
    cb[loc[0]][loc[1]] = pc[0]
sz = n
vis = set()

def succ(cb):
    s = []
    moves = []
    for r in range(sz):
        for c in range(sz):
            if cb[r][c] == 'N':
                for dr,dc in ((-2,-1),(-2,1),(-1,-2),(-1,2),(1,-2),(1,2),(2,-1),(2,1)):
                    moves.append((r,c,r+dr,c+dc,'N'))
            if cb[r][c] == 'R' or cb[r][c] == 'Q':
                # row and column
                for dr,dc in ((-1,0),(1,0),(0,-1),(0,1)):
                    newr = r
                    newc = c
                    while True:
                        newr += dr
                        newc += dc
                        if newr >= sz or newr < 0 or newc >= sz or newc < -0:
                            break
                        moves.append((r,c,newr,newc,cb[r][c]))
                        if cb[newr][newc] != '':
                            break
            if cb[r][c] == 'B' or cb[r][c] == 'Q':
                # diagonals
                for dr,dc in ((-1,-1),(-1,1),(1,-1),(1,1)):
                    newr = r
                    newc = c
                    while True:
                        newr += dr
                        newc += dc
                        if newr >= sz or newr < 0 or newc >= sz or newc < -0:
                            break
                        moves.append((r,c,newr,newc,cb[r][c]))
                        if cb[newr][newc] != '': break
            if cb[r][c] == 'K':
                for dr,dc in ((-1,-1),(-1,0),(-1,1),(0,-1),(0,1),(1,-1),(1,0),(1,1)):
                    moves.append((r,c,r+dr,c+dc,'K'))
            # are we getting rid of pawns? doesn't matter i guess
            if cb[r][c] == 'P':
                moves.append((r,c,r+1,c-1,'P'))
                moves.append((r,c,r+1,c+1,'P'))
    # make sure loc1, then loc2 is in lexicographical order
    # cleverly organized tuple FTW
    moves.sort()
    return moves

def print_moves(mv):
    for r,c,newr,newc,pc in mv:
        print(pc + ": " + chr(ord('A')+r) + str(c+1) + " -> " + chr(ord('A')+newr) + str(newc+1))

def dfs(cb,allmv):
    global vis
    #print('\n'.join([' . '.join(x) for x in cb]))
    #print(allmv)
    if len(allmv) == m-1:
        # we did it!
        print_moves(allmv)
        sys.exit()
    moves = succ(cb)
    for mv in moves:
        r, c, newr, newc, pc = mv
        if newr >= 0 and newc >= 0 and newr < sz and newc < sz:
            if cb[newr][newc] != '':
                op = cb[newr][newc]
                cb[r][c] = ''
                cb[newr][newc] = pc
                if str(cb) not in vis:
                    #print(allmv)
                    allmv.append(mv)
                    vis.add(str(cb))
                    #print(allmv)
                    dfs(cb,allmv)
                    # if here, this has failed, unwind
                    allmv.pop()
                cb[newr][newc] = op
                cb[r][c] = pc
                #print(allmv)
                #print(cb)

dfs(cb,[])

print('No solution')
