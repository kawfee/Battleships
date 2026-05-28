/**
 * @file PlayerV2.h
 * @author Matthew Getgen
 * @brief The Base Class for the Player V2 AIs.
 * @date 2026-05-24
 */

#ifndef PLAYER_V2_H
#define PLAYER_V2_H

#include <cerrno>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

#include "definitions.h"

using namespace std;

class PlayerV2 {
    public:
        PlayerV2() {
            this->socket_desc = 0;
            memset(this->msg, 0, MAX_MSG_SIZE+1);
        }

        ~Player() {
            // NOTE: if socket is -1 it means an error occurred,
            // if it's 0, 1, or 2 it's stdin, stdout, and stderr.
            if (this->socket_desc >= 3) {
                close(this->socket_desc);
            }
        }

        int play_match(char *socket_path, const char *ai_name, const char *author_names);

        virtual void handle_setup_match(PlayerNum player, int board_size) = 0;

        virtual void handle_start_game() = 0;

        virtual vector<Ship> choose_ship_placements(vector<int> &ship_lengths) = 0;

        virtual Shot choose_next_shot() = 0;

        virtual void handle_shot_result(PlayerNum player, Shot shot) = 0;

        virtual void handle_ship_dead(PlayerNum player, Ship ship) = 0;

        virtual void handle_game_over() = 0;

        virtual void handle_match_over() = 0;


    private:
        int socket_desc;

        char msg[MAX_MSG_SIZE+1];

    protected:
        int connect_to_socket(char *socket_path);

        int message_send();

        int message_receive(); 

        void message_hello_create(const char *ai_name, const char *author_names);

        void message_ships_placed_create(Ship *ships, int length);

        void message_shot_taken_create(Shot shot);
};

#endif // PLAYER_V2_H

