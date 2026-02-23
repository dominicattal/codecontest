/**
 * Merge adjacent `1`s to make `2`s and discard any singleton `1`s.
 * Then merge adjacent `2`s to make `4`s and discard any singleton `2`s.
 * Repeat for all exponents.
 *
 * Keep track of the largest value in the array after each of these merge
 * operations.
 *
 * Complexity O(n*k) where k is the largest exponent.
 *
 * @author Finn Lidbetter
 */

val EXP_MAX = 111

fun main(args: Array<String>) {
    val n = readln().toInt()
    val inputArr = readln().split(" ")
    var expStrings = Array<String>(EXP_MAX + 1) {
        ""
    }
    var curr = "1"
    for (exp in 0..EXP_MAX) {
        expStrings[exp] = curr
        curr = doubleString(curr)
    }
    expStrings[EXP_MAX] = curr
    val stringMap = expStrings.withIndex().associate{ it.value to it.index }
    var exps = inputArr.map{ it -> stringMap.get(it)!! }.toCollection(ArrayList())
    var best = exps.maxOrNull() ?: 0
    for (exp in 0..EXP_MAX) {
        exps = merge(exps, exp)
        best = exps.maxOrNull() ?: best
    }
    println(expStrings[best])
}
fun merge(arr: ArrayList<Int>, mergeValue: Int): ArrayList<Int> {
    var nextArr = ArrayList<Int>()
    var prev = -1
    for (value in arr) {
        if (value == mergeValue) {
            if (prev == mergeValue) {
                nextArr.add(mergeValue + 1)
                prev = -1
            } else {
                prev = value
            }
        } else {
            nextArr.add(value)
            prev = -1
        }
    }
    return nextArr
}

fun doubleString(str: String): String {
    var sb = StringBuilder()
    var carry = 0
    for (i in str.length-1 downTo 0) {
        val ch = str[i]
        val digit = ch - '0'
        val doubled = digit * 2 + carry
        carry = doubled / 10
        sb.append(doubled % 10)
    }
    while (carry > 0) {
        sb.append(carry % 10)
        carry /= 10
    }
    sb.reverse()
    return sb.toString()
}