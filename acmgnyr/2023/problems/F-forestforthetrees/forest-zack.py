# discrete forest for the trees
# pretty naive
# also very sorry for the code repetition 
# zbutler

import sys

def checkpos(px,py,found,trees,maxr):
    for t in trees:
        if t not in found and (abs(px-t[0])+abs(py-t[1])) <= maxr:
            #print('missing',t)
            return False
    return True

#MAXR = 10000
# #trees #sensed-trees
nt, ns, maxr = [int(x) for x in input().strip().split()]
if ns == 0:
    print("Ambiguous")
    sys.exit(0)
trees = set()
# tree-x tree-y
for _ in range(nt):
    trees.add(tuple([int(x) for x in input().strip().split()]))
# sensor-x sensor-y (sideways amt, forward amt)
sens = []
for _ in range(ns):
    sens.append(tuple([int(x) for x in input().strip().split()]))
    #if sens[-1] == (0,0):
    #    print("Impossible")
    #    sys.exit(0)
        
place = None
# pointing +x
for t in trees:
    px = t[0]-sens[0][1]
    py = t[1]+sens[0][0]
    #print(px,py)
    if (px,py) in trees:
        continue
    fail = False
    found = set()
    found.add(t)
    for s in range(1,ns):
        tx = px+sens[s][1]
        ty = py-sens[s][0]
        #print(tx,ty)
        if (tx,ty) in trees:
            found.add((tx,ty))
        else:
            #print('not',tx,ty)
            fail = True
            break
    if not fail and checkpos(px,py,found,trees,maxr):
        if place is None:
            place = (px,py)
        else:
            print("Ambiguous")
            sys.exit(0)

# pointing +y
for t in trees:
    px = t[0]-sens[0][0]
    py = t[1]-sens[0][1]
    if (px,py) in trees:
        continue
    #print(px,py)
    fail = False
    found = set()
    found.add(t)
    for s in range(1,ns):
        tx = px+sens[s][0]
        ty = py+sens[s][1]
        #print(tx,ty)
        if (tx,ty) in trees:
            found.add((tx,ty))
        else:
            #print('not',tx,ty)
            fail = True
            break
    if not fail and checkpos(px,py,found,trees,maxr):
        if place is None:
            place = (px,py)
        else:
            print("Ambiguous")
            sys.exit(0)

# pointing -x
for t in trees:
    px = t[0]+sens[0][1]
    py = t[1]-sens[0][0]
    if (px,py) in trees:
        continue
    #print(px,py)
    fail = False
    found = set()
    found.add(t)
    for s in range(1,ns):
        tx = px-sens[s][1]
        ty = py+sens[s][0]
        #print(tx,ty)
        if (tx,ty) in trees:
            found.add((tx,ty))
        else:
            #print('not',tx,ty)
            fail = True
            break
    if not fail and checkpos(px,py,found,trees,maxr):
        if place is None:
            place = (px,py)
        else:
            print("Ambiguous")
            sys.exit(0)

# pointing -y
for t in trees:
    px = t[0]+sens[0][0]
    py = t[1]+sens[0][1]
    if (px,py) in trees:
        continue
    #print(px,py)
    fail = False
    found = set()
    found.add(t)
    for s in range(1,ns):
        tx = px-sens[s][0]
        ty = py-sens[s][1]
        #print(tx,ty)
        if (tx,ty) in trees:
            found.add((tx,ty))
        else:
            #print('not',tx,ty)
            fail = True
            break
    if not fail and checkpos(px,py,found,trees,maxr):
        if place is None:
            place = (px,py)
        else:
            print("Ambiguous")
            sys.exit(0)

if not place:
    print("Impossible")
else:
    print(place[0],place[1])
