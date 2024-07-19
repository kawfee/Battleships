/**
 * @file display_match.cpp
 * @author Matthew Getgen
 * @brief Display Match functionality for Battleships
 * @date 2023-09-08
 */

#include "display_match.h"

void display_match_with_options(MatchLog &match, MatchOptions &options, int row) {
        DisplayInfo info;
        info.display_row = row;
        info.player1.ai_name = clean_name(match.player1.ai_name);
        info.player2.ai_name = clean_name(match.player2.ai_name);
        info.player1.author_name = clean_name(match.player1.author_name);
        info.player2.author_name = clean_name(match.player2.author_name);
        info.type = options.display_type;
        info.delay_time = options.delay_time;
        info.step_through = options.step_through;

        Board board = create_boards(match.board_size);
        display_match(info, match, board);
        delete_boards(board);
}

void display_match(DisplayInfo &info, MatchLog &match, Board &board) {
    int num_games = match.games.size(), increment, choice = 0;
    vector<int> game_list;

    if (num_games == 0) info.type = NONE;
    int win = -1, loss = -1, tie = -1, err = -1;

    switch (info.type) {
    case LAST:
        game_list.push_back(num_games-1);
        handle_game_list(info, match, board, game_list);
        break;
    case ALL:
        for (int i = 0; i < num_games; i++) game_list.push_back(i);
        handle_game_list(info, match, board, game_list);
        break;
    case EACH_TYPE:
        for (int i = 0; i < num_games; i++) {
            GameResult result = match.games.at(i).player1.stats.result;
            ErrorType err1 = match.games.at(i).player1.error.type;
            ErrorType err2 = match.games.at(i).player2.error.type;
            switch (result) {
            case WIN:
                win = i;
                break;
            case LOSS:
                loss = i;
                break;
            case TIE:
                tie = i;
                break;
            default:
                break;
            }
            if (err1 != OK || err2 != OK) err = i;
        }
        if ( win != -1 ) game_list.push_back(win);
        if ( loss != -1 ) game_list.push_back(loss);
        if ( tie != -1 ) game_list.push_back(tie);
        if ( err != -1 ) game_list.push_back(err);

        sort(game_list.begin(), game_list.end());
        game_list.erase(unique(game_list.begin(), game_list.end()), game_list.end());

        handle_game_list(info, match, board, game_list);
        break;
    case INCREMENT:
        if (num_games == 1) {
            game_list.push_back(0);
        } else {
            increment = ask_display_game_increment(info.display_row, num_games);
            if ( increment != -1 ) {
                for (int i = (int)match.games.size()-1; i >= 0; i -= increment) {
                    game_list.push_back(i);
                }
                sort(game_list.begin(), game_list.end());
                game_list.erase(unique(game_list.begin(), game_list.end()), game_list.end());
            }
        }
        handle_game_list(info, match, board, game_list);
        break;
    case CHOICE:
        do {
            game_list.clear();
            choice = ask_display_game_choice(info.display_row, 1, num_games);
            if ( choice == -1 ) break;
            choice--;
            game_list.push_back(choice);

            handle_game_list(info, match, board, game_list);
        } while ( choice != -1 );
        break;
    case NONE:
    default:
        reset_screen(info);
        display_match_vs(info);
    }
    

    display_match_result(info, match);
    display_match_errors(info, match);
    display_match_stats(info, match);
    display_elapsed_time(info, match.elapsed_time);

    reset_cursor(info.display_row);
    return;
}

void handle_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list) {
    if ( info.step_through ) {
        step_through_game_list(info, match, board, game_list);
    } else {
        display_game_list(info, match, board, game_list);
    }
    return;
}

void display_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list) {
    for (int i = 0; i < (int)game_list.size(); i++) {
        reset_screen(info);
        display_match_vs(info);
        display_game_number(info, game_list.at(i));
        display_game(info, match.games.at(game_list.at(i)), board);

        reset_cursor(info.display_row);
        if ( i < (int)game_list.size()-1 ) sleep(SLEEP_TIME);
    }
    return;
}

void step_through_game_list(DisplayInfo &info, MatchLog &match, Board &board, vector<int> &game_list) {
    StepThroughInfo step_info;
    step_info.max_games = (int)game_list.size();
    step_info.max_ships = 0;
    step_info.max_shots = 0;
    step_info.game_step = 0;
    step_info.ship_step = 0;
    step_info.shot_step = 0;
    step_info.quit = false;
    step_info.is_toggled = false;

    reset_screen(info);
    display_match_vs(info);
    step_info.board_row = info.display_row;

    while ( !step_info.quit && step_info.max_games != 0 ) {
        int game = game_list.at(step_info.game_step);
        step_info.max_ships = (int)match.games.at(game).player1.ships.size();
        step_info.max_shots = (int)match.games.at(game).player1.shots.size();
        if ( step_info.ship_step > step_info.max_ships ) step_info.ship_step = step_info.max_ships;
        if ( step_info.shot_step > step_info.max_shots ) step_info.shot_step = step_info.max_shots;

        info.display_row = step_info.board_row;
        display_game_number(info, game);
        display_game_board_names(info);
        step_through_game(info, step_info, match.games.at(game), board);
        cout << conio::gotoRowCol(info.display_row, 1) << conio::clearRow();
        cout << conio::gotoRowCol(step_info.question_row, 1) << conio::clearRow();

        if ( step_info.quit ) {
            display_game_results_and_errors(info, match.games.at(game));
            display_game_stats(info, match.games.at(game), board.size);
        }
    }
    return;
}

