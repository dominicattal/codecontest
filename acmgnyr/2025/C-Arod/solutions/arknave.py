# Arnav Sastry

import math


ACUTE = 0
RIGHT = 1
OBTUSE = 2
DEGEN = 3


def np3(x):
    return x * (x - 1) * (x - 2)


def nc3(x):
    return x * (x - 1) * (x - 2) // 6


def count_side_side(ways, w, h):
    for x in range(1, w):
        num = x * x + h * h - w * x
        den = h

        v = max(0, min(h, num // den))

        counts = [max(0, v), 0, max(0, h - 1 - v)]
        if 0 < v < h and num % den == 0:
            assert counts[0] > 0
            counts[1] += 1
            counts[0] -= 1

        ways[RIGHT] += 4 * counts[1]
        ways[OBTUSE] += 4 * counts[2]

def solve(max_x, max_y):
    """
    Runs in O(XY(X + Y)).
    """
    # Counts of each triangle
    cats = [0, 0, 0, 0]
    for w in range(1, max_x + 1):
        for h in range(1, max_y + 1):
            triangles = 0
            ways = [0, 0, 0, 0]

            # 1. Use 3 corners
            # 4 triangles, 2 ways each to orient
            triangles += 4
            ways[RIGHT] += 4

            # 2. Use 2 opposite corners.
            # Cannot use any other corner or point on the main diagonal
            g = math.gcd(w, h)
            num_points = (w + 1) * (h + 1) - (g + 1) - 2
            triangles += num_points * 2
            ways[OBTUSE] += num_points * 2

            # 3. Use 2 same side corners
            triangles += 2 * (w - 1)
            for x in range(1, w):
                dot = x * x + h * h - w * x
                if dot < 0:
                    ways[OBTUSE] += 2
                elif dot == 0:
                    ways[RIGHT] += 2
            triangles += 2 * (h - 1)
            for y in range(1, h):
                dot = y * y + w * w - h * y
                if dot < 0:
                    ways[OBTUSE] += 2
                elif dot == 0:
                    ways[RIGHT] += 2

            count_side_side(ways, w, h)
            count_side_side(ways, h, w)

            triangles += 4 * (w - 1) * (h - 1)

            ways[ACUTE] = triangles - ways[RIGHT] - ways[OBTUSE]

            placements = (max_x + 1 - w) * (max_y + 1 - h)
            for d in range(4):
                cats[d] += ways[d] * placements

    cats[-1] = nc3((max_x + 1) * (max_y + 1)) - sum(cats)
    return cats

def main():
    x, y = map(int, input().split())
    res = solve(x, y)
    for x in res:
        print(x)


if __name__ == "__main__":
    main()

