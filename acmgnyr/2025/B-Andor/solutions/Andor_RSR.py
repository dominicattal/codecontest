# Leaves always have a minchange value of 1.
# For each non-leaf node, determine its boolean value after DFS has visited
# all of its children, then determine its minchange value as follows. 
#    "AND" node with value True: chooose the minimum minchange value of its
#         children (all of which must have value "True")
#    "AND" node with value False: find the sum of the minchange values for
#         all of its "False" children
#    "OR" node with value True: find the sum of the minchange values for all of
#         its "True" children
#    "OR" node with value False: choose the minimum minchange value of its
#         children (all of which must have value "False")

import sys
import math

class Node:
  def __init__(self,t,n,v):
    self.type = t # A or O
    self.nch = n  # number of children
    self.value = v # True, False, or -1 for "not yet determined"
    self.children = []
    self.change = 0

def dfs(n):
  if n.nch == 0:
    n.change = 1
    return
  m = math.inf
  sum = 0
  if n.type == 'A':
    value = True
    for c in n.children:
      dfs(c)
      value = value and c.value
      if c.value: m = min(m,c.change)
      else: sum += c.change
    n.value = value
    if value: n.change = m
    else: n.change = sum
  else:
    value = False
    for c in n.children:
      dfs(c)
      value = value or c.value
      if c.value: sum += c.change
      else: m = min(m,c.change)
    n.value = value
    if value: n.change = sum
    else: n.change = m

line = sys.stdin.readline().split()
l = int(line[0])
curtype = 'AO'.index(line[1])

# Level 0:
nroot = int(sys.stdin.readline()) # number of children of root (> 0)
nodes = []
nodes.append(Node(line[1],nroot,[True,False][1-curtype]))
node_cnt = 1
next_node = 0

# Levels 1 through l-1:
for i in range(1,l):
  curtype = 1-curtype # flip AND/OR
  line = sys.stdin.readline().split()
  while len(line) > 0:
    for j in range(nodes[next_node].nch):
      v = line[0]
      line = line[1:]
      if v == 'F' or v == 'T':
        v = [True,False]['TF'.index(v)]
        temp = Node('AO'[curtype],0,v)
        nodes[next_node].children.append(temp)
      else:
        temp = Node('AO'[curtype],int(v),[True,False][1-curtype])        
        nodes[next_node].children.append(temp)
        node_cnt += 1
        nodes.append(temp)
    next_node += 1

dfs(nodes[0])
print(nodes[0].change)
