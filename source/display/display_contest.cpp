/**
 * @file display_contest.cpp
 * @author Matthew Getgen
 * @brief Display Contest functionality for Battleships
 * @date 2023-09-08
 */

 #include "display_contest.h"


/* ───────────────────────── *
 * DISPLAY CONTEST FUNCTIONS *
 * ───────────────────────── */

void display_contest_with_options(ContestLog &contest, ContestOptions &options) {
    DisplayInfo info;
    info.display_row = 1;
    info.type = LAST;
    info.contest_type = options.display_type;
    info.delay_time = options.delay_time;
    info.step_through = false;

    Board board = create_boards(contest.board_size);

    display_contest(info, contest, board);

    delete_boards(board);
    return;
}

void display_contest(DisplayInfo &info, ContestLog &contest, Board &board) {
    for (int i = 0; i < (int)contest.players.size(); i++) {
        ContestPlayer &player = contest.players.at(i);
        player.ai_name = clean_name(player.ai_name);
        player.author_name = clean_name(player.author_name);
    }
    switch (info.contest_type) {
    case NORMAL:
    case ROUNDS:
        display_contest_rounds(info, contest, board);
        display_contest_leaderboard(info, contest);
        break;
    case FINAL:
        display_contest_leaderboard(info, contest);
    }
    return;
}

void display_contest_leaderboard(DisplayInfo &info, ContestLog &contest) {
    vector<ContestPlayer> sorted_players;
    const string
        RANK   = "Rank",
        NAME   = "Name",
        WINS   = "Wins",
        LOSSES = "Losses",
        TIES   = "Ties";
    
    int rank_width = (int)RANK.size(),
        name_width = (int)NAME.size(),
        num_width  = 6;

    for (int i = 0; i < (int)contest.players.size(); i++) {
        ContestPlayer &player = contest.players.at(i);
        sorted_players.push_back(player);
    }
    sort(sorted_players.begin(), sorted_players.end(), sort_players_by_rank);

    // get max name width
    for (int i = 0; i < (int)sorted_players.size(); i++) {
        ContestPlayer &player = sorted_players.at(i);
        if ( (int)player.ai_name.size() > name_width ) {
            name_width = player.ai_name.size();
        }
    }

    reset_screen(info);
    cout << conio::gotoRowCol(info.display_row, 1)
            << conio::setTextStyle(conio::BOLD)
            << "Final Leaderboard"
            << conio::setTextStyle(conio::NORMAL_INTENSITY);
    info.display_row += 2;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(rank_width) << right << RANK << " " << vertical << " "
         << setfill(' ') << setw(name_width) << left << NAME << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << WINS << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << LOSSES << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << TIES << " " << vertical;
    info.display_row++; 

    cout << conio::gotoRowCol(info.display_row, 1)
         << multiply_string(horizontal, rank_width+2) << intersection
         << multiply_string(horizontal, name_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << end_horizontal;
    info.display_row++;

    for (int i = 0; i < (int)sorted_players.size(); i++) {
        ContestPlayer &player = sorted_players.at(i);
        cout << conio::gotoRowCol(info.display_row, 1) << " "
            << setfill(' ') << setw(rank_width) << right << i+1 << " " << vertical << " "
            << print_name_by_final_status(player.ai_name, name_width, player.lives) << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.wins << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.losses << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.ties << " " << vertical;
        info.display_row++; 
    }
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1)
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::RESET) << " *" << conio::resetAll()
         << " - Player lost" << endl
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::GREEN) << " *" << conio::resetAll()
         << " - Player won!" << endl;
    info.display_row += 2;
    reset_cursor(info.display_row);

    return;
}

string print_name_by_final_status(string name, int width, int lives) {
    ostringstream strm;
    conio::Color color = conio::RESET;
    switch (lives) {
    case 0:
        break;
    default:
        // winner should be displayed.
        color = conio::GREEN;
    }
    strm << conio::setTextStyle(conio::BOLD) << conio::fgColor(color) << setfill(' ') << setw(width) << left << name << conio::resetAll();
    return strm.str();
}


/* ─────────────────────── *
 * DISPLAY ROUND FUNCTIONS *
 * ─────────────────────── */

