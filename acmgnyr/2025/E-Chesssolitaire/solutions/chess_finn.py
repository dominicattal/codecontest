#!/bin/python3

"""
Perform a breadth first search with recursive backtracking.
Sort the pieces by row and column before making each move.

author: Finn Lidbetter
"""

from collections import namedtuple
from sys import stdin

PiecePosition = namedtuple('PiecePosition', ['row', 'col', 'piece'])
Move = namedtuple('Move', ['row_1', 'col_1', 'row_2', 'col_2', 'piece'])


def move_str(move):
    return f"{move.piece}: {chr(move.row_1 + ord('A'))}{move.col_1 + 1} -> {chr(move.row_2 + ord('A'))}{move.col_2 + 1}"

def piece_str(piece):
    return f"{piece.piece}: {chr(piece.row + ord('A'))}{piece.col + 1}"

def has_clear_path(start_row, start_col, end_row, end_col, row_dir, col_dir, piece_positions):
    occupied_positions = {(piece.row,piece.col) for piece in piece_positions}
    curr_row = start_row
    curr_col = start_col
    while curr_row + row_dir != end_row or curr_col + col_dir != end_col:
        curr_row += row_dir
        curr_col += col_dir
        if (curr_row, curr_col) in occupied_positions:
            return False
    return True

def can_take(piece_1, piece_2, piece_positions):
    if piece_1 == piece_2:
        raise ValueError("A piece cannot take itself!")
    row_delta = piece_2.row - piece_1.row
    col_delta = piece_2.col - piece_1.col
    row_dir = row_delta // abs(row_delta) if row_delta != 0 else 0
    col_dir = col_delta // abs(col_delta) if col_delta != 0 else 0
    if piece_1.piece == 'N':
        if abs(row_delta) == 1 and abs(col_delta) == 2:
            return True
        if abs(row_delta) == 2 and abs(col_delta) == 1:
            return True
        return False
    if piece_1.piece == 'B':
        if abs(row_delta) != abs(col_delta):
            return False
        return has_clear_path(piece_1.row, piece_1.col, piece_2.row, piece_2.col, row_dir, col_dir, piece_positions)
    if piece_1.piece == 'R':
        if abs(row_delta) > 0 and abs(col_delta) > 0:
            return False
        return has_clear_path(piece_1.row, piece_1.col, piece_2.row, piece_2.col, row_dir, col_dir, piece_positions)
    if piece_1.piece == 'Q':
        if abs(row_delta) == abs(col_delta) or abs(row_delta) == 0 or abs(col_delta) == 0:
            return has_clear_path(piece_1.row, piece_1.col, piece_2.row, piece_2.col, row_dir, col_dir, piece_positions)
        return False
    if piece_1.piece == 'K':
        if abs(row_delta) > 1 or abs(col_delta) > 1:
            return False
        return True
    raise ValueError(f"Unknown piece type: {piece_1.piece}")

def swap(arr, index1, index2):
    arr[index1], arr[index2] = arr[index2], arr[index1]

def solve(piece_positions, moves):
    pieces_remaining = len(piece_positions) - len(moves)
    if pieces_remaining == 1:
        return moves
    sorted_pieces = sorted(piece_positions[:pieces_remaining])
    original_index = {piece: index for index, piece in enumerate(piece_positions[:pieces_remaining])}
    for piece_index_1 in range(len(sorted_pieces)):
        piece_1 = sorted_pieces[piece_index_1]
        for piece_index_2 in range(len(sorted_pieces)):
            if piece_index_1 == piece_index_2:
                continue
            piece_2 = sorted_pieces[piece_index_2]
            if can_take(piece_1, piece_2, sorted_pieces):
                moves.append(Move(piece_1.row, piece_1.col, piece_2.row, piece_2.col, piece_1.piece))
                piece_positions[original_index[piece_1]] = PiecePosition(piece_2.row, piece_2.col, piece_1.piece)
                swap(piece_positions, original_index[piece_2], pieces_remaining - 1)
                result = solve(piece_positions, moves)
                if result is not None:
                    return result
                swap(piece_positions, original_index[piece_2], pieces_remaining - 1)
                piece_positions[original_index[piece_1]] = piece_1
                moves.pop()
    return None


def main():
    tokens = stdin.readline().split(" ")
    num_pieces = int(tokens[1])
    piece_positions = []
    for _ in range(num_pieces):
        tokens = stdin.readline().split(" ")
        piece = tokens[0]
        row = ord(tokens[1][0]) - ord('A')
        col = int(tokens[1][1]) - 1
        piece_positions.append(PiecePosition(row, col, piece))
    moves = []
    result = solve(piece_positions, moves)
    if result is None:
        print("No solution")
        return
    for move in moves:
        print(move_str(move))


if __name__ == "__main__":
    main()
