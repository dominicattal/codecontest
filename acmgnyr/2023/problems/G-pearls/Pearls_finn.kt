import java.util.*

/**
 * Try each direction at each step and obey all rules.
 * Stop exploring a path as soon as possible---i.e., evaluate
 * the rules as each segment of the necklace is placed.
 *
 * @author Finn Lidbetter
 */

enum class Direction {
    EAST, NORTH, SOUTH, WEST;

    fun opposite(): Direction {
        return when(this) {
            EAST -> WEST
            NORTH -> SOUTH
            SOUTH -> NORTH
            WEST -> EAST
        }
    }
    fun rowDelta(): Int {
        return when(this) {
            EAST -> 0
            NORTH -> -1
            SOUTH -> 1
            WEST -> 0
        }
    }
    fun colDelta(): Int {
        return when(this) {
            EAST -> 1
            NORTH -> 0
            SOUTH -> 0
            WEST -> -1
        }
    }
    fun getChar(): Char {
        return when(this) {
            EAST -> 'E'
            NORTH -> 'N'
            SOUTH -> 'S'
            WEST -> 'W'
        }
    }
}


fun main(args: Array<String>) {
    var tokens = readln().split(" ")
    val strLen = tokens[0].toInt()
    val nRows = tokens[1].toInt()
    val nCols = tokens[2].toInt()
    val beads = readln().toCharArray()
    tokens = readln().split(" ")
    val startRow = tokens[0].toInt() - 1
    val startCol = tokens[1].toInt() - 1

    var grid = Array(nRows) {
        BooleanArray(nCols)
    }
    var directions = arrayOfNulls<Direction>(beads.size)
    val result = solve(0, startRow, startCol, directions, 0, 0, grid, beads)
    if (result != null) {
        var sb = StringBuilder()
        for (dir in directions) {
            sb.append(dir!!.getChar())
        }
        println(sb.toString())
    } else {
        println("impossible")
    }
}


private fun isValidNextCell(row: Int, col: Int, grid: Array<BooleanArray>): Boolean {
    if (row < 0 || row >= grid.size || col < 0 || col >= grid[row].size) {
        return false
    }
    return !grid[row][col]
}

fun solve(
        index: Int,
        row: Int,
        col: Int,
        currDirections: Array<Direction?>,
        rowOffset: Int,
        colOffset: Int,
        grid: Array<BooleanArray>,
        beads: CharArray
): Array<Direction?>? {
//    println(Arrays.toString(currDirections))
    if (index == 0) {
        for (dir in Direction.values()) {
            val rowDelta = dir.rowDelta()
            val colDelta = dir.colDelta()
            val nextRow = row + rowDelta
            val nextCol = col + colDelta
            if (!isValidNextCell(nextRow, nextCol, grid)) {
                // Next cell is out of bounds or occupied.
//                println("\t$dir fails because out of bounds or occupied")
                continue
            }
            currDirections[0] = dir
            grid[nextRow][nextCol] = true
            val result = solve(index + 1, nextRow, nextCol, currDirections, rowDelta, colDelta, grid, beads)
            if (result != null) {
                return result
            }
            grid[nextRow][nextCol] = false
            currDirections[0] = null
        }
        return null
    }
    if (index == beads.size) {
        if (beads[0] == 'B' && currDirections[0] == currDirections[index - 1]) {
            // Need a corner through the first black bead.
//            println("\tLoop fails because we need a corner through the first black bead.")
            return null
        }
        if (beads[0] == 'W' && currDirections[0] != currDirections[index - 1]) {
            // Need to go straight through the first white bead.
//            println("\tLoop fails because we need to go straight through the first white bead.")
            return null
        }
        if (beads[index - 1] == 'B' && currDirections[index - 1] != currDirections[0]) {
            // Cannot have a corner immediately after the last black bead.
//            println("\tLoop fails because we cannot have a corner immediately after the last black bead.")
            return null
        }
        // White beads at indices [beads.size-1,0,1] need to have at least one corner before or after them.
        for (wCheckIndex in 0..2) {
            val i0 = wCheckIndex
            val i1 = (beads.size + wCheckIndex - 1) % beads.size
            val i2 = (beads.size + wCheckIndex - 2) % beads.size
            val i3 = (beads.size + wCheckIndex - 3) % beads.size
            val d0 = currDirections[i0]
            val d1 = currDirections[i1]
            val d2 = currDirections[i2]
            val d3 = currDirections[i3]
            if (beads[i1] == 'W' && d0 == d1 && d1 == d2 && d2 == d3) {
//                println("\tLoop fails because we need at least one corner before or after the starting/ending white beads.")
                return null
            }
        }
        return currDirections
    }
    val prevDirection = currDirections[index - 1]!!
    for (dir in Direction.values()) {
        if (dir == prevDirection.opposite()) {
            // Cannot go directly backwards.
//            println("\t$dir fails because backwards")
            continue
        }
        val rowDelta = dir.rowDelta()
        val colDelta = dir.colDelta()
        val nextRow = row + rowDelta
        val nextCol = col + colDelta
        val nextRowOffset = rowOffset + rowDelta
        val nextColOffset = colOffset + colDelta
        if (!isValidNextCell(nextRow, nextCol, grid)) {
            // Next cell is out of bounds or occupied.
//            println("\t$dir fails because out of bounds or occupied")
            continue
        }
        if (beads.size - (index + 1) < Math.abs(nextRowOffset) + Math.abs(nextColOffset)) {
            // We won't be able to make it back to starting location.
//            println("\t$dir fails because manhattan distance")
            continue
        }
        if (beads[index] == 'W' && dir != prevDirection) {
            // Corners are not allowed on white beads
//            println("\t$dir fails because no corners on white beads")
            continue
        }
        if (beads[index] == 'B' && dir == prevDirection) {
            // Corners are required on black beads
//            println("\t$dir fails because corners required on black beads")
            continue
        }
        if (beads[(index + 1) % beads.size] == 'B' && dir != prevDirection) {
            // Cannot have a corner immediately before a black bead.
//            println("\t$dir fails because no corners before black beads")
            continue
        }
        if (index > 0 && beads[index - 1] == 'B' && dir != prevDirection) {
            // Cannot have a corner immediatley after a black bead.
//            println("\t$dir fails because no corners after black beads")
            continue
        }
        if (index > 2 && beads[index - 1] == 'W' && dir == prevDirection && dir == currDirections[index - 2] && dir == currDirections[index - 3]) {
            // If there wasn't a corner before the white bead, then we need a corner after the white bead.
//            println("\t$dir fails because need at least one corner before/after white")
            continue
        }
        currDirections[index] = dir
        grid[nextRow][nextCol] = true
        val result = solve(index + 1, nextRow, nextCol, currDirections, nextRowOffset, nextColOffset, grid, beads)
        if (result != null) {
            return result
        }
        grid[nextRow][nextCol] = false
        currDirections[index] = null
    }
    return null
}
