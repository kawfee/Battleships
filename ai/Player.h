/**
 * @file Player.h
 * @author Matthew Getgen
 * @brief The Base Class for the Player AIs.
 * @date 2026-06-02
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

#include "definitions.h"

using namespace std;

class Player {

    public:

        /// @brief Base Player constructor. Clears the message buffer.
        Player() {
            this->socket_desc = 0;
            this->message.clear();
        };

        /// @brief Base Player destructor. Closes the socket_desc.
        ~Player() {
            // NOTE: if socket_desc is -1, it means an error occurred,
            // if it's 0, 1, of 2 it's stdin, stdout, and stderr
            if (this->socket_desc >= 3) {
                close(this->socket_desc);
            }
        };

        /// @brief Handles all message logic for the Player and inherited class meembers.
        /// @param socket_path Path to the socket.
        /// @param ai_name Name of the inherited AI.
        /// @param author_names Names of the authors of the inherited AI.
        /// @return 0 on success, -1 on error.
        int play_match(char *socket_path, const char *ai_name, const char *author_names);
        
        /// @brief Stores these values for the Player class
        /// and handles logic for the start of a match.
        /// This funciton is called once.
        /// @param player player number for that match.
        /// @param board_size size of the board for that match.
        virtual void handle_setup_match(PlayerNum player, int board_size) = 0;

        /// @brief Handles logic that happens at the start of every game.
        /// This function is called once for the number of games (usually 500).
        virtual void handle_start_game() = 0;

        /// @brief Handles logic for placing a ship.
        /// This function is called a few times per game (usually 6 per game).
        /// @param ship_length length of the ship to be placed.
        /// @return Struct storing the ship to be placed.
        virtual Ship choose_ship_place(int ship_length) = 0;

        /// @brief Handles logic for placing a shot.
        /// This function is called many times per game (usually 20-50).
        /// @return Struct storing the shot to take.
        virtual Shot choose_shot() = 0;

        /// @brief Handles shots that are made by either player.
        /// This function is called many times per game (usually 20-50).
        /// @param player Which player took the shot. (either you or your opponent).
        /// @param shot Struct storing the shot and the board value.
        virtual void handle_shot_return(PlayerNum player, Shot &shot) = 0;

        /// @brief Handles ships that have been killed by either player.
        /// This function is called a few times per game (up to 6).
        /// @param player Which player's ship died.
        /// @param ship Struct storing the ship that was killed.
        virtual void handle_ship_dead(PlayerNum player, Ship &ship) = 0;

        /// @brief Handles logic for the end of a game.
        /// This function is called once for the number of games (usually 500).
        virtual void handle_game_over() = 0;

        /// @brief Handles logic for the end of a match.
        /// This function is called once.
        virtual void handle_match_over() = 0;

    private:

        /// @brief The defined socket descriptor for the Player.
        int socket_desc;

        /// @brief Message buffer for sending and receiving messages.
        string message;

    protected:

        /// @brief Connects to the socket created by the server.
        /// @param socket_path Path to the socket.
        /// @return true on success, false on error.
        bool connect_to_socket(char *socket_path);

        /// @brief Sends a message on the message buffer.
        /// @return true on success, false on error.
        bool message_send();

        /// @brief Recieves a message from the message buffer.
        /// @return true on success, false on error.
        bool message_receive();

        /// @brief Creates a `hello` JSON message.
        /// @param ai_name Name of the inherited AI.
        /// @param author_names Names of the authors of the inherited AI.
        void message_hello_create(const char *ai_name, const char *author_names);

        /// @brief Creates a `ship_placed` JSON message.
        /// @param ships Vector of shots placed by the AI.
        void message_ships_placed_create(vector<Ship> ships);

        /// @brief Creates a `shot_taken` JSON message.
        /// @param shot Struct storing the shot to take.
        void message_shot_taken_create(Shot shot);
};

#endif

