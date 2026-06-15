/**
 * @file example_player.cpp
 * @author Matthew Getgen
 * @brief The starter C++ file for making your own AI using the Player class!
 * @date 2026-05-29
 */

#include "example_player.h"

#define AI_NAME "Example Player C++"

#define AUTHOR_NAMES "Example Team/Author Name"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PRINT_ERROR("AI Requires a socket path!");
        return 1;
    }

    char *socket_path = argv[1];

    srand(getpid());

    ExamplePlayer my_player = ExamplePlayer();
    if (my_player.play_match(socket_path, AI_NAME, AUTHOR_NAMES) != 0) {
        return 1;
    }
    return 0;
}

ExamplePlayer::ExamplePlayer():Player() {
    this->ship_lengths = {};
    return;
}

ExamplePlayer::~ExamplePlayer() {
    return;
}

void ExamplePlayer::handle_setup_match(PlayerNum player, int board_size) {
    this->player = player;
    this->board_size = board_size;
    create_boards();
}

void ExamplePlayer::handle_start_game() {
    clear_boards();
    this->ship_lengths.clear();
}

Ship ExamplePlayer::choose_ship_place(int ship_length) {
    Ship ship = {};

    ship.len = ship_length;
    ship.row = this->ship_lengths.size();
    ship.col = 0;
    ship.dir = HORIZONTAL;
    this->ship_lengths.push_back(ship_length);
    
    // store ship to board
    int row_multiplier = ship.dir == VERTICAL;
    int col_multiplier = ship.dir == HORIZONTAL;
    for (int i = 0; i < ship.len; i++) {
        int row = ship.row + (i * row_multiplier);
        int col = ship.col + (i * col_multiplier);
        this->ship_board[row][col] = SHIP;
    }
    
    return ship;
}

Shot ExamplePlayer::choose_shot() {
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

void ExamplePlayer::handle_shot_return(PlayerNum player, Shot &shot) {
    if (player == this->player) {
        this->shot_board[shot.row][shot.col] = shot.value;
    } else {
        this->ship_board[shot.row][shot.col] = shot.value;
    }
}

void ExamplePlayer::handle_ship_dead(PlayerNum player, Ship &ship) {
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

void ExamplePlayer::handle_game_over() {
}

void ExamplePlayer::handle_match_over() {
    delete_boards();
}

void ExamplePlayer::create_boards() {
    int size = this->board_size;

    this->ship_board = new char*[size];
    this->shot_board = new char*[size];

    for (int i = 0; i < size; i++) {
        this->ship_board[i] = new char[size];
        this->shot_board[i] = new char[size];
    }
}

void ExamplePlayer::clear_boards() {
    for (int row = 0; row < this->board_size; row++) {
        for (int col = 0; col < this->board_size; col++) {
            this->ship_board[row][col] = WATER;
            this->shot_board[row][col] = WATER;
        }
    }
}

void ExamplePlayer::delete_boards() {
    for (int i = 0; i <this->board_size; i++) {
        delete[] this->ship_board[i];
        delete[] this->shot_board[i];
    }
    delete[] this->ship_board;
    delete[] this->shot_board;
}

