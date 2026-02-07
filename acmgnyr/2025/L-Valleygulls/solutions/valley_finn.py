#!/usr/bin/python3

"""
Valley Gulls AC submission.

author: Finn Lidbetter
"""

from enum import Enum
from sys import stdin
from collections import namedtuple


Pt = namedtuple('Pt', ['x', 'y'])

X0 = None
XN = None
REACHED_PTS = set()
INF = 1000000000


def gcd(a, b):
    return a if b == 0 else gcd(b, a % b)


class Frac:
    def __init__(self, n, d):
        assert not isinstance(n, float) and not isinstance(d, float)
        gcf = gcd(abs(n), abs(d))
        self.n = n // gcf
        self.d = d // gcf
        if self.d < 0:
            self.n *= -1
            self.d *= -1

    def __add__(self, other):
        if isinstance(other, int):
            return self + Frac(other, 1)
        if isinstance(other, float):
            return float(self) + other
        return Frac(self.n * other.d + self.d * other.n, self.d * other.d)

    def __radd__(self, other):
        return self + other

    def __sub__(self, other):
        if isinstance(other, int):
            return self - Frac(other, 1)
        if isinstance(other, float):
            return float(self) - other
        return self + other.negate()

    def __rsub__(self, other):
        return self.negate() + other

    def __mul__(self, other):
        if isinstance(other, int):
            return self * Frac(other, 1)
        if isinstance(other, float):
            return float(self) * other
        return Frac(self.n * other.n, self.d * other.d)

    def __rmul__(self, other):
        return self * other

    def __truediv__(self, other):
        if isinstance(other, int):
            return self / Frac(other, 1)
        if isinstance(other, float):
            return float(self) / other
        return self * other.reciprocal()

    def __rtruediv__(self, other):
        return self.reciprocal() * other

    def negate(self):
        return Frac(-self.n, self.d)

    def reciprocal(self):
        return Frac(self.d, self.n)

    def __float__(self):
        return self.n / self.d

    def __abs__(self):
        if self.n < 0:
            return Frac(-self.n, self.d)
        return self

    def __lt__(self, other):
        if isinstance(other, int):
            return self < Frac(other, 1)
        if isinstance(other, float):
            return float(self) < other
        return self.n * other.d < self.d * other.n

    def __eq__(self, other):
        if isinstance(other, int):
            return self == Frac(other, 1)
        if isinstance(other, float):
            return float(self) == other
        return self.n * other.d == self.d * other.n

    def __gt__(self, other):
        return not self < other and not self == other

    def __le__(self, other):
        return self < other or self == other

    def __ge__(self, other):
        return self > other or self == other

    def __neg__(self):
        return self.negate()

    def __format__(self, format_spec):
        return f"{self.n}/{self.d}"

    def __hash__(self):
        return hash((self.n, self.d))


class Linear:
    def __init__(self, m, b):
        self.m = m
        self.b = b

    def evaluate(self, x):
        return self.m * x + self.b

    def solve(self, y):
        return (y - self.b) / self.m

    def min_in_range(self, lo, hi):
        return min(self.evaluate(lo), self.evaluate(hi))

    def max_in_range(self, lo, hi):
        return max(self.evaluate(lo), self.evaluate(hi))

    def integrate(self, lo, hi):
        quadratic = Quadratic(self.m/2, self.b, 0)
        return quadratic.evaluate(hi) - quadratic.evaluate(lo)


class Quadratic:
    def __init__(self, a, b, c):
        self.a = a
        self.b = b
        self.c = c

    def evaluate(self, x):
        return self.a*x*x + self.b*x + self.c

    def __str__(self):
        return f"{self.a}x^2 {'+' if self.b >= 0 else '-'} {abs(self.b)}x {'+' if self.c >= 0 else '-'} {abs(self.c)}"


class PiecewiseLinear(Linear):

    def __init__(self, index, x_lo, x_hi, m, b):
        super().__init__(m, b)
        self.index = index
        self.x_lo = x_lo
        self.x_hi = x_hi
        self.increasing = m > 0
        self.decreasing = m < 0
        self.y_lo = super().min_in_range(x_lo, x_hi)
        self.y_hi = super().max_in_range(x_lo, x_hi)
        self.nest_pts = []

    def min_in_range(self, lo, hi):
        return super().min_in_range(max(lo, self.x_lo), min(hi, self.x_hi))

    def max_in_range(self, lo, hi):
        return super().max_in_range(max(lo, self.x_lo), min(hi, self.x_hi))

    def minimum(self):
        return self.min_in_range(self.x_lo, self.x_hi)

    def maximum(self):
        return self.max_in_range(self.x_lo, self.x_hi)

    def __str__(self):
        linear_str = f"{self.m}x {'+' if self.b >= 0 else '-'} {abs(self.b)}"
        return f"{linear_str}: [{self.x_lo}, {self.x_hi}]"


