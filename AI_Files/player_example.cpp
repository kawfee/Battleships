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


/*================================================================================
 * Starts up the entire match. Do NOT change anything here unless you really understand
 * what you are doing.
 *================================================================================*/
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

/*================================================================================
 * This is like a constructor for the entire match.
 * You probably don't want to make changes here unless it is something that is done once at the beginning 
 * of the entire match..
 *================================================================================*/
void PlayerExample::handle_setup_match(PlayerNum player, int board_size) {
    this->player = player;
    this->board_size = board_size;
    create_boards();
    return;
}

/*================================================================================
 * This is like a constructor for one game/round within the entire match.
 * Add anything here that you need to do at the beginning of each game.
 *================================================================================*/
void PlayerExample::handle_start_game() {
    clear_boards();
    num_ships_placed = 0;
    return;
}

/*================================================================================
 * Example of how to decide where to place a ship of length ship_length and
 * inform the contest controller of your decision.
 *
 * If you place your ship even a bit off the board or collide with a previous ship
 * that you placed in this round, you instantly forfeit the round.
 *
 * TLDR: set ship.len, ship.row, and ship.col to good values and return ship.
 *================================================================================*/
Ship PlayerExample::choose_ship_place(int ship_length) {
    Ship ship;

    ship.len = ship_length;
    ship.row = num_ships_placed;
    ship.col = 0;
    ship.dir = HORIZONTAL;

    num_ships_placed++;

    return ship;
}

/*================================================================================
 * Example of how to decide where to shoot and inform the contest controller
 * of your decision.
 *================================================================================*/
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

/*================================================================================
 * This function is called to inform your AI of the result of a previous shot,
 * as well as where the opponent has shot.
 *================================================================================*/
void PlayerExample::handle_shot_return(PlayerNum player, Shot &shot) {
    // Results of your AI's shot was returned, store it
    if ( player == this->player ) {
        this->shot_board[shot.row][shot.col] = shot.value;
    }

    // Your AI is informed of where the opponent AI shot, store it
    // NOTE: Opponent shots are stored in ship_board, not shot_board
    else {
        this->ship_board[shot.row][shot.col] = shot.value;
    }
    return;
}

/*================================================================================
 * This function is called to update your shot_board (results of your shots at
 * opponent) when an opponent ship has been killed, OR to update your ship_board
 * (where you keep track of your ships) to show that your ship was killed.
 *================================================================================*/
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

/*================================================================================
 * This function runs at the end of a particular game/round.
 * Do anything here that needs to be done at the end of a game/round in the match.
 *================================================================================*/
void PlayerExample::handle_game_over() {
    return;
}

/*================================================================================
 * This function is called by the AI's destructor and runs at the end of the entire match.
 *================================================================================*/
void PlayerExample::handle_match_over() {
    delete_boards();
    return;
}

/*================================================================================
 * This function sets up all boards at the beginning of the whole match.
 * Add setup here for any boards you create.
 *================================================================================*/
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

/*================================================================================
 * This function resets boards between rounds.
 *================================================================================*/
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

/*================================================================================
 * This function is called by the AI's destructor and runs at the end of the entire match.
 *================================================================================*/
void PlayerExample::delete_boards() {
    // deallocates memory using the delete operator

    for (int i = 0; i < this->board_size; i++) {
        delete[] this->ship_board[i];
        delete[] this->shot_board[i];
    }
    delete[] this->ship_board;
    delete[] this->shot_board;
    return;
}

