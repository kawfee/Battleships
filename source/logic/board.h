/**
 * @file board.h
 * @author Matthew Getgen
 * @brief Header file storing methods performed on boards.
 * @date 2023-08-28
 */

#ifndef BOARD_H
#define BOARD_H

#include "../defines.h"


/// @brief Data about the boards and the size of the boards.
struct Board {
    char board1[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    char board2[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int size;
};


/* ──────────────────── *
 * MAIN BOARD FUNCTIONS *
 * ──────────────────── */

/// @brief Creates the boards for a match.
/// @param size intended size of the row/col of each board.
/// @return Board struct storing board info.
Board create_boards(int size);

/// @brief Clears the boards. Needed between each game.
/// @param board board struct to clear.
void clear_boards(Board &board);


/* ────────────────────── *
 * UPDATE BOARD FUNCTIONS *
 * ────────────────────── */

/// @brief Stores a ship into a board.
/// @param board board struct to use.
/// @param num number representing the player board to store to.
/// @param ship Ship data structure to store.
/// @param value Value of the ship (SHIP or KILL).
void store_ship_board_value(
    Board &board,
    PlayerNum num,
    Ship &ship,
    BoardValue value
);

/// @brief Stores a shot value into a board.
/// @param board board struct to use.
/// @param num number representing the player board to store to.
/// @param shot Shot data structure to store.
void store_shot_board_value(Board &board, PlayerNum num, Shot &shot);


/* ──────────────────── *
 * READ BOARD FUNCTIONS *
 * ──────────────────── */

/// @brief Gets a shot value from a board.
/// @param board Board struct to use.
/// @param num number to decide which board.
/// @param shot Shot data structure to store.
/// @return BoardValue value of board.
BoardValue get_shot_board_value(Board &board, PlayerNum num, Shot &shot);

/// @brief Calculates whether a ship is dead or not.
/// @param board board struct to use.
/// @param num number to decide which board.
/// @param ship Ship structure to check the value of.
/// @return True if ship dead, false if not.
bool board_ship_died(Board &board, PlayerNum num, Ship &ship);

#endif