class RainBucket:

    def __init__(self, *, x_lo, x_hi, line_left, line_right, agg_rate, active_rate, water_y):
        self.index = None
        self.x_lo = x_lo
        self.x_hi = x_hi
        self.line_left = line_left
        self.line_right = line_right
        self.agg_rate = agg_rate
        self.active_rate = active_rate
        self.water_y = water_y
        self.water_area = 0
        self.left_peaked = False
        self.right_peaked = False

    def next_y(self):
        if self.left_peaked or self.right_peaked:
            return INF, None, None
        if self.line_left is None:
            assert self.line_right is not None, "Error! Bucket with None left and right!"
            for pt in self.line_right.nest_pts:
                if pt in REACHED_PTS:
                    continue
                return pt.y, EventType.NEST, pt
            y = self.line_right.maximum()
            if self.x_hi == self.line_right.x_hi:
                return y, EventType.RIGHT_PEAKED, None
            return y, EventType.RIGHT_LINE_CHANGE, None
        if self.line_right is None:
            for pt in self.line_left.nest_pts:
                if pt in REACHED_PTS:
                    continue
                return pt.y, EventType.NEST, pt
            y = self.line_left.maximum()
            if self.x_lo == self.line_left.x_lo:
                return y, EventType.LEFT_PEAKED, None
            return y, EventType.LEFT_LINE_CHANGE, None
        best_y = INF
        best_pt = None
        for pt in self.line_left.nest_pts:
            if pt in REACHED_PTS:
                continue
            best_y = pt.y
            best_pt = pt
            break
        for pt in self.line_right.nest_pts:
            if pt in REACHED_PTS:
                continue
            if pt.y > best_y:
                break
            best_y = pt.y
            best_pt = pt
            break
        left_max_y = self.line_left.maximum()
        right_max_y = self.line_right.maximum()
        if best_pt is not None and best_y <= left_max_y and best_y <= right_max_y:
            return best_y, EventType.NEST, best_pt
        if left_max_y < right_max_y:
            if self.line_left.x_lo != X0 and self.line_left.x_lo == self.x_lo:
                return left_max_y, EventType.LEFT_PEAKED, None
            return left_max_y, EventType.LEFT_LINE_CHANGE, None
        if self.line_right.x_hi != XN and self.line_right.x_hi == self.x_hi:
            return right_max_y, EventType.RIGHT_PEAKED, None
        return right_max_y, EventType.RIGHT_LINE_CHANGE, None

    def time_to_reach(self, target_y):
        """Assume that line_left and line_right are valid until height y."""
        if target_y == INF:
            return INF
        if self.active_rate == 0:
            return INF
        area_to_fill = area(self.line_left, self.line_right, self.water_y, target_y) - self.water_area
        return area_to_fill / self.active_rate

    def merge(self, other):
        if self.x_lo < other.x_lo:
            merged_line_left = self.line_left
            merged_line_right = other.line_right
        else:
            merged_line_left = other.line_left
            merged_line_right = self.line_right
        return RainBucket(
            x_lo=min(self.x_lo, other.x_lo), 
            x_hi=max(self.x_hi, other.x_hi), 
            line_left=merged_line_left,
            line_right=merged_line_right,
            agg_rate=max(self.agg_rate, other.agg_rate), 
            active_rate=max(self.agg_rate, other.agg_rate),
            water_y=max(self.water_y, other.water_y)
        )
    
    def advance_time(self, delta):
        """Assume that advancing this much time does not change line_left nor line_right."""
        if self.active_rate == 0 or delta <= 0:
            return
        target_area = delta * self.active_rate
        self.water_area += target_area

    def advance_to_y(self, new_y):
        self.water_y = new_y
        self.water_area = 0

    def __str__(self):
        return f"[{self.x_lo:.3f}, {self.x_hi:.3f}], water={self.water_y}, line_left: {self.line_left}, line_right: {self.line_right}, agg_rate={self.agg_rate}, active_rate={self.active_rate}"