void display_match_vs(DisplayInfo &info) {
    cout << conio::gotoRowCol(info.display_row, 1)
         << print_author_and_ai(info.player1) << endl
         << "\t──── VS ────" << endl
         << print_author_and_ai(info.player2);
    info.display_row += 4;
    return;
}

void display_match_result(DisplayInfo &info, MatchLog &match) {
    int p1_wins = match.player1.stats.wins,
        p2_wins = match.player2.stats.wins;
    cout << conio::gotoRowCol(info.display_row, 1) << conio::setTextStyle(conio::ITALIC);
    if ( p1_wins > p2_wins ) {
        cout << print_author_and_ai(info.player1)
             << " won the match! (player 1)";
    } else if ( p1_wins < p2_wins ) {
        cout << print_author_and_ai(info.player2)
             << " won the match! (player 2)";
    } else if ( p1_wins == p2_wins ) {
        cout << "The match ended in a tie!";
    }
    cout << conio::resetAll();
    info.display_row += 2;
    return;
}

string print_author_and_ai(Name &name) {
    ostringstream strm;
    if (name.author_name != "") {
        strm << name.author_name << "'s ";
    }
    strm << conio::setTextStyle(conio::BOLD)
         << name.ai_name
         << conio::setTextStyle(conio::NORMAL_INTENSITY);
    return strm.str();
}

void display_match_errors(DisplayInfo &info, MatchLog &match) {
    display_match_error(info, match.player1.error, PLAYER_1);
    display_match_error(info, match.player2.error, PLAYER_2);
    return;
}

void display_match_error(DisplayInfo &info, Error &error, PlayerNum player) {
    cout << conio::gotoRowCol(info.display_row, 1) << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::RED);
    if ( error.type != OK ) {
        if ( player == PLAYER_1 ) cout << info.player1.ai_name;
        else                      cout << info.player2.ai_name;
        cout << " Error Info: (code: " << error.type << ")" << conio::fgColor(conio::RESET);
        info.display_row++;
    }

    cout << conio::gotoRowCol(info.display_row, 1);
    switch ( error.type ) {
    case ErrFork:
        cout << "Couldn't execute the AI." << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure the AI file is an executable, and has execute permissions."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 3;
        break;
    case ErrConnect:
        cout << "Couldn't connect to the AI." << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure the AI is not exiting early, and that it's connecting to the socket file."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 3;
        break;
    case ErrSend:
        cout << "Message not sent to AI. This is a very odd error, and I haven't experienced it in the wild." << endl
             << "My guess is this can happen if an AI exits and closes their socket connection before the server sends a message.";
        info.display_row += 3;
        break;
    case ErrReceive:
        cout << "No message received from AI. This can be the result of:" << endl
             << " * Exiting early." << endl
             << "   - Can be caused by a segfault." << endl
             << " * Not responding to the controller on time (default timeout is 0.5s)." << endl
             << "   - Can be caused by an infinite loop, recursion without a base case, or a slow algorithm." << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure you debug your AI if this occurs."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 7;
        break;
    case ErrHelloMessage:
        cout << "Invalid hello message:" << endl
             << "Message received: " << error.message << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI is returning a valid JSON message with valid key/value pairs."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 4;
        break;
    case ErrShipPlacedMessage:
        cout << "Invalid ship_placed message:" << endl
             << "Message received: " << error.message << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI is returning a valid JSON message with valid key/value pairs."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 4;
        break;
    case ErrShotTakenMessage:
        cout << "Invalid shot_taken message:" << endl
             << "Message received: " << error.message << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI is returning a valid JSON message with valid key/value pairs."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 4;
        break;
    case ErrShipLength:
        cout << "Ship returned has the wrong length:" << endl
             << "\tlength received: " << error.ship.len << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI places a ship of the same length requested."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 4;
        break;
    case ErrShipOffBoard:
        cout << "Ship returned is off the board:" << endl
             << "\tlength: " << error.ship.len << endl
             << "\trow: " << error.ship.row << endl
             << "\tcol: " << error.ship.col << endl
             << "\tdirection: ";
        if ( error.ship.dir == HORIZONTAL ) cout << "HORIZONTAL" << endl;
        else                                cout << "VERTICAL" << endl;
        cout << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI places ships onto the board."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 7;
        break;
    case ErrShipIntersect:
        cout << "Ship returned intersects with another ship:" << endl
             << "\tlength: " << error.ship.len << endl
             << "\trow: " << error.ship.row << endl
             << "\tcol: " << error.ship.col << endl
             << "\tdirection: ";
        if ( error.ship.dir == HORIZONTAL ) cout << "HORIZONTAL" << endl;
        else                                cout << "VERTICAL" << endl;
        cout << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI doesn't place ships on top of each other."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 7;
        break;
    case ErrShotOffBoard:
        cout << "Shot returned is off the board:" << endl
             << "\trow: " << error.shot.row << endl
             << "\tcol: " << error.shot.col << endl
             << conio::setTextStyle(conio::ITALIC)
             << "Make sure your AI shoots onto the board."
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 5;
        break;
    case OK:
        break;
    }
    cout << conio::resetAll();
    return;
}