void display_contest_rounds(DisplayInfo &info, ContestLog &contest, Board &board) {
    vector<ContestPlayer> copy_players;

    // NOTE: set players back to default stats just for the sake of replay.
    for (int i = 0; i < (int)contest.players.size(); i++) {
        ContestPlayer player = contest.players.at(i);
        player.lives = 3;
        if ( !player.played ) {
            player.lives = 0;
            player.error.type = OK;
        }
        memset(&player.stats, 0, sizeof(ContestStats));
        copy_players.push_back(player);
    }

    for (int i = 0; i < (int)contest.rounds.size(); i++) {
        ContestRound &round = contest.rounds.at(i);
        vector<int> round_player_numbers;
        
        for (int j = 0; j < (int)copy_players.size(); j++) {
            ContestPlayer &player = copy_players.at(j);
            if ( player.lives > 0 ) {
                round_player_numbers.push_back(j);
            }
        }

        switch (info.contest_type) {
        case NORMAL:
        case FINAL:
            break;
        case ROUNDS:
            display_round_screen(info, i+1);
            break;
        }

        for (int j = 0; j < (int)round.matches.size(); j++) {
            ContestMatch &match = round.matches.at(j);
            ContestPlayer &player1 = copy_players.at(match.player1.player_idx);
            ContestPlayer &player2 = copy_players.at(match.player2.player_idx);

            collect_contest_player_stats(player1, match.player1);
            collect_contest_player_stats(player2, match.player2);

            display_contest_match(info, match, player1, player2, board, i);
        }

        vector<ContestPlayer> round_players;
        for (int j = 0; j < (int)round_player_numbers.size(); j++) {
            int idx = round_player_numbers.at(j);
            ContestPlayer &player = copy_players.at(idx);
            round_players.push_back(player);
        }

        switch (info.contest_type) {
        case FINAL:
            break;
        case NORMAL:
            display_round_screen(info, i);
            display_round_leaderboard(info, round_players);
            break;
        case ROUNDS:
            display_round_leaderboard(info, round_players);
            break;
        }
    }
    return;
}

void display_round_screen(DisplayInfo &info, int round_num) {
    reset_screen(info);
    cout << conio::gotoRowCol(info.display_row, 1)
         << conio::setTextStyle(conio::BOLD)
         << "Round #" << round_num + 1 << conio::setTextStyle(conio::NORMAL_INTENSITY) << flush;
    info.display_row += 2;
    return;
}

void display_round_leaderboard(DisplayInfo &info, vector<ContestPlayer> &round_players) {
    const string
        RANK   = "Rank",
        NAME   = "Name",
        WINS   = "Wins",
        LOSSES = "Losses",
        TIES   = "Ties";
    
    int rank_width = (int)RANK.size(),
        name_width = (int)NAME.size(),
        num_width  = 6;
    
    sort(round_players.begin(), round_players.end(), sort_players_by_rank);

    cout << conio::gotoRowCol(info.display_row, 1)
            << conio::setTextStyle(conio::BOLD)
            << "Leaderboard"
            << conio::setTextStyle(conio::NORMAL_INTENSITY);
    info.display_row += 2;

    // get max name width
    for (int i = 0; i < (int)round_players.size(); i++) {
        ContestPlayer &player = round_players.at(i);
        if ( (int)player.ai_name.size() > name_width ) {
            name_width = player.ai_name.size();
        }
    }
    if ( name_width < 5 ) name_width = 5;

    cout << conio::gotoRowCol(info.display_row, 1) << " "
         << setfill(' ') << setw(rank_width) << right << RANK << " " << vertical << " "
         << setfill(' ') << setw(name_width) << left << NAME << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << WINS << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << LOSSES << " " << vertical << " "
         << setfill(' ') << setw(num_width) << right << TIES << " " << vertical;
    info.display_row++; 

    cout << conio::gotoRowCol(info.display_row, 1)
         << multiply_string(horizontal, rank_width+2) << intersection
         << multiply_string(horizontal, name_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << intersection
         << multiply_string(horizontal, num_width+2) << end_horizontal;
    info.display_row++;

    for (int i = 0; i < (int)round_players.size(); i++) {
        ContestPlayer &player = round_players.at(i);
        cout << conio::gotoRowCol(info.display_row, 1) << " "
            << setfill(' ') << setw(rank_width) << right << i+1 << " " << vertical << " "
            << print_name_by_status(player.ai_name, name_width, player.lives) << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.wins << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.losses << " " << vertical << " "
            << setfill(' ') << setw(num_width) << right << player.stats.ties << " " << vertical;
        info.display_row++; 
    }
    info.display_row++;

    cout << conio::gotoRowCol(info.display_row, 1)
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::RESET) << " *" << conio::resetAll()
         << " - Player is moving on" << endl
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::YELLOW) << " *" << conio::resetAll()
         << " - Player is close to removal" << endl
         << conio::setTextStyle(conio::BOLD) << conio::fgColor(conio::RED) << " *" << conio::resetAll()
         << " - Player will be removed next round" << endl;
    info.display_row += 3;
    reset_cursor(info.display_row);
    sleep(5);
    return;
}