def area(line_left, line_right, height_1, height_2):
    if line_left is None:
        x1l = X0
        x2l = X0
    else:
        x1l = line_left.solve(height_1)
        x2l = line_left.solve(height_2)
    if line_right is None:
        x1r = XN
        x2r = XN
    else:
        x1r = line_right.solve(height_1)
        x2r = line_right.solve(height_2)
    dx1 = x1r - x1l
    dx2 = x2r - x2l
    dx_avg = (dx1 + dx2) / 2
    return dx_avg * (height_2 - height_1)



class EventType(Enum):
    NEST = 0
    LEFT_PEAKED = 1
    RIGHT_PEAKED = 2
    LEFT_LINE_CHANGE = 3
    RIGHT_LINE_CHANGE = 4


def get_min_time_to_next_event(buckets):
    best_time = INF
    best_y, best_event, best_pt = None, None, None
    best_bucket = None
    for bucket in buckets:
        y, event, pt = bucket.next_y()
        delta = bucket.time_to_reach(y)
        if delta < best_time:
            best_time = delta
            best_event, best_pt = event, pt
            best_bucket = bucket
            best_y = y
    return best_time, best_bucket, best_event, best_pt, best_y


def main():
    tokens = stdin.readline().split(" ")
    p = int(tokens[0])
    r = Frac(0, 1)
    denom = 1
    for ch in tokens[1]:
        if ch == ".":
            continue
        r += Frac(int(ch), denom)
        denom *= 10

    num_nest_pts = int(tokens[2])
    pts = []
    while len(pts) < p:
        tokens = stdin.readline().strip().split(" ")
        for i in range(0, len(tokens), 2):
            x = Frac(int(tokens[i]), 1)
            y = Frac(int(tokens[i + 1]), 1)
            pts.append(Pt(x, y))
    global X0
    global XN
    X0 = pts[0].x
    XN = pts[-1].x

    lines = []
    for index, (p1, p2) in enumerate(zip(pts[:-1], pts[1:])):
        m = (p2.y - p1.y) / (p2.x - p1.x)
        b = p1.y - m * p1.x
        lines.append(PiecewiseLinear(index, p1.x, p2.x, m, b))

    tokens = stdin.readline().split(" ")
    nest_pts = []
    for i in range(num_nest_pts):
        nest_x = Frac(int(tokens[i]), 1)
        for line in lines:
            if line.x_lo < nest_x < line.x_hi:
                nest_y = line.evaluate(nest_x)
                nest_pts.append(Pt(nest_x, nest_y))
                line.nest_pts.append(nest_pts[-1])
                break
    for line in lines:
        line.nest_pts.sort(key=lambda pt: pt.y)
    # Get "rain buckets" between peaks.
    # Track water level (y coordinate) for each bucket.
    # Track left line and right line for each bucket. (Handle left cliff and right cliff)
    # Water level starts at the valley between the peaks (there should be one unless it is at the cliff edge)
    # Track active rainfall rate.
    # Track cumulative rainfall rate.
    # For each bucket see which one takes the smallest amount of time to reach the next "event" then advance all buckets by that amount of time.
    # Events consist of: 
    # - Changing from one line to another in a bucket.
    # - Reaching a nest.
    # - Reaching a peak.
    #   - When a peak is reached we either overflow or merge buckets.
    #   - In overflow scenario we preserve the left or right line but get an active rate of 0.
    #   - Cumulative rate is width plus overflow rate from other buckets.
    #   - In merge scenario we delete the two old buckets and create a new bucket.
    buckets = []
    index = 0
    x_lo = X0
    line_left = None
    decreasing = True
    while index < len(lines):
        if decreasing:
            if lines[index].increasing:
                # Start ascending.
                water_y = lines[index].evaluate(lines[index].x_lo)
                line_right = lines[index]
                decreasing = False
            else:
                # Continue going down into a valley.
                line_left = lines[index]
        else:
            if lines[index].increasing:
                # Continue coming out of a valley.
                x_hi = lines[index].x_hi
            else:
                # Reached a new peak.
                x_hi = lines[index].x_lo
                bucket = RainBucket(
                    x_lo=x_lo,
                    x_hi=x_hi,
                    line_left=line_left,
                    line_right=line_right,
                    agg_rate=r * (x_hi - x_lo),
                    active_rate=r * (x_hi - x_lo),
                    water_y=water_y,
                )
                buckets.append(bucket)

                decreasing = True
                x_lo = lines[index].x_lo
                line_left = lines[index]
        index += 1
    if decreasing:
        line_right = None
        x_hi = XN
        water_y = lines[-1].evaluate(lines[-1].x_hi)
        bucket = RainBucket(
            x_lo=x_lo,
            x_hi=x_hi,
            line_left=line_left,
            line_right=line_right,
            agg_rate=r * (x_hi - x_lo),
            active_rate=r * (x_hi - x_lo),
            water_y=water_y,
        )
        buckets.append(bucket)
    else:
        x_hi = XN
        bucket = RainBucket(
            x_lo=x_lo,
            x_hi=x_hi,
            line_left=line_left,
            line_right=line_right,
            agg_rate=r * (x_hi - x_lo),
            active_rate=r * (x_hi - x_lo),
            water_y=water_y,
        )
        buckets.append(bucket)

    for index, bucket in enumerate(buckets):
        bucket.index = index

    # print("Buckets")
    # for bucket in buckets:
    #     print(str(bucket))
    # print()

    curr_time = Frac(0, 1)
    nest_reached = {nest_pt: None for nest_pt in nest_pts}
    iterations = 0
    while any(reach_time is None for reach_time in nest_reached.values()) and iterations < 10:
        # iterations += 1
        time_to_next_event, best_bucket, event_type, nest_pt, best_y = get_min_time_to_next_event(buckets)
        # print(f"{time_to_next_event}, {best_bucket}, {event_type}, {nest_pt}, {best_y}")
        # print()
        for bucket in buckets:
            if bucket == best_bucket:
                bucket.advance_to_y(best_y)
            else:
                bucket.advance_time(time_to_next_event)
        curr_time += time_to_next_event
        if nest_pt is not None:
            nest_reached[nest_pt] = curr_time
            REACHED_PTS.add(nest_pt)
        if event_type is EventType.RIGHT_LINE_CHANGE:
            if best_bucket.line_right.index < len(lines) - 1:
                best_bucket.line_right = lines[best_bucket.line_right.index + 1]
            else:
                best_bucket.line_right = None
        if event_type is EventType.LEFT_LINE_CHANGE:
            if best_bucket.line_left.index > 0:
                best_bucket.line_left = lines[best_bucket.line_left.index - 1]
            else:
                best_bucket.line_left = None
        if event_type is EventType.LEFT_PEAKED:
            best_bucket.left_peaked = True
            best_bucket.active_rate = 0
            # Bucket to the left gets this bucket's agg_rate.
            left_bucket = buckets[best_bucket.index - 1]
            if left_bucket.right_peaked:
                # Merge buckets
                merged_bucket = left_bucket.merge(best_bucket)
                new_buckets = [bucket for bucket in buckets[:left_bucket.index]]
                new_buckets.append(merged_bucket)
                new_buckets.extend([bucket for bucket in buckets[best_bucket.index + 1:]])
                for index, new_bucket in enumerate(new_buckets):
                    new_bucket.index = index
                buckets = new_buckets
            else:
                left_bucket.agg_rate += best_bucket.agg_rate
                while left_bucket.active_rate == 0:
                    left_bucket = buckets[left_bucket.index - 1]
                    left_bucket.agg_rate += best_bucket.agg_rate
                left_bucket.active_rate = left_bucket.agg_rate
        if event_type is EventType.RIGHT_PEAKED:
            best_bucket.right_peaked = True
            best_bucket.active_rate = 0
            # Bucket to the right gets this bucket's agg_rate
            right_bucket = buckets[best_bucket.index + 1]
            if right_bucket.left_peaked:
                # Merge buckets
                merged_bucket = best_bucket.merge(right_bucket)
                new_buckets = [bucket for bucket in buckets[:best_bucket.index]]
                new_buckets.append(merged_bucket)
                new_buckets.extend([bucket for bucket in buckets[right_bucket.index+1:]])
                for index, new_bucket in enumerate(new_buckets):
                    new_bucket.index = index
                buckets = new_buckets
            else:
                right_bucket.agg_rate += best_bucket.agg_rate
                while right_bucket.active_rate == 0:
                    right_bucket = buckets[right_bucket.index + 1]
                    right_bucket.agg_rate += best_bucket.agg_rate
                right_bucket.active_rate = right_bucket.agg_rate

    for nest_pt in nest_pts:
        # print(f"{nest_reached[nest_pt]}")
        print(float(nest_reached[nest_pt]))

if __name__ == "__main__":
    main()

