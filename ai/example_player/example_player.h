/**
 * @file example_player.h
 * @author Matthew Getgen
 * @brief The starter header file for making your own AI using the Player class!
 * @date 2026-05-29
 */

#ifndef EXAMPLE_PLAYER_H
#define EXAMPLE_PLAYER_H

#include "../Player.h"

using namespace std;

class ExamplePlayer: public Player {
    
    public:

        ExamplePlayer();

        ~ExamplePlayer();

        void handle_setup_match(PlayerNum player, int board_size);

        void handle_start_game();

        Ship choose_ship_place(int ship_length);

        Shot choose_shot();

        void handle_shot_return(PlayerNum player, Shot &shot);

        void handle_ship_dead(PlayerNum player, Ship &ship);

        void handle_game_over();

        void handle_match_over();

    private:

        PlayerNum player;

        int board_size;

        vector<int> ship_lengths;

        char **ship_board;

        char **shot_board;

        void create_boards();

        void clear_boards();

        void delete_boards();
};

#endif // EXAMPLE_PLAYER_H

