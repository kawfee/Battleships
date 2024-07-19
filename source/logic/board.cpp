/**
 * @file board.cpp
 * @author Matthew Getgen
 * @brief Functionality for methods performed on boards.
 * @date 2022-09-26
 * 
 * Dynamic memory allocation of a 2D array from [Techie Delight](https://www.techiedelight.com/dynamic-memory-allocation-in-c-for-2d-3d-array/)
 * 
 */

#include "board.h"


/* ──────────────────── *
 * MAIN BOARD FUNCTIONS *
 * ──────────────────── */

Board create_boards(int size) {
    Board board;
    board.size = size;
    // dynamically create an array of pointers.
    board.board1 = new char*[size];
    board.board2 = new char*[size];

    // dynamically allocate memory of size `board_size` for each row.
    for (int i = 0; i < size; i++) {
        board.board1[i] = new char[size];
        board.board2[i] = new char[size];
    }
    return board;
}

void clear_boards(Board &board) {
    int size = board.size;
    // assign values to the board
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            board.board1[i][j] = WATER;
            board.board2[i][j] = WATER;
        }
    }
    return;
}

void delete_boards(Board &board) {
    // deallocate memory using the delete operator
    for (int i = 0; i < board.size; i++) {
        delete[] board.board1[i];
        delete[] board.board2[i];
    }
    delete[] board.board1;
    delete[] board.board2;

    return;
}


/* ────────────────────── *
 * UPDATE BOARD FUNCTIONS *
 * ────────────────────── */

void store_ship_to_board(Ship &ship, char **board, BoardValue value) {
    int r = ship.row, c = ship.col;
    for (int l = 0; l < ship.len; l++) {
        if (ship.dir == HORIZONTAL) board[r][c+l] = value;
        else                        board[r+l][c] = value;

    }
    return;
}

void store_shot_to_board(Shot &shot, char **board) {
    board[shot.row][shot.col] = shot.value;
    return;
}

