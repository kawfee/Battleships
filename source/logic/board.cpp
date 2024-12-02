/**
 * @file board.cpp
 * @author Matthew Getgen
 * @brief Functionality for methods performed on boards.
 * @date 2022-09-26
 */

#include "board.h"


/* ──────────────────── *
 * MAIN BOARD FUNCTIONS *
 * ──────────────────── */

Board create_boards(int size) {
    Board board;
    assert(size >= MIN_BOARD_SIZE);
    assert(size <= MAX_BOARD_SIZE);
    board.size = size;

    return board;
}

void clear_boards(Board &board) {
    assert(board.size >= MIN_BOARD_SIZE);
    assert(board.size <= MAX_BOARD_SIZE);
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


/* ────────────────────── *
 * UPDATE BOARD FUNCTIONS *
 * ────────────────────── */

void store_ship_board_value(
    Board &board,
    PlayerNum num,
    Ship &ship,
    BoardValue value
) {
    assert(board.size >= MIN_BOARD_SIZE);
    assert(board.size <= MAX_BOARD_SIZE);
    assert(num == PLAYER_1 || num == PLAYER_2);
    assert(ship.row >= 0);
    assert(ship.row < board.size);
    assert(ship.col >= 0);
    assert(ship.col < board.size);
    assert(ship.dir == HORIZONTAL || ship.dir == VERTICAL);
    assert(
        (ship.dir == HORIZONTAL && (ship.col + (ship.len-1) < board.size)) ||
        (ship.dir == VERTICAL && (ship.row + (ship.len-1) < board.size))
    );

    int r = ship.row, c = ship.col;
    int rm = ship.dir == VERTICAL, cm = ship.dir == HORIZONTAL;
    int rv, cv;
    for (int l = 0; l < ship.len; l++) {
        rv = r + (l * rm);
        cv = c + (l * cm);
        if (num == PLAYER_1) board.board1[rv][cv] = value;
        else board.board2[rv][cv] = value;
    }
    return;
}

void store_shot_board_value(Board &board, PlayerNum num, Shot &shot) {
    assert(board.size >= MIN_BOARD_SIZE);
    assert(board.size <= MAX_BOARD_SIZE);
    assert(num == PLAYER_1 || num == PLAYER_2);
    assert(shot.row >= 0);
    assert(shot.row < board.size);
    assert(shot.col >= 0);
    assert(shot.col < board.size);
    if (num == PLAYER_1) board.board1[shot.row][shot.col] = shot.value;
    else board.board2[shot.row][shot.col] = shot.value;
    return;
}


/* ──────────────────── *
 * READ BOARD FUNCTIONS *
 * ──────────────────── */

BoardValue get_shot_board_value(Board &board, PlayerNum num, Shot &shot) {
    assert(board.size >= MIN_BOARD_SIZE);
    assert(board.size <= MAX_BOARD_SIZE);
    assert(num == PLAYER_1 || num == PLAYER_2);
    assert(shot.row >= 0);
    assert(shot.row < board.size);
    assert(shot.col >= 0);
    assert(shot.col < board.size);
    if (num == PLAYER_1) return (BoardValue)board.board1[shot.row][shot.col];
    else return (BoardValue)board.board2[shot.row][shot.col];
}

bool board_ship_died(Board &board, PlayerNum num, Ship &ship) {
    assert(board.size >= MIN_BOARD_SIZE);
    assert(board.size <= MAX_BOARD_SIZE);
    assert(num == PLAYER_1 || num == PLAYER_2);
    assert(ship.row >= 0);
    assert(ship.row < board.size);
    assert(ship.col >= 0);
    assert(ship.col < board.size);
    assert(ship.dir == HORIZONTAL || ship.dir == VERTICAL);
    assert(
        (ship.dir == HORIZONTAL && (ship.col + (ship.len-1) < board.size)) ||
        (ship.dir == VERTICAL && (ship.row + (ship.len-1) < board.size))
    );

    int r = ship.row, c = ship.col;
    int rm = ship.dir == VERTICAL, cm = ship.dir == HORIZONTAL;
    int rv, cv;
    BoardValue value;
    for (int l = 0; l < ship.len; l++) {
        rv = r + (l * rm);
        cv = c + (l * cm);
        if (num == PLAYER_1) value = (BoardValue)board.board1[rv][cv];
        else value = (BoardValue)board.board2[rv][cv];

        if (value != HIT && value != DUPLICATE_HIT) {
            return false;
        }
    }
    return true;
}

