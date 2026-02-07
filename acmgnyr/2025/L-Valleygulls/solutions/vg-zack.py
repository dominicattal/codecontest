import math
import sys

DEBUG = False
EPS = 0.000001
pts = []
p = 0
r = 0
m = 0
nests = dict()
allt = 0

class Basin():
    min : int # index
    lc : int # left catchment
    rc : int # right
    full : bool
    wh : float # water height
    vol : float # water vol

    def __init__(self,min,lv,rv):
        self.min = min
        self.lv = lv
        self.rv = rv
        self.lc = lv
        self.rc = rv
        self.full = False
        self.wh = pts[self.min][1]
        self.vol = 0
        self.lw = min
        self.rw = min
        self.lx = pts[self.min][0]
        self.rx = self.lx

    def __repr__(self):
        if self.full:
            return f"{self.lv}->{self.min}->{self.rv} (full, catch {self.lc}->{self.rc})"
        else:
            return f"{self.lv}->{self.min}->{self.rv} catch {self.lc}->{self.rc} water {self.lx:.3f}->{self.rx:.3f}@{self.wh:.3f} vol {self.vol:.3f}"

    def ttn(self,r):
        # how long until next vertex hit?
        if self.full:
            return 0
        nly = pts[self.lw-1][1] if self.lw > 0 else 1001
        nry = pts[self.rw+1][1] if self.rw < p-1 else 1001
        if nly < nry:
            nh = nly
            nlx = pts[self.lw-1][0]
            nrx = getx(nh,pts[self.rw],pts[self.rw+1]) if self.rw < p-1 else pts[self.rw][0]
        else:
            nh = nry
            nlx = getx(nh,pts[self.lw-1],pts[self.lw]) if self.lw > 0 else pts[0][0]
            nrx = pts[self.rw+1][0]
        a = (nh-self.wh) * ((nrx-nlx)+(self.rx-self.lx))/2
        if DEBUG: print('next area is ' + str(a))
        t = a / ((pts[self.rc][0]-pts[self.lc][0])*r)
        return t

    def add_water(self, t):
        global nests,m
        if DEBUG: print('adding to ' + str(self) + " for time " + str(t))
        # assume that this is only called if at most the bottom section fills
        vol = t*r*(pts[self.rc][0]-pts[self.lc][0])
        if DEBUG: print('adding vol ' + str(vol))
        self.vol += vol
        # is this really right? humph
        mlr = 0 if self.lw == 0 else (pts[self.lw][0]-pts[self.lw-1][0])/(pts[self.lw][1]-pts[self.lw-1][1])
        mrr = 0 if self.rw == p-1 else (pts[self.rw+1][0]-pts[self.rw][0])/(pts[self.rw+1][1]-pts[self.rw][1])
        b = 2*(self.rx-self.lx)
        radic = b*b+8*(mrr-mlr)*vol
        dh = (-1*b + math.sqrt(radic))/(2*(mrr-mlr))
        self.wh += dh
        self.lx += dh * mlr
        self.rx += dh * mrr
        spill = 0
        if self.lw > 0 and abs(self.lx-pts[self.lw-1][0]) < EPS:
            # move lw to previous vertex
            if DEBUG: print("hitting left vertex " + str(self.lw-1))
            self.lw -= 1
            self.lx = pts[self.lw][0]
            if self.lx in nests:
                nests[self.lx] = allt
                if DEBUG: print('left side nest at ' + str(self.lx) + ' at t=' + str(allt))
                m -= 1
            if self.lw > 0 and self.lw == self.lv:
                self.full = True
                if DEBUG: print("full")
                spill = -1
        elif self.rw < p-1 and abs(self.rx-pts[self.rw+1][0]) < EPS:
            if DEBUG: print("hitting right vertex " + str(self.rw+1))
            self.rw += 1
            self.rx = pts[self.rw][0]
            if self.rx in nests:
                nests[self.rx] = allt
                if DEBUG: print('right side nest at ' + str(self.rx) + ' at t=' + str(allt))
                m -= 1
            if self.rw < p-1 and self.rw == self.rv:
                self.full = True
                if DEBUG: print('full')
                spill = 1
        if DEBUG: print("new height " + str(self.wh) + " new lx " + str(self.lx) + " new rx " + str(self.rx))
        return spill

