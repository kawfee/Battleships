/**
 * @file example_player_v2.h
 * @author Matthew Getgen
 * @brief The starter header file for making your own AI using the new PlayerV2 class!
 * @date 2026-05-29
 */

#ifndef EXAMPLE_PLAYER_H
#define EXAMPLE_PLAYER_H

#include "../PlayerV2.h"

using namespace std;

class ExamplePlayerV2: public PlayerV2 {
    
    public:

        ExamplePlayerV2();

        ~ExamplePlayerV2();

        void handle_setup_match(PlayerNum player, int board_size);

        void handle_start_game();

        vector<Ship> choose_ship_placements(vector<int> &ship_lengths);

        Shot choose_next_shot();

        void handle_shot_result(PlayerNum player, Shot shot);

        void handle_ship_dead(PlayerNum player, Ship ship);

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

