/**
 * @file board.h
 * @author Matthew Getgen
 * @brief Header file storing methods performed on boards.
 * @date 2023-08-28
 * 
 * Dynamic memory allocation of a 2D array from [Techie Delight](https://www.techiedelight.com/dynamic-memory-allocation-in-c-for-2d-3d-array/)
 */

#ifndef BOARD_H
#define BOARD_H

#include "../defines.h"


/// @brief Data about the boards and the size of the boards.
struct Board {
    char **board1;
    char **board2;
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

/// @brief Deletes the boards. Useful for cleaning up the memory.
/// @param board board data structure.
void delete_boards(Board &board);


/* ────────────────────── *
 * UPDATE BOARD FUNCTIONS *
 * ────────────────────── */

/// @brief Stores a ship into a board.
/// @param ship Ship data structure to store.
/// @param board Board to store ship into.
/// @param value Value of the ship (SHIP or KILL).
void store_ship_to_board(Ship &ship, char **board, BoardValue value);

/// @brief Stores a shot value into a board.
/// @param shot Shot data structure to store.
/// @param board Board to store shot into.
void store_shot_to_board(Shot &shot, char **board);

#endif