def combine_basins(bl,br):
    newb = Basin(bl.min, bl.lv, br.rv)
    newb.full = False
    newb.wh = bl.wh # should be equal anyway
    newb.vol = bl.vol + br.vol
    newb.lc = bl.lc
    newb.rc = br.rc
    newb.lw = bl.lw
    newb.rw = br.rw
    newb.lx = bl.lx
    newb.rx = br.rx
    return newb

def make_basins(mins,pts):
    bas = []
    for mi in mins:
        li = mi
        while li > 0 and pts[li-1][1] > pts[li][1]:
            li -= 1
        ri = mi
        while ri < p-1 and pts[ri+1][1] > pts[ri][1]:
            ri += 1
        bas.append(Basin(mi,li,ri))
    return bas

def getmins(pts):
    mins = [] # indices
    if pts[0][1] < pts[1][1]:
        mins.append(0)
    for i in range(1,p-1):
        if pts[i][1] < pts[i-1][1] and pts[i][1] < pts[i+1][1]:
            mins.append(i)
    if pts[-1][1] < pts[-2][1]:
        mins.append(p-1)
    return mins

def getx(y,lp,rp):
    return lp[0] + (rp[0]-lp[0])*(y - lp[1])/(rp[1]-lp[1])

def gety(x,pts):
    for pi in range(p-1):
        if pts[pi][0] == x:
            return pts[pi][1]
        elif pts[pi][0] < x and pts[pi+1][0] > x:
            lx, ly = pts[pi]
            rx, ry = pts[pi+1]
            return ly + (ry-ly)*(x-lx)/(rx-lx)

def run(bas):
    global allt
    iters = 0
    while m > 0:
        iters += 1
        if DEBUG: print('----------------')
        if DEBUG: print(allt,':',bas)
        if DEBUG: print('----------------')
        nft = 100000001
        nb = -1
        for bi in range(len(bas)):
            if not bas[bi].full:
                ttn = bas[bi].ttn(r)
                if ttn < nft:
                    nft = ttn
                    nb = bi
        allt += nft
        if DEBUG: print('basin to fill is ' + str(bas[nb]) + ' t ' + str(nft))
        #if nft == 0:
        #    sys.exit()
        bi = 0
        filled = []
        while bi < len(bas):
            if not bas[bi].full:
                spill = bas[bi].add_water(nft)
                if spill != 0:
                    filled.append((bi,spill))
            bi += 1
        for bisp in filled:
            bi, spill = bisp
            if spill == -1:
                # I am fill, will spill left
                if not bas[bi-1].full:
                    bas[bi-1].rc = bas[bi].rc
                else:
                    # full, is it spilling over itself or are we merging?
                    if abs(bas[bi-1].wh - bas[bi].wh) < EPS:
                        newb = combine_basins(bas[bi-1],bas[bi])
                        bas = bas[:bi-1]+[newb]+bas[bi+1:]
                    else:
                        si = bi-1
                        while bas[si].full:
                            bas[si].rc = bas[bi].rc
                            si -= 1
                        bas[si].rc = bas[bi].rc
            elif spill == 1:
                # spill right
                if not bas[bi+1].full:
                    bas[bi+1].lc = bas[bi].lc
                else:
                    # full, is it spilling over itself or are we merging?
                    if abs(bas[bi+1].wh - bas[bi].wh) < EPS:
                        newb = combine_basins(bas[bi],bas[bi+1])
                        bas = bas[:bi]+[newb]+bas[bi+2:]
                    else:
                        si = bi+1
                        while bas[si].full:
                            bas[si].lc = bas[bi].lc
                            si += 1
                        bas[si].lc = bas[bi].lc

def main():
    global p,r,m
    prm = input().strip().split()
    p = int(prm[0])
    r = float(prm[1])
    m = int(prm[2])
    global pts
    allpts = [int(x) for x in input().strip().split()]
    for i in range(p):
        pts.append((allpts[2*i],allpts[2*i+1]))

    global nests
    nlist = [int(x) for x in input().strip().split()]
    nests = {x : None for x in nlist}
    ni = 0
    pi = 0
    while ni < len(nlist):
        if nlist[ni] > pts[pi][0] and nlist[ni] < pts[pi+1][0]:
            pts.insert(pi+1,(nlist[ni],gety(nlist[ni],pts)))
            pi += 1
            ni += 1
            p += 1
        else:
            pi += 1
    if DEBUG: print(pts)

    mins = getmins(pts)
    if DEBUG: print(mins)
    bas = make_basins(mins,pts)

    run(bas)

    for n in nests:
        print(nests[n])

main()
