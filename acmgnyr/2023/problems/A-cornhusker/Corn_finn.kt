/**
 * Read the statement carefully and follow the instructions.
 *
 * @author Finn Lidbetter
 */
fun main(args: Array<String>) {
    val earVals = readln().split(" ").map{ it.toInt() }
    var kernelSampleTotal = 0
    for (i in 0..4) {
        kernelSampleTotal += earVals[2 * i] * earVals[2 * i + 1]
    }
    val kernelAverage = kernelSampleTotal / 5
    val weights = readln().split(" ").map{ it.toInt() }

    println((kernelAverage * weights[0]) / weights[1])
}