void display_match_stats(DisplayInfo &info, MatchLog &match) {
    const string
        MATCH_STATS             = "Match Stats",
        WINS                    = "Wins",
        LOSSES                  = "Losses",
        TIES                    = "Ties",
        AVG_PERCENT_SHOT        = "Avg % Board Shot",
        TOTAL_NUM_KILLED        = "Total # Ships Killed",
        TOTAL_NUM_HITS          = "Total # Hits",
        TOTAL_NUM_MISSES        = "Total # Misses",
        TOTAL_NUM_DUPLICATES    = "Total # Duplicates";
    
    int percent1, percent2, size1, size2, col_width = 20;
    percent1 = calculate_avg_percent_board_hit(match.player1.stats.total_num_board_shot,
        match.board_size, match.games.size());
    percent2 = calculate_avg_percent_board_hit(match.player2.stats.total_num_board_shot,
        match.board_size, match.games.size());

    size1 = info.player1.ai_name.size();
    size2 = info.player2.ai_name.size();
    if ( size1 < 5 ) size1 = 5;
    if ( size2 < 5 ) size1 = 5;

    GameResult result1, result2;
    if (match.player1.stats.wins > match.player1.stats.losses) {
        result1 = WIN;
        result2 = LOSS;
    } else if (match.player1.stats.wins < match.player1.stats.losses) {
        result1 = LOSS;
        result2 = WIN;
    } else {
        result1 = TIE;
        result2 = TIE;
    }

    ostringstream name1, name2;
    name1 << setfill(' ') << setw(size1) << right << info.player1.ai_name;
    name2 << setfill(' ') << setw(size2) << right << info.player2.ai_name;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << MATCH_STATS << " " << vertical << " "
         << print_name_by_result(name1.str(), result1) << " " << vertical << " "
         << print_name_by_result(name2.str(), result2) << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1)
         << multiply_string(horizontal, col_width+2) << intersection
         << multiply_string(horizontal, size1+2) << intersection
         << multiply_string(horizontal, size2+2) << end_horizontal;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << WINS << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.wins << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.wins << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << LOSSES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.losses << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.losses << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << TIES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.ties << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.ties << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << AVG_PERCENT_SHOT << " " << vertical << " "
         << setfill(' ') << setw(size1-1) << right << percent1 << "% " << vertical << " "
         << setfill(' ') << setw(size2-1) << right << percent2 << "% " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << TOTAL_NUM_KILLED << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.total_ships_killed << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.total_ships_killed << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << TOTAL_NUM_HITS << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.total_hits << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.total_hits << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << TOTAL_NUM_MISSES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.total_misses << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.total_misses << " " << vertical;
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(col_width) << left << TOTAL_NUM_DUPLICATES << " " << vertical << " "
         << setfill(' ') << setw(size1) << right << match.player1.stats.total_duplicates << " " << vertical << " "
         << setfill(' ') << setw(size2) << right << match.player2.stats.total_duplicates << " " << vertical;
    info.display_row += 2;

    return;
}

void display_elapsed_time(DisplayInfo &info, float elapsed_time) {
    cout << conio::gotoRowCol(info.display_row, 1)
         << "Elapsed time: " << elapsed_time;
    if ( elapsed_time == 1.0 ) {
        cout << " second";
    } else {
        cout << " seconds";
    }
    info.display_row += 2;
    return;
}

void reset_screen(DisplayInfo &info) {
    info.display_row = 1;
    cout << conio::clearScreen() << conio::gotoRowCol(info.display_row, 1);
    return;
}

string clean_name(string name) {
    if ( name.length() <= 28 ) {
        return name;
    }
    return name.substr(0, 25) + "...";
}

int calculate_avg_percent_board_hit(int total_num_board_shot, int board_size, int num_games) {
    float size, avg, percent;

    if ( total_num_board_shot <= 0 || num_games <= 0 ) return 0;

    size = (float)(board_size * board_size);
    if ( size <= 0 ) return 0;

    avg = (float)total_num_board_shot / num_games;

    percent = (float)(avg / size);
    percent *= 100.0;
    int clean = (int)percent;
    if ( clean >= 100 ) return 100;
    else if ( clean <= 0 ) return 0;
    
    return clean;
}
