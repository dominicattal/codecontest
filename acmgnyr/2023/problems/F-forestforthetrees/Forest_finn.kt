/**
 * Preprocess the trees in a QuadTree data structure. Store the points
 * according to the indices of the diagonals that they lie on.
 * For a query, try the first offset relative to each tree and each direction
 * to get a candidate location. Query the QuadTree to check if the correct
 * number of trees are within sensor range of the candidate location. If
 * that is correct, then check each offset to verify that there is a tree
 * at each of the expected locations.
 *
 * N = number of trees
 * Q = number of queries
 * M = data points
 * K = coordinates max
 * Complexity: O(Q*N*(M + log(K)))
 *
 * @author Finn Lidbetter
 */

val COORD_MAX = 1000000000L
val COORD_MIN = -COORD_MAX

fun main(args: Array<String>) {
    var tokens = readln().split(" ")
    val numTrees = tokens[0].toInt()
    val numDataPoints = tokens[1].toInt()
    val sensorRange = tokens[2].toLong()
    var trees = ArrayList<Point>()
    var treesSet = HashSet<Point>()
    var quadTree = Node(
        Range(
            COORD_MIN - COORD_MAX,
            COORD_MIN - COORD_MAX,
            COORD_MAX + COORD_MAX,
            COORD_MAX + COORD_MAX
        )
    )
    for (i in 0..numTrees - 1) {
        tokens = readln().split(" ")
        val treePoint = Point(tokens[0].toLong(), tokens[1].toLong())
        trees.add(treePoint)
        treesSet.add(trees.get(i))
        val transformedPoint = Point(treePoint.x - treePoint.y, treePoint.x + treePoint.y)
        quadTree.insert(transformedPoint)
    }
    var offsets = ArrayList<Point>()
    for (i in 0..numDataPoints - 1) {
        tokens = readln().split(" ")
        val dx = tokens[0].toLong()
        val dy = tokens[1].toLong()
        offsets.add(Point(dx, dy))
        if (dx == 0L && dy == 0L) {
            println("Impossible")
            return
        }
    }
    // Try the first offset against each tree.
    var ambiguous = false
    var robotLocation: Point? = null
    for (tree in trees) {
        for (j in 0..3) {
            val candidatePoint = rotatedOffset(tree, offsets[0], j)
            val xMin = candidatePoint.x - sensorRange
            val yMin = candidatePoint.y - sensorRange
            val xMax = candidatePoint.x + sensorRange
            val yMax = candidatePoint.y + sensorRange
            val treeRange = Range(
                xMin - candidatePoint.y, xMin + candidatePoint.y, xMax - candidatePoint.y, xMax + candidatePoint.y
            )
            val treesInRange = quadTree.count(treeRange)
            if (treesInRange != numDataPoints) {
                continue
            }
            var goodCandidate = true
            for (offset in offsets) {
                val expectedTree = rotatedOffset(candidatePoint, offset, (j+2)%4)
                if (!treesSet.contains(expectedTree)) {
                    goodCandidate = false
                    break
                }
            }
            if (goodCandidate) {
                if (robotLocation == null) {
                    robotLocation = candidatePoint
                } else {
                    ambiguous = true
                    break
                }
            }
        }
        if (ambiguous) {
            break
        }
    }
    if (ambiguous) {
        println("Ambiguous")
    } else if (robotLocation == null) {
        println("Impossible")
    } else {
        val outputX = robotLocation.x
        val outputY = robotLocation.y
        println("$outputX $outputY")
    }
}

fun rotatedOffset(p1: Point, offset: Point, direction: Int): Point {
    if (direction == 0) {
        return Point(p1.x + offset.x, p1.y + offset.y)
    } else if (direction == 1) {
        return Point(p1.x + offset.y, p1.y - offset.x)
    } else if (direction == 2) {
        return Point(p1.x - offset.x, p1.y - offset.y)
    } else {
        return Point(p1.x - offset.y, p1.y + offset.x)
    }
}


data class Point(val x: Long, val y: Long)


class Range(var x1: Long, var y1: Long, var x2: Long, var y2: Long) {
    operator fun contains(pt: Point): Boolean {
        return x1 <= pt.x && pt.x <= x2 && y1 <= pt.y && pt.y <= y2
    }

    fun intersects(r: Range): Boolean {
        return !(x2 < r.x1 || r.x2 < x1 || y2 < r.y1 || r.y2 < y1)
    }

    operator fun contains(r: Range): Boolean {
        return x1 <= r.x1 && r.x2 <= x2 && y1 <= r.y1 && r.y2 <= y2
    }
}

class Node(var bound: Range) {
    val N_PTS = 10 // Can be modified
    var nw: Node? = null
    var ne: Node? = null
    var sw: Node? = null
    var se: Node? = null
    var pts: Array<Point?> = arrayOfNulls<Point>(N_PTS)
    var nPoints = 0
    var totalPoints = 0
    fun insert(pt: Point): Boolean {
        if (!bound.contains(pt)) return false
        totalPoints++
        if (nPoints < N_PTS) {
            pts[nPoints++] = pt
            return true
        }
        if (nw == null) subdivide()
        return nw!!.insert(pt) || ne!!.insert(pt) || sw!!.insert(pt) || se!!.insert(pt)
    }

    fun count(r: Range): Int {
        if (!bound.intersects(r)) return 0
        if (r.contains(bound)) return totalPoints
        var sum = 0
        for (i in 0 until nPoints) if (r.contains(pts[i]!!)) sum++
        return if (nw == null) sum else sum + nw!!.count(r) + ne!!.count(r) + sw!!.count(r) + se!!.count(r)
    }

    private fun subdivide() {
        val cx = (bound.x1 + bound.x2) / 2
        val cy = (bound.y1 + bound.y2) / 2
        nw = Node(Range(bound.x1, bound.y1, cx, cy))
        ne = Node(Range(cx + 1, bound.y1, bound.x2, cy))
        sw = Node(Range(bound.x1, cy + 1, cx, bound.y2))
        se = Node(Range(cx + 1, cy + 1, bound.x2, bound.y2))
    }
}
