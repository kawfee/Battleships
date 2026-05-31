/**
 * @file example_player_v2.cpp
 * @author Matthew Getgen
 * @brief The starter C++ file for making your own AI using the new PlayerV2 class!
 * @date 2026-05-29
 */

#include "example_player_v2.h"

#define AI_NAME "Example Player V2 C++"

#define AUTHOR_NAMES "Example Team/Author Name"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRINT_ERROR("AI Requires a socket path!");
        return 1;
    }

    char *socket_path = argv[1];

    srand(getpid());

    ExamplePlayerV2 my_player = ExamplePlayerV2();
    if (!my_player.play_match(socket_path, AI_NAME, AUTHOR_NAMES)) {
        return 1;
    }
    return 0;
}

ExamplePlayerV2::ExamplePlayerV2():PlayerV2() {
    this->ship_lengths = {};
    return;
}

ExamplePlayerV2::~ExamplePlayerV2() {
    return;
}

void ExamplePlayerV2::handle_setup_match(PlayerNum player, int board_size) {
    this->player = player;
    this->board_size = board_size;
    create_boards();
}

void ExamplePlayerV2::handle_start_game() {
    clear_boards();
    this->ship_lengths.clear();
}

vector<Ship> ExamplePlayerV2::choose_ship_placements(vector<int> &ship_lengths) {
    vector<Ship> ships = {};

    for (int length : ship_lengths) {
        Ship ship = {};
        ship.len = length;
        ship.row = this->ship_lengths.size();
        ship.col = 0;
        ship.dir = HORIZONTAL;
        ships.push_back(ship);
        this->ship_lengths.push_back(length);

        // store ship to board
        int row_multiplier = ship.dir == VERTICAL;
        int col_multiplier = ship.dir == HORIZONTAL;
        for (int i = 0; i < ship.len; i++) {
            int row = ship.row + (i * row_multiplier);
            int col = ship.col + (i * col_multiplier);
            this->ship_board[row][col] = SHIP;
        }
    }

    return ships;
}

Shot ExamplePlayerV2::choose_next_shot() {
    Shot shot = {};
    shot.row = 0;
    shot.col = 0;

    for (int row = 0; row < this->board_size; row++) {
        for (int col = 0; col < this->board_size; col++) {
            if (this->shot_board[row][col] == WATER) {
                shot.row = row;
                shot.col = col;
                return shot;
            }
        }
    }
    return shot;
}

void ExamplePlayerV2::handle_shot_result(PlayerNum player, Shot shot) {
    if (player == this->player) {
        this->shot_board[shot.row][shot.col] = shot.value;
    } else {
        this->ship_board[shot.row][shot.col] = shot.value;
    }
}

void ExamplePlayerV2::handle_ship_dead(PlayerNum player, Ship ship) {
    int row_multiplier = ship.dir == VERTICAL;
    int col_multiplier = ship.dir == HORIZONTAL;
    for (int i = 0; i < ship.len; i++) {
        int row = ship.row + (i * row_multiplier);
        int col = ship.col + (i * col_multiplier);
        if (player == this->player) {
            this->ship_board[row][col] = KILL;
        } else {
            this->shot_board[row][col] = KILL;
        }
    }
}

void ExamplePlayerV2::handle_game_over() {
}

void ExamplePlayerV2::handle_match_over() {
    delete_boards();
}

void ExamplePlayerV2::create_boards() {
    int size = this->board_size;

    this->ship_board = new char*[size];
    this->shot_board = new char*[size];

    for (int i = 0; i < size; i++) {
        this->ship_board[i] = new char[size];
        this->shot_board[i] = new char[size];
    }
}

void ExamplePlayerV2::clear_boards() {
    for (int row = 0; row < this->board_size; row++) {
        for (int col = 0; col < this->board_size; col++) {
            this->ship_board[row][col] = WATER;
            this->shot_board[row][col] = WATER;
        }
    }
}

void ExamplePlayerV2::delete_boards() {
    for (int i = 0; i <this->board_size; i++) {
        delete[] this->ship_board[i];
        delete[] this->shot_board[i];
    }
    delete[] this->ship_board;
    delete[] this->shot_board;
}

