#!/usr/bin/python3

NODE_TYPES = ['A','O']

tree = [['X',0,0,False,0]]

# line 1: nLevels rootType
# lines 2-(nLevels+1): level info

(nLevelsStr,rootType) = input().split(' ')
nLevels = int(nLevelsStr)

# node: [type, nChildren, firstChildIndex, value, cost]

levelType = rootType
if rootType == 'A':
    typeIndex = 0
else:
    typeIndex = 1

nextNode = 0

for i in range(nLevels):
    nodeList = input().split(' ')

    for node in nodeList:
        if node == 'T':
            tree[nextNode] = ['T',0,0,True,1]
        elif node == 'F':
            tree[nextNode] = ['F',0,0,False,1]
        else:
            nextChild = len(tree)
            tree.extend([['X',0,0,False,0]] * int(node))
            tree[nextNode] = [levelType,int(node),nextChild,False,0]

        nextNode = nextNode + 1

    typeIndex = 1 - typeIndex
    levelType = NODE_TYPES[typeIndex]

for node in reversed(tree):
    # print(node)
    if node[0] == 'A':
        node[3] = True
        childIndex = node[2]
        minCost = tree[childIndex][4]

        for i in range(node[1]):
            if tree[childIndex][3] == False:
                node[3] = False
                node[4] += tree[childIndex][4]
            elif tree[childIndex][4] < minCost:
                minCost = tree[childIndex][4]

            childIndex = childIndex + 1

        if node[3]:
            node[4] = minCost

    elif node[0] == 'O':
        node[3] = False
        childIndex = node[2]
        minCost = tree[childIndex][4]

        for i in range(node[1]):
            if tree[childIndex][3] == True:
                node[3] = True
                node[4] += tree[childIndex][4]
            elif tree[childIndex][4] < minCost:
                minCost = tree[childIndex][4]

            childIndex = childIndex + 1

        if not node[3]:
            node[4] = minCost

    # print(node)

print(tree[0][4])
