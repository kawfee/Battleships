/**
 * @file player_example.h
 * @author Matthew Getgen
 * @brief The starter file for making your own AI.
 * @date 2022-11-22
 */

#ifndef PLAYER_EXAMPLE_H
#define PLAYER_EXAMPLE_H

#include "protected/Player.h"

using namespace std;

// PlayerExample inherits from/extends Player

// Rename this Class with your own class!
// Make sure to change all references to
// the `PlayerExample` class in both
// this file, and your c++ file

class PlayerExample: public Player {

    public:

        /// @brief Constructor for PlayerExample.
        PlayerExample();

        /// @brief Destructor for PlayerExample.
        ~PlayerExample();

        /// @brief Stores these values for the Player class
        /// and handles logic for the start of a match.
        /// This function is only called once.
        /// @param player player number for that match.
        /// @param board_size size of the board for that match.
        void handle_setup_match(PlayerNum player, int board_size);

        /// @brief Handles logic that happens at the start of every game.
        /// This function is called once for the number of games (usually 500).
        void handle_start_game();

        /// @brief Handles logic for placing a ship.
        /// This function is called a few times per game (usually 6 per game).
        /// @param ship_length length of the ship to be placed.
        /// @return Struct storing the ship to be placed.
        Ship choose_ship_place(int ship_length);

        /// @brief Handles logic for placing a shot.
        /// This function is called many times per game (usually 20-50).
        /// @return Struct storing the shot to take.
        Shot choose_shot();

        /// @brief Handles shots that are made by either player.
        /// This function is called many times per game (usually 20-50).
        /// @param player Which player took the shot. (either you or your opponent).
        /// @param shot Struct storing the shot and the board value.
        void handle_shot_return(PlayerNum player, Shot &shot);

        /// @brief Handles ships that have been killed by either player.
        /// This function is called a few times per game (up to 6).
        /// @param player Which player's ship died.
        /// @param ship Struct storing the ship that was killed.
        void handle_ship_dead(PlayerNum player, Ship &ship);

        /// @brief Handles logic for the end of a game.
        /// This function is called once for the number of games (usually 500).
        void handle_game_over();

        /// @brief Handles logic for the end of a match.
        /// This function is called once.
        void handle_match_over();

    private:

        // If you want to track other boards, make sure to add them to
        // the class  and to the <create/clear/delete>_boards() functions.

        /// @brief Defines what player you are in relation to the messages.
        PlayerNum player;

        /// @brief Defines the size of the board for the match.
        int board_size;

        /// @brief Stores the board where you placed ships, and where the opponent has shot.
        char** ship_board;

        /// @brief Stores where you have shot, and eventually where the opponent has kept their ships.
        char** shot_board;

        /// @brief Creates all boards for this player class in memory.
        void create_boards();

        /// @brief Fills the boards with their basic values, like WATER.
        void clear_boards();

        /// @brief Deletes the memory assigned to the boards.
        /// Used when the the match is over.
        void delete_boards();

        // Place any private PlayerExample functions here!
};

#endif

