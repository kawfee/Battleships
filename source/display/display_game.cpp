/**
 * @brief display_game.cpp
 * @author Matthew Getgen
 * @brief Display Game Functionality for the Battleships Contest
 * @date 2023-09-08
 */

#include "display_game.h"

void display_game(DisplayInfo &info, GameLog &game, Board &board) {
    clear_boards(board);

    display_game_board_names(info);
    display_empty_boards(info, board.size);

    for (int i = 0; i < (int)game.player1.ships.size(); i++) {
        store_ship_board_value(board, PLAYER_1, game.player1.ships.at(i), SHIP);
        store_ship_board_value(board, PLAYER_2, game.player2.ships.at(i), SHIP);
    }

    usleep(info.delay_time);
    
    for (int i = 0; i < (int)game.player1.shots.size(); i++) {
        Shot shot1, shot2;
        shot1 = game.player1.shots.at(i);
        shot2 = game.player2.shots.at(i);
        store_shot_board_value(board, PLAYER_2, shot1);
        store_shot_board_value(board, PLAYER_1, shot2);

        if ( shot1.ship_sunk_idx != -1 ) {
            Ship ship = game.player2.ships.at(shot1.ship_sunk_idx);
            store_ship_board_value(board, PLAYER_2, ship, KILL);
            display_ship(info, ship, KILL, BOARD_2_OFFSET);
            shot1.value = KILL;
        }
        if ( shot2.ship_sunk_idx != -1 ) {
            Ship ship = game.player1.ships.at(shot2.ship_sunk_idx);
            store_ship_board_value(board, PLAYER_1, ship, KILL);
            display_ship(info, ship, KILL, BOARD_1_OFFSET);
            shot2.value = KILL;
        }

        display_shots(info, shot1, shot2, true);

        display_shot_infos(info, shot1, shot2);

        reset_cursor(info.display_row);
        usleep(info.delay_time);
        display_shots(info, shot1, shot2, false);
    }

    display_end_game_boards(info, board);
    display_game_results_and_errors(info, game);
    display_game_stats(info, game, board.size);

    reset_cursor(info.display_row);
    return;
}

void step_through_game(DisplayInfo &info, StepThroughInfo &step_info, GameLog &game, Board &board) {
    clear_boards(board);

    display_empty_boards(info, board.size);
    int ch;

    step_info.question_row = info.display_row + 2;

    if ( !step_info.is_toggled ) {
        cout << conio::gotoRowCol(step_info.question_row, 1)
             << "Please press ENTER to start: ";
        ch = (int)getchar();
        if ( ch != ENTER ) {
            step_info.quit = true;
            return;
        };

        step_info.toggle.off();
        step_info.is_toggled = true;
    }

    store_step_through_state(info, step_info, game, board);
    display_end_game_boards(info, board);

    do {
        cout << conio::gotoRowCol(step_info.question_row, 1) << conio::clearRow();
        cout << conio::gotoRowCol(step_info.question_row, 1)
             << "Please press ⯇ ⯈ ⯅ ⯆, WASD, or HJKL (ENTER to stop): ";

        ch = getchar();
        cout << "       ";

        switch (ch) {
        case UP:
        case W:
        case K:
            if ( process_key(step_info, UP) ) return;
            break;
        case DOWN:
        case S:
        case J:
            if ( process_key(step_info, DOWN) ) return;
            break;
        case RIGHT:
        case D:
        case L:
            if ( process_key(step_info, RIGHT) ) {
                store_step_through_state(info, step_info, game, board);
                display_end_game_boards(info, board);
            }
            break;
        case LEFT:
        case A:
        case H:
            if ( process_key(step_info, LEFT) ) {
                store_step_through_state(info, step_info, game, board);
                display_end_game_boards(info, board);
            }
            break;
        case ENTER:
            step_info.toggle.on();
            step_info.is_toggled = false;
            step_info.quit = true;
            break;
        default:
            break;
        }
    } while ( !step_info.quit );
    return;
}

