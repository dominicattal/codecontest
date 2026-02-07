import sys

n = int(sys.stdin.readline())

def display(rows):
  for r in rows:
    for c in r:
      print(c,end=' ')
    print()

def consistent(v1,v2,v3):
  ans = list(filter(lambda x: x!=None,[v1,v2,v3]))
  if ans == []:
    return None
  else:
    for i in ans[1:]:
      if i != ans[0]: return False
    if abs(ans[0]) >= 100: return False
    return ans[0]

def val1(rows,r,c,n):
  ans = None
  if r > 0 and c > 0 and abs(rows[r-1][c-1]) < 100 and abs(rows[r][c-1]) < 100:
    ans= rows[r-1][c-1] - rows[r][c-1]
  return ans

def val2(rows,r,c,n):
  ans = None
  if r > 0 and c < r and abs(rows[r-1][c]) < 100 and abs(rows[r][c+1]) < 100:
    ans = rows[r-1][c] - rows[r][c+1]
  return ans

def val3(rows,r,c,n):
  ans = None
  if r < n-1 and abs(rows[r+1][c]) < 100 and abs(rows[r+1][c+1]) < 100:
    ans = rows[r+1][c] + rows[r+1][c+1]
  return ans

rows = []
unfilled = {}
nempty = 0
for r in range(n):
  line = list(map(int,sys.stdin.readline().split()))
  rows.append(line)
  nempty += line.count(100)
  for i in range(len(line)):
    if line[i] == 100:
      unfilled[(r,i)]=None
change = True
while change:
  change = False
  deletions = []
  ks = unfilled.keys()
  for k in ks:
    (r,c) = k
    # above left:
    v1 = val1(rows,r,c,n)
    if v1 != None:
      change = True
    v2 = val2(rows,r,c,n)
    if v2 != None:
      change = True
    v3 = val3(rows,r,c,n)
    if v3 != None:
      change = True
    if v1 or v2 or v3 or v1==0 or v2==0 or v3==0:
      x = consistent(v1,v2,v3)
      if  x==0 or x:
         rows[r][c] = x
         deletions.append((r,c))
         nempty -= 1
      else:
        print("no solution")
        sys.exit(0)
    #else:
    #  deletions.append((r,c))
    #  nempty -= 1
  for item in deletions:
    del unfilled[item]
  
if nempty > 0:
  print("ambiguous")
  sys.exit(0)
# Check:
for r in range(n-1):
  rw = rows[r]
  for c in range(r+1): 
    if rows[r][c] != rows[r+1][c]+rows[r+1][c+1]:
      print("no solution")
      sys.exit(0)
print("solvable")
display(rows)
