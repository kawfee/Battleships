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

void create_boards(Board &boards) {
    int size = boards.size;
    // dynamically create an array of pointers.
    boards.board1 = new char*[size];
    boards.board2 = new char*[size];

    // dynamically allocate memory of size `board_size` for each row.
    for (int i = 0; i < size; i++) {
        boards.board1[i] = new char[size];
        boards.board2[i] = new char[size];
    }
    return;
}

void clear_boards(Board &boards) {
    int size = boards.size;
    // assign values to the board
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            boards.board1[i][j] = WATER;
            boards.board2[i][j] = WATER;
        }
    }
    return;
}

void delete_boards(Board &boards) {
    // deallocate memory using the delete operator
    for (int i = 0; i < boards.size; i++) {
        delete[] boards.board1[i];
        delete[] boards.board2[i];
    }
    delete[] boards.board1;
    delete[] boards.board2;

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

