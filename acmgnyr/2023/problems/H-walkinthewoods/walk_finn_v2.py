#!/usr/bin/python3

"""
Repeatedly do the following until you get stuck:
- Start tracking (location, direction) pairs that you have been to
- Follow edges and decrement their interest until you get stuck or reach a (location, direction) you are tracking
- Find the edge with the smallest interest that among those traversed before reaching the repeat position
- The edges traversed will be repeated this many times so decrement each of them by this amount.
- Reset the (location, direction) pairs being tracked.

Each repetition will remove an edge from the graph, so this can be repeated at most m times.
Each repetition will cover at most 4*n distinct (location, direction) pairs.

Time complexity: O(4*n*m)

author: Finn Lidbetter
"""

from enum import Enum
from collections import namedtuple
from sys import stdin


class Direction(Enum):
    N = "N"
    E = "E"
    S = "S"
    W = "W"

    def __repr__(self):
        return self.value

    __str__ = __repr__


Position = namedtuple("Position", ["node_id", "dir"])


class Node:
    def __init__(self, node_id, x, y):
        self.node_id = node_id
        self.x = x
        self.y = y
        self.n = Edge(-1, -1, 0)
        self.e = Edge(-1, -1, 0)
        self.s = Edge(-1, -1, 0)
        self.w = Edge(-1, -1, 0)

    def next_edge_node_id_dir(self, dir):
        if dir is Direction.N:
            edges = [edge for edge in (self.w, self.n, self.e) if edge.k > 0]
        elif dir is Direction.E:
            edges = [edge for edge in (self.n, self.e, self.s) if edge.k > 0]
        elif dir is Direction.S:
            edges = [edge for edge in (self.e, self.s, self.w) if edge.k > 0]
        else:
            edges = [edge for edge in (self.s, self.w, self.n) if edge.k > 0]
        if not edges:
            return None, None, None
        if len(edges) == 1 or len(edges) == 2:
            edge = edges[0]
        else:
            edge = edges[1]
        if edge == self.n:
            next_dir = Direction.N
        elif edge == self.e:
            next_dir = Direction.E
        elif edge == self.s:
            next_dir = Direction.S
        else:
            next_dir = Direction.W
        next_node_id = edge.u if edge.u != self.node_id else edge.v
        return edge, next_node_id, next_dir

    def __str__(self):
        return f'{self.node_id}: ({self.x}, {self.y})'


class Edge:

    def __init__(self, u, v, k):
        self.u = u
        self.v = v
        self.k = k

    def __repr__(self):
        return f'{self.u}--{self.v}: {self.k}'


def main():
    line = stdin.readline()
    tokens = line.split(' ')
    n = int(tokens[0])
    m = int(tokens[1])
    intersection_tokens = []
    counted = 0
    while counted < 2 * n:
        tokens = stdin.readline().strip().split(' ')
        intersection_tokens.extend(tokens)
        counted += len(tokens)
    nodes = []
    for i in range(n):
        x = int(intersection_tokens[2 * i])
        y = int(intersection_tokens[2 * i + 1])
        nodes.append(Node(i, x, y))
    edge_tokens = []
    counted = 0
    while counted < 3 * m:
        tokens = stdin.readline().strip().split(' ')
        edge_tokens.extend(tokens)
        counted += len(tokens)

    for i in range(m):
        id_1 = int(edge_tokens[3 * i]) - 1
        id_2 = int(edge_tokens[3 * i + 1]) - 1
        interest = int(edge_tokens[3 * i + 2])
        if interest <= 0:
            continue
        edge = Edge(min(id_1, id_2), max(id_1, id_2), interest)
        if nodes[id_1].x == nodes[id_2].x:
            if nodes[id_1].y > nodes[id_2].y:
                # id_1 is north of id_2.
                nodes[id_1].s = edge
                nodes[id_2].n = edge
            else:
                # id_1 is south of id_2.
                nodes[id_1].n = edge
                nodes[id_2].s = edge
        else:
            if nodes[id_1].x < nodes[id_2].x:
                # id_1 is west of id_2
                nodes[id_1].e = edge
                nodes[id_2].w = edge
            else:
                # id_2 is east of id_1
                nodes[id_1].w = edge
                nodes[id_2].e = edge
    tokens = stdin.readline().strip().split(' ')
    start_node = nodes[int(tokens[0]) - 1]
    start_dir = Direction(tokens[1])
    if start_dir is Direction.N:
        start_edge = start_node.n
    elif start_dir is Direction.E:
        start_edge = start_node.e
    elif start_dir is Direction.S:
        start_edge = start_node.s
    else:
        start_edge = start_node.w
    start_edge.k -= 1
    second_node_id = start_edge.u if start_edge.u != start_node.node_id else start_edge.v
    position = Position(second_node_id, start_dir)
    while True:
        vis = {}
        edges = []
        index = 0
        while position not in vis:
            # print(f'position: {position}')
            vis[position] = index
            next_edge, next_node_id, next_dir = nodes[position.node_id].next_edge_node_id_dir(position.dir)
            if next_node_id is None:
                node = nodes[position.node_id]
                print(f'{node.x} {node.y}')
                return
            next_edge.k -= 1
            edges.append(next_edge)
            index += 1
            position = Position(next_node_id, next_dir)
        loop_start_index = vis[position]
        single_edges = set()
        double_edges = set()
        # print(f'Loop edges: {edges[loop_start_index:]}')
        for edge in edges[loop_start_index:]:
            if edge in single_edges:
                double_edges.add(edge)
                single_edges.remove(edge)
            else:
                single_edges.add(edge)
        repeats = None
        for edge in single_edges:
            if repeats is None or edge.k < repeats:
                repeats = edge.k
        for edge in double_edges:
            if repeats is None or edge.k // 2 < repeats:
                repeats = edge.k // 2
        # print(f'repeats: {repeats}')
        for edge in single_edges:
            edge.k -= repeats
        for edge in double_edges:
            edge.k -= 2 * repeats
        # print()


if __name__ == "__main__":
    main()