bool process_key(StepThroughInfo &step_info, StepThroughKey key) {
    calculate_step_through_state(step_info);

    switch (key) {
    case UP:
    case W:
    case K:
        // go to previous game
        if ( step_info.game_step > 0 ) {
            step_info.game_step--;
            return true;
        }
        break;
    case DOWN:
    case S:
    case J:
        // go to next game
        if ( step_info.game_step < (step_info.max_games-1) ) {
            step_info.game_step++;
            return true;
        }
        break;
    case RIGHT:
    case D:
    case L:
        // go to next action
        if ( !step_info.state.full_ships ) {
            step_info.ship_step++;
            return true;
        } else if ( step_info.state.full_ships && !step_info.state.full_shots ) {
            step_info.shot_step++;
            return true;
        }
        break;
    case LEFT:
    case A:
    case H:
        // go to previous action
        if ( step_info.state.full_ships && !step_info.state.no_shots ) {
            step_info.shot_step--;
            return true;
        } else if ( !step_info.state.no_ships ) {
            step_info.ship_step--;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void store_step_through_state(DisplayInfo &info, StepThroughInfo &step_info, GameLog &game, Board &board) {
    clear_boards(board);
    
    Ship ship1, ship2;
    Shot shot1, shot2;

    for (int i = 0; i < step_info.ship_step; i++) {
        ship1 = game.player1.ships.at(i);
        ship2 = game.player2.ships.at(i);
        store_ship_board_value(board, PLAYER_1, ship1, SHIP);
        store_ship_board_value(board, PLAYER_2, ship2, SHIP);
    }

    for (int i = 0; i < step_info.shot_step; i++) {
        shot1 = game.player1.shots.at(i);
        shot2 = game.player2.shots.at(i);
        store_shot_board_value(board, PLAYER_2, shot1);
        store_shot_board_value(board, PLAYER_1, shot2);

        if ( shot1.ship_sunk_idx != -1 ) {
            Ship ship = game.player2.ships.at(shot1.ship_sunk_idx);
            store_ship_board_value(board, PLAYER_2, ship, KILL);
            shot1.value = KILL;
        }
        if ( shot2.ship_sunk_idx != -1 ) {
            Ship ship = game.player1.ships.at(shot2.ship_sunk_idx);
            store_ship_board_value(board, PLAYER_1, ship, KILL);
            shot2.value = KILL;
        }
    }

    calculate_step_through_state(step_info);
    
    if ( step_info.state.no_ships && step_info.state.no_shots ) {
        // clear row for ship/shot info.
        cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
    } else if ( !step_info.state.no_ships && step_info.state.no_shots ) {
        display_ship_infos(info, ship1, ship2);
    } else if ( step_info.state.full_ships && !step_info.state.no_shots ) {
        display_shot_infos(info, shot1, shot2);
    }
    if ( step_info.state.full_ships && step_info.state.full_shots ) {
        display_game_results_and_errors(info, game);
        info.display_row -= 2;
        if ( game.player1.error.type != OK || game.player2.error.type != OK ) {
            info.display_row--;
        }
    } else {
        cout << conio::gotoRowCol(info.display_row+1, 1) << conio::clearRow();
    }
    return;
}

void calculate_step_through_state(StepThroughInfo &step_info) {
    step_info.state.full_ships = (step_info.ship_step == step_info.max_ships);
    step_info.state.some_ships = (
        step_info.ship_step < step_info.max_ships &&
        step_info.ship_step > 0
    );
    step_info.state.no_ships = (step_info.ship_step == 0);

    step_info.state.full_shots = (step_info.shot_step == step_info.max_shots);
    step_info.state.some_shots = (
        step_info.shot_step < step_info.max_shots &&
        step_info.shot_step > 0
    );
    step_info.state.no_shots = (step_info.shot_step == 0);
    return;
}

void display_game_number(DisplayInfo &info, int game_num) {
    if ( game_num < 0 ) return;
    cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow()
         << "Game #" << game_num + 1;
    info.display_row += 2;
    return;
}

void display_game_board_names(DisplayInfo &info) {
    cout << conio::gotoRowCol(info.display_row, BOARD_1_OFFSET)
         << info.player1.ai_name << "'s Board:"
         << conio::gotoRowCol(info.display_row, BOARD_2_OFFSET) 
         << info.player2.ai_name << "'s Board:";
    info.display_row += 2;
    return;
}

void display_empty_boards(DisplayInfo &info, int board_size) {
    string column_numbers = " " + vertical;
    string column_line = horizontal + intersection;
    string water_line = "";

    for (int i = 0; i < board_size; i++) {
        // will look like ` │0123456789`
        column_numbers.append(to_string(i));
        // will look like `─┼──────────`
        column_line.append(horizontal);
        // will look like `~~~~~~~~~~`
        water_line.push_back(WATER);
    }

    cout << conio::gotoRowCol(info.display_row, BOARD_1_OFFSET) << column_numbers;
    cout << conio::gotoRowCol(info.display_row, BOARD_2_OFFSET) << column_numbers;
    info.display_row++;
    cout << conio::gotoRowCol(info.display_row, BOARD_1_OFFSET) << column_line;
    cout << conio::gotoRowCol(info.display_row, BOARD_2_OFFSET) << column_line;
    info.display_row++;

    info.board_row = info.display_row;
    for (int i = 0; i < board_size; i++) {
        cout << conio::gotoRowCol(info.display_row, BOARD_1_OFFSET)
             << i << vertical
             << conio::bgColor(conio::LIGHT_CYAN) << conio::fgColor(conio::BLACK) 
             << water_line 
             << conio::resetAll();
        
        cout << conio::gotoRowCol(info.display_row, BOARD_2_OFFSET)
             << i << vertical
             << conio::bgColor(conio::LIGHT_CYAN) << conio::fgColor(conio::BLACK) 
             << water_line 
             << conio::resetAll();
        
        info.display_row++;
    }
    info.display_row++;

   reset_cursor(info.display_row);
    return;
}

void display_end_game_boards(DisplayInfo &info, Board &board) {
    Shot shot1, shot2;
    for (int i = 0; i < board.size; i++) {
        shot1.row = i;
        shot2.row = i;
        for (int j = 0; j < board.size; j++) {
            shot1.col = j;
            shot2.col = j;
            shot1.value = (BoardValue)board.board1[i][j];
            shot2.value = (BoardValue)board.board2[i][j];
            display_shots(info, shot2, shot1, false);
        }
    }
    return;
}

void display_ship(DisplayInfo &info, Ship &ship, BoardValue value, int col_offset) {
    Shot shot;
    shot.value = value;
    for (int i = 0; i < ship.len; i++) {
        if ( ship.dir == HORIZONTAL ) {
            shot.row = ship.row;
            shot.col = ship.col + i;
        } else {
            shot.row = ship.row + i;
            shot.col = ship.col;
        }
        display_shot(info, shot, col_offset, false);
    }
    return;
}

void display_ship_infos(DisplayInfo &info, Ship &ship1, Ship &ship2) {
    cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
    display_ship_info(info, ship1, BOARD_1_OFFSET);
    display_ship_info(info, ship2, BOARD_2_OFFSET);
    return;
}

void display_ship_info(DisplayInfo &info, Ship &ship, int col_offset) {
    cout << conio::gotoRowCol(info.display_row, col_offset);

    switch (ship.dir) {
    case HORIZONTAL:
        cout << "HORIZONTAL";
        break;
    case VERTICAL:
        cout << "VERTICAL";
        break;
    }
    // printf is just superior when it comes to this
    printf(" @ [%d, %d] x %d", ship.row, ship.col, ship.len);
    return;
}

void display_shots(DisplayInfo &info, Shot &shot1, Shot &shot2, bool highlight) {
    display_shot(info, shot2, BOARD_1_OFFSET, highlight);
    display_shot(info, shot1, BOARD_2_OFFSET, highlight);
    return;
}

void display_shot(DisplayInfo &info, Shot &shot, int col_offset, bool highlight) {
    int row = shot.row + info.board_row;
    int col = shot.col + col_offset + LEFT_COL_OFFSET;

    conio::Color background = conio::LIGHT_CYAN, foreground = conio::BLACK;
    char value;
    switch (shot.value) {
    case WATER:
        background = conio::LIGHT_CYAN;
        foreground = conio::BLACK;
        break;
    case SHIP:
        background = conio::WHITE;
        foreground = conio::BLACK;
        break;
    case HIT:
    case DUPLICATE_HIT:
        background = conio::LIGHT_YELLOW;
        foreground = conio::BLACK;
        break;
    case MISS:
    case DUPLICATE_MISS:
        background = conio::GRAY;
        foreground = conio::BLACK;
        break;
    case KILL:
    case DUPLICATE_KILL:
        background = conio::LIGHT_RED;
        foreground = conio::WHITE;
        break;
    }
    switch (shot.value) {
    case DUPLICATE_HIT:
    case DUPLICATE_MISS:
    case DUPLICATE_KILL:
        value = '!';
        break;
    default:
        value = shot.value;
        break;
    }
    if ( highlight ) {
        foreground = background;
        background = conio::BLACK;
    }
    cout << conio::gotoRowCol(row, col)
         << conio::bgColor(background) << conio::fgColor(foreground)
         << value
         << conio::bgColor(conio::RESET) << conio::fgColor(conio::RESET) << flush;
    return;
}

void display_shot_infos(DisplayInfo &info, Shot &shot1, Shot &shot2) {
    cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
    display_shot_info(info, shot2, BOARD_1_OFFSET);
    display_shot_info(info, shot1, BOARD_2_OFFSET);
    return;
}

void display_shot_info(DisplayInfo &info, Shot &shot, int col_offset) {
    cout << conio::gotoRowCol(info.display_row, col_offset);

    switch (shot.value) {
    case HIT:
        cout << conio::fgColor(conio::LIGHT_YELLOW)
             << "HIT"
             << conio::fgColor(conio::RESET);
        break;
    case MISS:
        cout << conio::fgColor(conio::GRAY)
             << "MISS"
             << conio::fgColor(conio::RESET);
        break;
    case KILL:
        cout << conio::fgColor(conio::LIGHT_RED)
             << "KILL"
             << conio::fgColor(conio::RESET);
        break;
    case DUPLICATE_HIT:
    case DUPLICATE_MISS:
    case DUPLICATE_KILL:
        cout << conio::bgColor(conio::WHITE) << conio::fgColor(conio::RED)
             << "DUPLICATE"
             << conio::resetAll();
        break;
    default:
        break;
    }
    // printf is just superior when it comes to this
    printf(" @ [%d, %d]", shot.row, shot.col);
    return;
}

void display_game_results_and_errors(DisplayInfo &info, GameLog &game) {
    cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
    display_game_result(info, game.player1.stats.result, PLAYER_1);
    display_game_result(info, game.player2.stats.result, PLAYER_2);

    if ( game.player1.error.type != OK || game.player2.error.type != OK ) {
        info.display_row++;
        cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
        display_game_error(info, game.player1.error.type, BOARD_1_OFFSET);
        display_game_error(info, game.player2.error.type, BOARD_2_OFFSET);
    }

    info.display_row += 2;
    return;
}

void display_game_result(DisplayInfo &info, GameResult result, PlayerNum player) {
    int col_offset = BOARD_1_OFFSET;
    string name = info.player1.ai_name;
    if ( player == PLAYER_2 ) {
        col_offset = BOARD_2_OFFSET;
        name = info.player2.ai_name;
    }
    cout << conio::gotoRowCol(info.display_row, col_offset) << print_name_by_result(name, result);

	switch (result) {
    case WIN:
        cout << " won!";
        break;
    case LOSS:
        cout << " lost.";
        break;
    case TIE:
        cout << " tied.";
        break;
	}
    return;
}

void display_game_error(DisplayInfo &info, ErrorType &error_type, int col_offset) {
    string error_msg = "";
    switch (error_type) {
    case ErrFork:
        error_msg = PLAYER_FORK_ERR;
        break;
    case ErrConnect:
        error_msg = SOCKET_CONNECT_ERR;
        break;
    case ErrSend:
        error_msg = SEND_MESSAGE_ERR;
        break;
    case ErrReceive:
        error_msg = RECV_MESSAGE_ERR;
        break;
    case ErrHelloMessage:
        error_msg = HELLO_MESSAGE_ERR;
        break;
    case ErrShipPlacedMessage:
        error_msg = SHIP_MESSAGE_ERR;
        break;
    case ErrShotTakenMessage:
        error_msg = SHOT_MESSAGE_ERR;
        break;
    case ErrShipLength:
    case ErrShipOffBoard:
    case ErrShipIntersect:
        error_msg = SHIP_PLACE_ERR;
        break;
    case ErrShotOffBoard:
        error_msg = SHOT_PLACE_ERR;
        break;
    case OK:
        break;
    }
    if ( error_type != OK ) {
        cout << conio::gotoRowCol(info.display_row, col_offset)
             << conio::setTextStyle(conio::BOLD) << conio::bgColor(conio::WHITE) << conio::fgColor(conio::RED)
             << "Error: " << error_msg << " (code: " << error_type << ")" << conio::resetAll();
    }
    return;
}

void display_game_stats(DisplayInfo &info, GameLog &game, int board_size) {
    const string
        GAME_STATS      = "Game Stats",
        PERCENT_SHOT    = "% Board Shot",
        NUM_KILLED      = "# Ships Killed",
        NUM_HITS        = "# Hits",
        NUM_MISSES      = "# Misses",
        NUM_DUPLICATES  = "# Duplicates";

    int percent1, percent2, size1, size2, col_width = 14;
    percent1 = calculate_percent_board_shot(game.player1.stats.num_board_shot, board_size);
    percent2 = calculate_percent_board_shot(game.player2.stats.num_board_shot, board_size);

    size1 = info.player1.ai_name.size();
    size2 = info.player2.ai_name.size();
    if ( size1 < 5 ) size1 = 5;
    if ( size2 < 5 ) size1 = 5;
    ostringstream name1, name2;
    name1 << setfill(' ') << setw(size1) << right << info.player1.ai_name;
    name2 << setfill(' ') << setw(size2) << right << info.player2.ai_name;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << GAME_STATS << " " << vertical << " "
         << print_name_by_result(name1.str(), game.player1.stats.result) << " " << vertical << " "
         << print_name_by_result(name2.str(), game.player2.stats.result) << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1)
         << multiply_string(horizontal, col_width+2) << intersection
         << multiply_string(horizontal, size1+2) << intersection
         << multiply_string(horizontal, size2+2) << end_horizontal;
    info.display_row++;
    
    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << PERCENT_SHOT << " " << vertical << " "
         << setfill(' ') << setw(size1-1) << right << percent1 << "% " << vertical << " "
         << setfill(' ') << setw(size2-1) << right << percent2 << "% " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << NUM_KILLED << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << game.player1.stats.ships_killed << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << game.player2.stats.ships_killed << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << NUM_HITS << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << game.player1.stats.hits << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << game.player2.stats.hits << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << NUM_MISSES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << game.player1.stats.misses << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << game.player2.stats.misses << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << NUM_DUPLICATES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << game.player1.stats.duplicates << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << game.player2.stats.duplicates << " " << vertical;
    info.display_row += 2;
    return;
}

string print_name_by_result(string name, GameResult result) {
    ostringstream strm;
    switch (result) {
    case WIN:
        strm << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::GREEN) << name << conio::resetAll();
        break;
    case LOSS:
    case TIE:
        strm << conio::setTextStyle(conio::BOLD) << name << conio::setTextStyle(conio::NORMAL_INTENSITY);
        break;
    }
    return strm.str();
}

string multiply_string(string s, int num) {
    string str;
    for (int i = 0; i < num; i++) {
        str += s;
    }
    return str;
}

void reset_cursor(int display_row) {
    cout << conio::gotoRowCol(display_row+2, 1) << conio::resetAll() << "" << flush;
    return;
}

int calculate_percent_board_shot(int num_board_shot, int board_size) {
    float size, percent;

    if ( num_board_shot <= 0 ) return 0;

    size = (float)(board_size * board_size);
    if ( size <= 0 ) return 0;

    percent = (float)(num_board_shot / size);
    percent *= 100.0;
    int clean = (int)percent;
    if ( clean >= 100 ) return 100;
    else if ( clean <= 0 ) return 0;

    return clean;
}

