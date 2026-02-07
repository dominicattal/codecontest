#!/bin/python3

"""
Construct the tree then recursively find the minimum cost to change the value
at a node.
If we are at an AND level that currently evaluates to TRUE, then we just
need to change the value of 1 child to FALSE. Try each child and take the minimum.
If we are at an AND level that currently evaluates to FALSE, then we need to
change the value of all children that evaluate to FALSE to TRUE. So recurse on
those children and take the sum.
If we are at an OR level that currently evaluates to TRUE, then we need to
change the value of all children that evaluate to TRUE to FALSE. So recurse on
those children and take the sum.
If we are at an OR level that currently evaluates to FALSE, then we just
need to change the value of 1 child to TRUE. Try each child and take the minimum.

Time complexity: O(k) where k is the number of nodes in the tree.

author: Finn Lidbetter
"""

from sys import stdin

class Node:
    def __init__(self, node_id, parent_id, num_children, leaf_value, depth):
        self.node_id = node_id
        self.parent = parent_id
        self.num_children = num_children
        self.children = []
        self.depth = depth
        self.leaf_value = leaf_value
        self.computed_value = leaf_value

    def __str__(self):
        return f'Node(id={self.node_id}, children={[child.node_id for child in self.children]})'

    def evaluate(self, curr_and):
        if self.computed_value is not None:
            return self.computed_value
        if curr_and:
            for child in self.children:
                child.evaluate(False)
            self.computed_value = all(child.evaluate(False) for child in self.children)
        else:
            for child in self.children:
                child.evaluate(True)
            self.computed_value = any(child.evaluate(True) for child in self.children)
        return self.computed_value

    def cost_to_flip(self, curr_and):
        if self.leaf_value is not None:
            return 1
        if curr_and:
            # AND case
            if self.computed_value:
                # Need to change one child to F
                return min(child.cost_to_flip(not curr_and) for child in self.children)
            else:
                # Need to change all F children to T
                return sum(
                    child.cost_to_flip(not curr_and) for child in self.children
                    if child.computed_value is False
                )
        else:
            # OR case
            if self.computed_value:
                # Need to change all T children to F
                return sum(
                    child.cost_to_flip(not curr_and) for child in self.children
                    if child.computed_value is True
                )
            else:
                # Need to change one child to T
                return min(child.cost_to_flip(not curr_and) for child in self.children)


def main():
    tokens = stdin.readline().strip().split(' ')
    levels = int(tokens[0])
    root_and = tokens[1] == 'A'
    nodes = []
    node_id = 0
    root_children = int(stdin.readline())
    root = Node(node_id, None, root_children, None, 0)
    node_id += 1
    nodes.append(root)
    parent_id = 0
    for depth in range(1, levels):
        tokens = stdin.readline().strip().split(' ')
        for level_index, token in enumerate(tokens):
            while len(nodes[parent_id].children) >= nodes[parent_id].num_children:
                parent_id += 1
            if token == 'T':
                node = Node(node_id, parent_id, 0, True, depth)
            elif token == 'F':
                node = Node(node_id, parent_id, 0, False, depth)
            else:
                num_children = int(token)
                node = Node(node_id, parent_id, num_children, None, depth)
            nodes[parent_id].children.append(node)
            nodes.append(node)
            node_id += 1
    root_value = nodes[0].evaluate(root_and)

    print(nodes[0].cost_to_flip(root_and))


if __name__ == "__main__":
    main()
