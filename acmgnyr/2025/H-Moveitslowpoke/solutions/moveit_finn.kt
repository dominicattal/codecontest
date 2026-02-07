/**
 * Run dijkstra's algorithm on a graph where we keep track
 * of the "continuous distance" travelled, the current node,
 * and the previous node.
 *
 * V <= n*n*d
 * E <= n*V
 * Time complexity: O((V+E)log(V))
 */

import java.util.*

fun main() {
    val tokens = readLine()!!.split(" ")
    val n = tokens[0].toInt()
    val m = tokens[1].toInt()
    val k = tokens[2].toInt()
    val d = tokens[3].toInt()
    val s = tokens[4].toInt() - 1
    val t = tokens[5].toInt() - 1

    var dist = Array<IntArray>(n) {
        IntArray(n){ -1 }
    }
    var continuous = Array<Array<BooleanArray>>(n) {
        Array(n) {
            BooleanArray(n)
        }
    }
    for (i in 0..m-1) {
        val edge = readLine()!!.split(" ")
        val a = edge[0].toInt() - 1
        val b = edge[1].toInt() - 1
        val l = edge[2].toInt()
        dist[a][b] = l
        dist[b][a] = l
    }
    for (i in 0..k-1) {
        val continuity = readLine()!!.split(" ")
        val a = continuity[0].toInt() - 1
        val b = continuity[1].toInt() - 1
        val c = continuity[2].toInt() - 1
        continuous[a][b][c] = true
    }


    val start = Node(s, 0, -1)
    val pq = PriorityQueue<QNode>()
    val visited = HashSet<Node>()
    val bestDist = HashMap<Node, Int>()
    var bestTDist = -1
    pq.offer(QNode(start, 0))
    bestDist.put(start, 0);
    while (!pq.isEmpty()) {
        val qNode = pq.remove()
        val curr = qNode.node
        if (visited.contains(curr)) {
            continue
        }
        visited.add(curr)
        for (i in 0..n-1) {
            if (dist[curr.id][i] == -1) {
                continue
            }
            if (i==curr.prev) {
                // No uturns.
                continue
            }
            var nextNode: Node? = null
            if (curr.prev == -1) {
                nextNode = Node(i, dist[curr.id][i], curr.id)
            } else {
                if (continuous[curr.prev][curr.id][i]) {
                    if (curr.contDist + dist[curr.id][i] > d) {
                        continue
                    }
                    nextNode = Node(i, curr.contDist + dist[curr.id][i], curr.id)
                } else {
                    nextNode = Node(i, dist[curr.id][i], curr.id)
                }
            }
            val nextDist = qNode.totalDist + dist[curr.id][i]
            if (!bestDist.containsKey(nextNode) || nextDist < bestDist.get(nextNode)!!) {
                if (nextNode.id == t && (bestTDist == -1 || nextDist < bestTDist)) {
                    bestTDist = nextDist
                }
                bestDist.put(nextNode, nextDist)
                pq.offer(QNode(nextNode, nextDist))
            }
        }
    }
    if (bestTDist == -1) {
        println("impossible")
    } else {
        println(bestTDist)
    }
}
class Node(val id: Int, val contDist: Int, val prev: Int) {
    override fun equals(other: Any?): Boolean {
        val node2 = other as Node
        return id == node2.id && contDist == node2.contDist && prev == node2.prev
    }

    override fun hashCode(): Int {
        return Objects.hash(id, contDist, prev)
    }
}
class QNode(val node: Node, val totalDist: Int): Comparable<QNode> {
    override fun compareTo(other: QNode) = compareValuesBy(this, other) {it.totalDist}
}

