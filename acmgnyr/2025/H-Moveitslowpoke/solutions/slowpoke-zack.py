import sys
from heapq import *

n,m,k,d,s,t = [int(x) for x in input().strip().split()]

e = [list() for _ in range(n+1)]
ed = {}
for _ in range(m):
    a,b,l = [int(x) for x in input().strip().split()]
    e[a].append((b,l))
    e[b].append((a,l))
    ed[(a,b)] = l
    ed[(b,a)] = l

r = {}
for _ in range(k):
    a,b,c = [int(x) for x in input().strip().split()]
    if (a,b) in r:
        r[(a,b)].append(c)
    else:
        r[(a,b)] = [c]

# states are (g, prev, curr, slowdist)
# g is first so python sort is easy
# slowdist is dist traveled *before* prev
q = []
heappush(q,(0, -1, s, 0))
# visitation check needs to include last edge and slowdist I think
vis = {}
vis[(-1,s,0)] = 0
while q:
    g, prev, curr, sd = heappop(q)
    if curr == t:
        print(g)
        sys.exit(0)
    for next, nd in e[curr]:
        if next == prev:
            continue # no u-turns
        news = None
        # are we in danger?
        if (prev,curr) in r:
            if (next in r[(prev,curr)] and sd + ed[(prev,curr)] + ed[(curr,next)] <= d):
                # OK, but now we have more sd to worry about
                news = (g + ed[(curr,next)], curr, next, sd + ed[(prev,curr)])
            elif next not in r[(prev,curr)]:
                # new edge is fine and resets our slowdist
                # even if last edge was part of a triple, that was checked previously (above)
                news = (g + ed[(curr,next)], curr, next, 0)
        else:
            # last edge not part of a triple, all is well
            news = (g + ed[(curr, next)], curr, next, 0)
        if news and (news[1:] not in vis or vis[news[1:]] > news[0]):
            heappush(q,news)
            vis[news[1:]] = news[0]
print('impossible')
