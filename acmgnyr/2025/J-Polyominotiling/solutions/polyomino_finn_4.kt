/**
 * Separately find the xyxy factorizations and xyzxyz factorizations.
 *
 * For the xyxy case, consider A B = X Y XBar YBar where A=X Y and B=XBar YBar (so |A|=|B|=k/2).
 * The factorization  exists for a split X Y of A iff B is the reversed complement of a rotation of A.
 * For each starting position for A (each rotation of the original string) use KMP to count occurrences
 * of B in the reverse-complement of A.
 * 
 * For the xyzxyz case we use the method of Gambini, Vuillon 2007.
 *
 * Assume that X will contain the first letter. We will multiply
 * our final answer by 6 to account for the cyclic rotations.
 *
 * First we find a pair, (X', XBar') of maximal length to get a factorization
 *    X U XBar V
 *
 * Assume that there is a factorization X Y Z XBar YBar ZBar where |X'|>|X|.
 * Then we can write X' = LXR (where at least one of L,R is nonempty) there is
 * some Y' such that X Y Z XBar YBar ZBar = X R Y' Z' RBar XBar LBar YBar' ZBar' L.
 * This gives
 *      Y = R Y'
 *      Z = Z' RBar.
 * Thus
 *      YBar = YBar' RBar
 *      ZBar = R ZBar'
 * We use this to rebuild X Y Z XBar YBar ZBar as
 *   X Y Z XBar YBar' RBar R ZBar'
 * But this gives
 *    RBar R
 * as a factor. This is impossible because the last letter of RBar is equal to
 * the complement of the first letter of R. So we have one of 'rl','lr','ud','du'
 * in the boundary string. This is impossible by the definition of the polyomino.
 *
 * So any factorization must have (X', XBar') of maximal length.
 *
 * So we have a factorization X' U XBar' V. We are only interested in continuing if |U| == |V|.
 * We then want to check if we can factorize U such that U = Y Z and V = YBar ZBar.
 * If such a factorization exists then it is provided by an occurrence of
 *    UBar = ZBar YBar
 * in
 *   V V = YBar ZBar YBar ZBar
 * So compute UBar and V V and use the Knuth-Morris-Pratt algorithm to count such occurrences.
 * Each occurrence corresponds to a different factorization U = Y Z.
 * Note that we need to exclude the case where Y or Z is empty, i.e., UBar = V.
 *
 * @author Finn Lidbetter
 */



fun main() {
    val tokens = readLine()!!.split(" ")
    val n = tokens[0].toInt()
    println(solve(tokens[1]))
}

fun solve(s: String): Int {
    val seq = s.toCharArray()
    val n = seq.size
    var pseudoSquareCount = 0
    var pseudoHexagonCount = 0
    pseudoSquareCount = countFactorizations(s)
    val zeroComplement = complement(seq[0])
    // Count pseudo-hexagons
    for (xBarIndex in 1..n-1) {
        if (seq[xBarIndex] != zeroComplement) {
            continue
        }
        var xL = 0
        var xR = 0
        var xBarL = xBarIndex
        var xBarR = xBarIndex
        // Expand to the left.
        while (seq[(xL - 1 + n) % n] == complement(seq[(xBarR + 1) % n])) {
            xL--
            xBarR++
            xL = (xL + n) % n
            xBarR %= n
        }
        // Expand to the right.
        while (seq[(xR + 1) % n] == complement(seq[(xBarL - 1 + n) % n])) {
            xR++
            xBarL--
            xR %= n
            xBarL = (xBarL + n) % n
        }
        // We have a factorization: X U XBar V
        var uL = (xR + 1) % n
        var uR = (xBarL - 1 + n) % n
        var vL = (xBarR + 1) % n
        var vR = (xL - 1 + n) % n
        var uLen = uR - uL + 1
        var vLen = vR - vL + 1
        if (uLen <= 0) {
            uLen += n
        }
        if (vLen <= 0) {
            vLen += n
        }
        if (uLen != vLen) {
            continue
        }

        // var uArray = CharArray(uLen) { '0' }
        var uComplementArray = CharArray(uLen) { '0' }
        for (uIndex in 0..uLen - 1) {
            uComplementArray[uIndex] = complement(seq[(uL + uIndex) % n])
            // uArray[uIndex] = seq[(uL + uIndex) % n]
        }
        val uBar = uComplementArray.reversed().joinToString("")
        // val u = uArray.joinToString("")
        var vvArray = CharArray(vLen * 2) { '0' }
        for (vvIndex in 0..2*vLen - 1) {
            vvArray[vvIndex] = seq[(vL + (vvIndex % vLen)) % n]
        }
        val vv = vvArray.joinToString("")

        pseudoHexagonCount += 6 * countOccurrences(vv, uBar, vv.length)
        if (uBar==vv.slice(0..vLen-1)) {
            // Subtract 12 for the pseudo-hexagons that are actually pseduo-squares.
            pseudoHexagonCount -= 12
        }
    }
    return pseudoSquareCount + pseudoHexagonCount
}


// Count occurrences of pattern in text where start indices are less than maxStart (inclusive 0..maxStart-1)
fun countOccurrences(text: String, pattern: String, maxStart: Int): Int {
    if (pattern.isEmpty()) return 0
    val pi = kmpHelper(pattern)
    var j = 0
    var count = 0
    for (i in text.indices) {
        while (j > 0 && text[i] != pattern[j]) j = pi[j - 1]
        if (text[i] == pattern[j]) j++
        if (j == pattern.length) {
            val start = i - pattern.length + 1
            if (start < maxStart) count++
            j = pi[j - 1]
        }
    }
    return count
}


fun kmpHelper(pat: String): IntArray {
    val m = pat.length
    val arr = IntArray(m)
    var i = 1
    var len = 0
    while (i < m) {
        if (pat[i] == pat[len]) {
            arr[i++] = ++len
        } else {
            if (len > 0) len = arr[len - 1]
            else i++
        }
    }
    return arr
}


/**
 * Count the number of factorizations: X Y X' Y'
 */
fun countFactorizations(s: String): Int {
    val N = s.length
    if (N == 0 || N % 2 == 1) return 0
    val half = N / 2
    val T = s + s

    var total = 0
    for (start in 0 until N) {
        val A = T.substring(start, start + half)
        val B = T.substring(start + half, start + N)
        val rcA = reverseComplement(A)
        val doubled = rcA + rcA
        total += countOccurrences(doubled, B, half)
    }
    return total
}

fun reverseComplement(s: String): String {
    val n = s.length
    val arr = CharArray(n)
    for (i in 0 until n) arr[i] = complement(s[n - 1 - i])
    return String(arr)
}

fun complement(ch: Char): Char {
    if (ch == 'u') {
        return 'd'
    }
    if (ch == 'd') {
        return 'u'
    }
    if (ch == 'r') {
        return 'l'
    }
    if (ch == 'l') {
        return 'r'
    }
    return '0'
}

