/**
 * Check all of the rules and then iterate over each character.
 * @author Finn Lidbetter
 */

fun main(args: Array<String>){
    var numLines = readln().toInt()
    for (lineNumber in 1..numLines) {
        var isbnLine = readln()
        var numHyphens = isbnLine.count { it == '-' }
        if (numHyphens > 3 || "--" in isbnLine || isbnLine.length - numHyphens != 10) {
            println("invalid")
            continue
        }
        if (numHyphens == 3 && isbnLine[isbnLine.length-2] != '-') {
            println("invalid")
            continue
        }
        if (isbnLine[0] == '-' || isbnLine[isbnLine.length-1] == '-') {
            println("invalid")
            continue
        }
        var numX = isbnLine.count { it == 'X' }
        if (numX > 1 || (numX == 1 && isbnLine[isbnLine.length - 1] != 'X')) {
            println("invalid")
            continue
        }
        var digitSum = isbnLine.filter {
            it.isDigit() || it == 'X'
        }.map {
            if (it == 'X') 10 else it.digitToInt()
        }.foldIndexed(0) {
            index, sum, element -> (sum + (index + 1) * element) % 11
        }
        if (digitSum != 0) {
            println("invalid")
            continue
        }
        var isbn13 = "978-" + isbnLine.substring(0, isbnLine.length - 1)
        var sum13 = isbn13.filter {
            it.isDigit()
        }.map {
            it.digitToInt()
        }.foldIndexed(0) {
            index, sum, element -> (if (index % 2 == 0) sum + element else sum + 3 * element) % 10
        }
        println(isbn13 + ((10 - sum13)%10).toString())
    }
}
