/**
 * @file player_example.cpp
 * @author Matthew Getgen, Luke Staritz
 * @brief The starter file for making your own AI.
 * @date 2026-05-21
 */

#include "player_example.h"


// Write your AI's name here. Please don't make it more than 64 bytes.
#define AI_NAME "Player Example C++"

// Defines the name of the bot's settings file.
#define AI_SETTINGS_FILEPATH "./ai_files/player_example.json"

// Write your name(s) or team name here. Please don't make it more than 64 bytes.
#define AUTHOR_NAMES "Example Team/Author Name"


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
    load_settings_from_file();
    example_json();
    return;
}

void PlayerExample::handle_start_game() {
    clear_boards();
    num_ships_placed = 0;
    return;
}

Ship PlayerExample::choose_ship_place(int ship_length) {
    Ship ship;

    ship.len = ship_length;
    ship.row = num_ships_placed;
    ship.col = 0;
    ship.dir = HORIZONTAL;

    num_ships_placed++;

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

void PlayerExample::load_settings_from_file() {
    std::ifstream file(AI_SETTINGS_FILEPATH);
    if (!file.is_open()) {
        std::cerr<<"Could not open JSON file! Check naming!";
        return;
    }
        file >> data;
    
}

/*
================================================================================
HOW TO RETRIEVE VALUES FROM THE LOADED JSON
================================================================================

Since the JSON structure may change dramatically between files, you should
ALWAYS check for existence and type before accessing values.

Common patterns:

1. Check if a key exists:
       if (data.contains("introText")) { ... }

2. Check type before using:
       if (data["printText"].is_boolean()) { ... }

3. Retrieve safely with default values:
       std::string s = data.value("introText", "default text");

4. Access nested objects:
       if (data.contains("subsection1") && data["subsection1"].is_object()) {
           auto& sub = data["subsection1"];
           double a = sub.value("variableA", 0.0);
           int    b = sub.value("variableB", 0);
       }

5. Iterate through unknown structures:
       for (auto& [key, value] : data.items()) {
           std::cout << key << " -> " << value << "\n";
       }

These patterns allow you to handle JSON files whose structure may change
without causing crashes.
================================================================================
*/
void PlayerExample::example_json() {
    std::cout << "\n=== Example JSON Access ===\n";

    // introText
    if (data.contains("introText") && data["introText"].is_string()) {
        std::cout << "introText: " << data["introText"].get<std::string>();
    }

    // printText
    if (data.contains("printText") && data["printText"].is_boolean()) {
        std::cout << "printText: " << (data["printText"].get<bool>() ? "true" : "false") << "\n";
    }

    // subsection1
    if (data.contains("subsection1") && data["subsection1"].is_object()) {
        const auto& sub = data["subsection1"];

        double variableA = sub.value("variableA", -1.0);
        int    variableB = sub.value("variableB", -1);

        std::cout << "subsection1.variableA: " << variableA << "\n";
        std::cout << "subsection1.variableB: " << variableB << "\n";
    }
}