string print_name_by_status(string name, int width, int lives) {
    ostringstream strm;
    conio::Color color = conio::RESET;
    switch (lives) {
    case 1:
        color = conio::YELLOW;
        break;
    case 0:
        color = conio::RED;
        break;
    }
    strm << conio::setTextStyle(conio::BOLD) << conio::fgColor(color) << setfill(' ') << setw(width) << left << name << conio::resetAll();
    return strm.str();
}

bool sort_players_by_rank(const ContestPlayer &a, const ContestPlayer &b) {
    // compare the number of lives.
    if ( a.lives < b.lives ) return false;

    // check for errors and put errored players at the end.
    if ( a.error.type != OK && b.error.type == OK ) return false;

    // check for played status and move unplayed players to the end.
    if ( !a.played && b.played ) return false;

    // check wins.
    if ( a.stats.wins < b.stats.wins ) return false;

    // if a has better or equal lives to b
    // if a error state is better or equal to b
    // if a played is better or equal to b
    // if a has won more or equal to b
    return true;
}


/* ─────────────────────── *
 * DISPLAY MATCH FUNCTIONS *
 * ─────────────────────── */

void display_contest_match(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2, Board &board, int round_num) {
    switch (info.contest_type) {
    case FINAL:
        break;
    case NORMAL:
        display_round_screen(info, round_num);
        display_contest_match_game(info, match, player1, player2, board);
        display_contest_match_result(info, match, player1, player2);
        sleep(2);
        break;
    case ROUNDS:
        display_contest_match_result(info, match, player1, player2);
        break;
    }
    return;
}

void display_contest_match_game(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2, Board &board) {
    switch (info.contest_type) {
    case ROUNDS:
    case FINAL:
        break;
    case NORMAL:
        info.player1.ai_name = player1.ai_name;
        info.player1.author_name = player1.author_name;
        info.player2.ai_name = player2.ai_name;
        info.player2.author_name = player2.author_name;

        display_match_vs(info);
        display_game(info, match.last_game, board);
        break;
    }
    return;
}

void display_contest_match_result(DisplayInfo &info, ContestMatch &match, ContestPlayer &player1, ContestPlayer &player2) {
    display_contest_match_vs(info, player1.ai_name, player2.ai_name);
    switch (info.contest_type) {
    case FINAL:
        break;
    case NORMAL:
    case ROUNDS:
        cout << conio::gotoRowCol(info.display_row, 1) << conio::setTextStyle(conio::BOLD)
             << print_contest_match_by_result(match.player1.match_result, player1.ai_name, player2.ai_name)
             << conio::setTextStyle(conio::NORMAL_INTENSITY);
        info.display_row += 2;
        reset_cursor(info.display_row);
        sleep(1);
        break;
    }
    return;
}

void display_contest_match_vs(DisplayInfo &info, string name1, string name2) {
    switch (info.contest_type) {
    case NORMAL:
    case FINAL:
        break;
    case ROUNDS:
        cout << conio::gotoRowCol(info.display_row, 1)
             << name1 << " VS " << name2;
        info.display_row++;
        break;
    }
    return;
}

string print_contest_match_by_result(GameResult player1_result, string p1_name, string p2_name) {
    ostringstream strm;
    switch (player1_result) {
    case WIN:
        strm << conio::fgColor(conio::GREEN)
             << p1_name
             << conio::fgColor(conio::RESET)
             << " won the match! (player 1)";
        break;
    case LOSS:
        strm << conio::fgColor(conio::GREEN)
             << p2_name
             << conio::fgColor(conio::RESET)
             << " won the match! (player 2)";
        break;
    case TIE:
        strm << "The match was a tie.";
        break;
    }
    return strm.str();
}
