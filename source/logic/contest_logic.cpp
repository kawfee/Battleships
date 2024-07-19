/**
 * @file contest_logic.cpp
 * @author Matthew Getgen
 * @brief Battleships Contest Logic
 * @date 2023-08-28
 */

#include "contest_logic.h"


ContestLog run_contest(Connection &connect, ContestOptions &options, const char *socket_name) {
    ContestLog contest;
    contest.board_size = options.board_size;

    initialize_players(contest, connect, options.execs, socket_name);
    
    run_standard_contest(contest, connect, options, socket_name);

    return contest;
}

void initialize_players(ContestLog &contest, Connection &connect, vector<Executable> &execs, const char *socket_name) {
    for (int i = 0; i < (int)execs.size(); i++) {
        Executable &exec = execs.at(i);
        ContestPlayer player;
        player.exec = exec;
        player.lives = 3;
        player.played = true;
        player.error.type = OK;
        memset(&player.stats, 0, sizeof(ContestStats));

        wake_up_test(player, connect, socket_name);
        if ( player.error.type != OK ) {
            cerr << endl << player.exec.file_name << " failed a basic test. They will not participate in the contest." << endl;
            player.lives = 0;
            player.played = false;
            player.ai_name = player.exec.file_name;
        }
        contest.players.push_back(player);
    }
    return;
}

void wake_up_test(ContestPlayer &player, Connection &connect, const char *socket_name) {
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, MAX_MSG_SIZE);

    // start the player
    player.error.type = start_player(connect.player1, connect.server_desc, player.exec.exec.c_str(), socket_name);
    if ( player.error.type != OK ) {
        close_player_sockets(connect);
        return;
    }

    // listen for a hello message
    player.error.type = recv_msg(connect.player1.desc, msg);
    if ( player.error.type != OK ) {
        close_player_sockets(connect);
        kill_player(connect.player1.pid);
        return;
    }

    // parse the hello message
    player.error = parse_hello_msg(msg, player.ai_name, player.author_name);
    close_player_sockets(connect);
    kill_player(connect.player1.pid);
    return;
}

void run_standard_contest(ContestLog &contest, Connection &connect, ContestOptions &options, const char *socket_name) {
    vector<ContestMatchPlayer> round_players;
    int round_players_size;

    do {
        append_alive_players_to_round(contest.players, round_players);
        round_players_size = (int)round_players.size();

        handle_contest_round(contest, round_players, connect, options, socket_name);
    } while ( round_players_size > 1 );
    return;
}

void append_alive_players_to_round(vector<ContestPlayer> &players, vector<ContestMatchPlayer> &round_players) {
    round_players.clear();

    for (int i = 0; i < (int)players.size(); i++) {
        ContestMatchPlayer alive_player;
        if ( players.at(i).lives > 0 ) {
            alive_player.player_idx = i;
            alive_player.exec = players.at(i).exec;
            alive_player.error.type = OK;

           round_players.push_back(alive_player); 
        }
    }
    return;
}

void handle_contest_round(ContestLog &contest, vector<ContestMatchPlayer> &round_players, Connection &connect, ContestOptions &options, const char *socket_name) {
    ContestRound round;
    int round_num = (int)contest.rounds.size() + 1;

    randomly_set_match_opponents(round, round_players);
    if ( round.matches.size() == 0 ) {
        return;
    }

    cout << endl << "Running Round #" << round_num << flush;

    for (int i = 0; i < (int)round.matches.size(); i++) {
        ContestMatch &match = round.matches.at(i);

        ContestPlayer &player1 = contest.players.at(match.player1.player_idx);
        ContestPlayer &player2 = contest.players.at(match.player2.player_idx);

        cout << "." << flush;

        handle_contest_match(match, connect, options, socket_name);

        collect_contest_player_stats(player1, match.player1);
        collect_contest_player_stats(player2, match.player2);

    }
    cout << endl << flush;

    contest.rounds.push_back(round);

    return;
}

void randomly_set_match_opponents(ContestRound &round, vector<ContestMatchPlayer> &round_players) {
    int round_players_size = (int)round_players.size();

    while ( round_players_size > 1 ) {
        ContestMatch match;
        match.elapsed_time = 0.0;

        int choice1 = rand() % round_players_size;
        match.player1 = round_players.at(choice1);

        round_players.erase(round_players.begin()+choice1);
        round_players_size = round_players.size();

        int choice2 = rand() % round_players_size;
        match.player2 = round_players.at(choice2);

        round_players.erase(round_players.begin()+choice2);
        round_players_size = (int)round_players.size();

        round.matches.push_back(match);
    }
    return;
}

void collect_contest_player_stats(ContestPlayer &c_player, ContestMatchPlayer &m_player) {
    switch (m_player.match_result) {
    case WIN:
        c_player.stats.wins++;
        break;
    case LOSS:
        c_player.stats.losses++;
        c_player.lives--;
        break;
    case TIE:
        c_player.stats.ties++;
        c_player.lives--;
        break;
    }
    c_player.stats.total_wins += m_player.stats.wins;
    c_player.stats.total_losses += m_player.stats.losses;
    c_player.stats.total_ties += m_player.stats.ties;

    c_player.error = m_player.error;
    if ( c_player.error.type != OK ) c_player.lives = 0;
    return;
}

void handle_contest_match(ContestMatch &c_match, Connection &connect, ContestOptions &contest_options, const char *socket_name) {
    MatchOptions match_options;
    match_options.board_size = contest_options.board_size;
    match_options.num_games = contest_options.num_games;
    match_options.exec1 = c_match.player1.exec;
    match_options.exec2 = c_match.player2.exec;

    timeval start, end;

    gettimeofday(&start, NULL);
    MatchLog match = run_match(connect, match_options, socket_name);
    gettimeofday(&end, NULL);
    store_elapsed_time(match, start, end);
    close_player_sockets(connect);

    collect_match_player_stats(c_match.player1, match.player1);
    collect_match_player_stats(c_match.player2, match.player2);

    if ( c_match.player1.stats.wins > c_match.player2.stats.wins ) {
        c_match.player1.match_result = WIN;
        c_match.player2.match_result = LOSS;
    } else if ( c_match.player2.stats.wins > c_match.player1.stats.wins ) {
        c_match.player1.match_result = LOSS;
        c_match.player2.match_result = WIN;
    } else {
        c_match.player1.match_result = TIE;
        c_match.player2.match_result = TIE;
    }

    c_match.elapsed_time = match.elapsed_time;

    int num_games = (int)match.games.size();
    if ( num_games > 0 ) {
        c_match.last_game = match.games.at(num_games-1);
    }

    return;
}

void collect_match_player_stats(ContestMatchPlayer &c_player, MatchPlayer &m_player) {
    c_player.error = m_player.error;
    c_player.stats = m_player.stats;
    return;
}

