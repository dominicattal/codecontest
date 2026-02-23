/**
 * Build up two auxiliary arrays
 *  1. The value at each index is the smallest value at or to the right of that index.
 *  2. The value at each index is the largest value at or to the left of that index.
 * After constructing the arrays, iterate over the original values
 * and check for the pivot property.
 *
 * Time complexity: O(n)
 *
 * @author Finn Lidbetter
 */

import java.util.*;
val INF = 987654321
val OUTPUT_LIMIT = 100

fun main(args: Array<String>) {
    var vals = readln().split(" ").filterIndexed {
        index, element -> index != 0
    }.map {
        it.toInt()
    }
    var reversed = vals.reversed()
    var smallestRight: List<Int> = reversed.runningFold(INF) {
        total, element -> Math.min(total, element)
    }.drop(1)
    Collections.reverse(smallestRight)
    var largestLeft = vals.runningFold(-1) {
        total, element -> Math.max(total, element)
    }.drop(1)
    var outputVals = ArrayList<Int>()
    vals.forEachIndexed { index, element ->
        var goodPivot = true
        if (index > 0 && element < largestLeft[index - 1]) {
            goodPivot = false
        }
        if (index < vals.size - 1 && element >= smallestRight[index + 1]) {
            goodPivot = false
        }
        if (goodPivot) {
            outputVals.add(element)
        }
    }
    var sb = StringBuilder()
    sb.append(outputVals.size)
    for (i in 0..Math.min(OUTPUT_LIMIT, outputVals.size) - 1) {
        sb.append(" ")
        sb.append(outputVals[i])
    }
    println(sb)
}