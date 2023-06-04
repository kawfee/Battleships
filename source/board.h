/**
 * @file board.h
 * @author Matthew Getgen
 * @brief Header file storing methods performed on boards.
 * @date 2022-09-26
 * 
 * Dynamic memory allocation of a 2D array from [Techie Delight](https://www.techiedelight.com/dynamic-memory-allocation-in-c-for-2d-3d-array/)
 * 
 */

#include "defines.h"


/* ──────────────────── *
 * MAIN BOARD FUNCTIONS *
 * ──────────────────── */

/**
 * @brief Create the boards for the match.
 * @param boards board data structure for storing the boards.
 */
void create_boards(Board &boards);

/**
 * @brief Clears the boards for each game.
 * @param boards board data structure.
 */
void clear_boards(Board &boards);

/**
 * @brief Deletes the boards at the end of the match.
 * @param boards board data structure.
 */
void delete_boards(Board &boards);


/* ────────────────────── *
 * UPDATE BOARD FUNCTIONS *
 * ────────────────────── */

/**
 * @brief Stores a ship into a board.
 * @param ship ship data structure to store.
 * @param board board to store ship into.
 * @param value value of ship (SHIP or KILL).
 */
void store_ship_to_board(Ship &ship, char **board, BoardValue value);

/**
 * @brief Stores a shot value into a board.
 * @param shot shot data structure to store.
 * @param board board to store shot into.
 */
void store_shot_to_board(Shot &shot, char **board);

