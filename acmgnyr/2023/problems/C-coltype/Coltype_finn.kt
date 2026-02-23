/**
 * Try each possible ending value that could be reached by a
 * starting value less than 2^64. Note that while the starting value
 * will be less than 2^64, the final power of two may be larger than
 * 2^64. If the sequence alternates OEOEOEOEOEO... then it can grow
 * by a factor of 1.5 with every pair of terms. Apply the operations
 * in reverse. If an invalid state is reached (i.e., an even number
 * after an odd operation in reverse, or reaching a power of two)
 * then the ending value was invalid.
 *
 * Complexity:
 *  n = STR_LEN_MAX
 *  m = START_EXP_MAX
 *  O(n*(n+m)*m)
 *
 *
 * @author Finn Lidbetter
 */

import java.math.BigInteger
import kotlin.math.log

val STR_LEN_MAX = 50
val START_EXP_MAX = 64

fun main(args: Array<String>) {
    val seq = readln().toCharArray()
    val END_EXP_MAX = START_EXP_MAX + (STR_LEN_MAX * log(1.5, 2.0) / 2.0).toInt() + 10
    val TWO = BigInteger("2")
    val THREE = BigInteger("3")
    val UPPER_BOUND = BigInteger.ONE.shiftLeft(START_EXP_MAX)
    var best = BigInteger(UPPER_BOUND.toString())
    for (endExp in 4..END_EXP_MAX) {
        var curr = BigInteger.ONE.shiftLeft(endExp)
        var valid = true
        for (index in 0..seq.size - 1) {
            val ch = seq[seq.size - index - 1]
            if (ch == 'E') {
                curr = curr.multiply(TWO)
            } else {
                // ch == 'O'
                if (!curr.mod(THREE).equals(BigInteger.ONE)) {
                    valid = false
                    break
                }
                curr = curr.subtract(BigInteger.ONE)
                curr = curr.divide(THREE)
                if (curr.mod(TWO).equals(BigInteger.ZERO)) {
                    valid = false
                    break
                }
            }
            if (curr.bitCount() == 1) {
                // curr is a power of two.
                valid = false
                break
            }
        }
        if (valid && curr.compareTo(best) < 0) {
            best = curr
        }
    }
    if (best.compareTo(UPPER_BOUND) < 0) {
        println(best.toString())
    } else {
        println("INVALID")
    }
}