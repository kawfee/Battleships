/**
 * @file player_example.cpp
 * @author Matthew Getgen
 * @brief The starter file for making your own AI.
 * @date 2022-11-22
 */

#include "player_example.h"


// Write your AI's name here. Please don't make it more than 64 bytes.
#define AI_NAME "Player Example C++"

// Write your name(s) here. Please don't make it more than 64 bytes.
#define AUTHOR_NAMES "Mamthew Gemchin & Goey Jorski"


int main(int argc, char *argv[]) {
    // player must have the socket path as an argument.
    if ( argc != 2 ) {
        printf("%s Error: Requires socket name! (line: %d)\n", AI_NAME, __LINE__);
        return -1;
    }
    char *socket_path = argv[1];
    
    // set random seed
    srand(getpid());

    PlayerExample my_player = PlayerExample();
    return my_player.play_match(socket_path, AI_NAME, AUTHOR_NAMES);
}

PlayerExample::PlayerExample():Player() {
    return;
}

PlayerExample::~PlayerExample() {
    return;
}

void PlayerExample::handle_setup_match(PlayerNum player, int board_size) {
    this->player = player;
    this->board_size = board_size;
    create_boards();
    return;
}

void PlayerExample::handle_start_game() {
    clear_boards();
    return;
}

Ship PlayerExample::choose_ship_place(int ship_length) {
    Ship ship;
    ship.len = ship_length;
    ship.row = 0;
    ship.col = 0;
    ship.dir = HORIZONTAL;
    bool ship_okay = false;

    for (int row = 0; row < this->board_size; row++) {
        for (int col = 0; col < this->board_size-(ship.len-1); col++) {
            if ( this->ship_board[row][col] == WATER ) {
                ship_okay = true;
                for (int len = 0; len < ship.len; len++) {
                    if ( this->ship_board[row][col+len] != WATER ) {
                        ship_okay = false;
                        break;
                    }
                }
                if (ship_okay) {
                    for (int len = 0; len < ship.len; len++) {
                        this->ship_board[row][col+len] = SHIP;
                    }
                    ship.row = row;
                    ship.col = col;
                    return ship;
                }
            }
        }
    }
    return ship;
}

Shot PlayerExample::choose_shot() {
    Shot shot;
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

void PlayerExample::handle_shot_return(PlayerNum player, Shot &shot) {
    // your shot was returned, store it
    if ( player == this->player ) {
        this->shot_board[shot.row][shot.col] = shot.value;
    }
    // their shot was returned, store it
    else {
        this->ship_board[shot.row][shot.col] = shot.value;
    }
    return;
}

void PlayerExample::handle_ship_dead(PlayerNum player, Ship &ship) {
    // store the ship that was killed
    for (int i = 0; i < ship.len; i++) {
        if ( player == this->player ) { // your ship is dead
            if      (ship.dir == HORIZONTAL) this->ship_board[ship.row][ship.col+i] = KILL;
            else if (ship.dir == VERTICAL)   this->ship_board[ship.row+i][ship.col] = KILL;
        } else {             // their ship is dead
            if      (ship.dir == HORIZONTAL) this->shot_board[ship.row][ship.col+i] = KILL;
            else if (ship.dir == VERTICAL)   this->shot_board[ship.row+i][ship.col] = KILL;
        }
    }
    return;
}

void PlayerExample::handle_game_over() {
    return;
}

void PlayerExample::handle_match_over() {
    delete_boards();
    return;
}

void PlayerExample::create_boards() {
    int size = this->board_size;
    // dynamically create an array of pointers.
    this->ship_board = new char*[size];
    this->shot_board = new char*[size];
    //this->int_board = new int*[size];

    // dynamically allocate memory of size `board_size` for each row.
    for (int i = 0; i < size; i++) {
        this->ship_board[i] = new char[size];
        this->shot_board[i] = new char[size];
    }
    return;
}

void PlayerExample::clear_boards() {
    // assign WATER to the boards
    for (int i = 0; i < this->board_size; i++) {
        for (int j = 0; j < this->board_size; j++) {
            this->ship_board[i][j] = WATER;
            this->shot_board[i][j] = WATER;
        }
    }
    return;
}

void PlayerExample::delete_boards() {
    // deallocate memory using the delete operator
    for (int i = 0; i < this->board_size; i++) {
        delete[] this->ship_board[i];
        delete[] this->shot_board[i];
    }
    delete[] this->ship_board;
    delete[] this->shot_board;
    return;
}

