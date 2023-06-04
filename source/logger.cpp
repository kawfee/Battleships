/**
 * @file logger.cpp
 * @author Matthew Getgen, Joey Gorski
 * @brief Battleships Logging Logic for Battleships Contest
 * @date 2022-08-22
 * 
 */

#include "logger.h"
#include "defines.h"


/* ───────────────────────── *
 * CONTEST LOGGING FUNCTIONS *
 * ───────────────────────── */

void log_contest(json &contest_log, string log_dir) {
    // save contest_json into contest_log.json file
    ofstream outfile((log_dir + CONTEST_LOG).c_str());
    outfile << contest_log << endl;
    outfile.close();
    return;
}

void make_contest_json(json &contest_log, int board_size) {
    // define basic values for the contest log
    contest_log = json::object();
    contest_log[BOARD_SIZE_KEY] = board_size;
    contest_log[PLAYERS_KEY] = json::array();
    contest_log[MATCHES_KEY] = json::array();
    return;
}

void append_players_to_log(json &contest_log, vector<ContestPlayer> &players) {
    int num_players = players.size();

    for (int i = 0; i < num_players; i++) {
        append_player_to_log(contest_log, players.at(i));
    }
}

void append_player_to_log(json &contest_log, ContestPlayer player) {
    json j_player = json::object();
    // append players that are involved in the contest
    j_player[PLAYER_IDX_KEY] = player.idx;
    j_player[AI_NAME_KEY] = player.ai_name;
    j_player[AUTHOR_NAMES_KEY] = player.author_name;
    j_player[TOTAL_WINS_KEY] = 0;
    j_player[TOTAL_LOSSES_KEY] = 0;
    j_player[TOTAL_TIES_KEY] = 0;

    contest_log[PLAYERS_KEY].push_back(j_player);
    return;
}

void append_new_contest_round(json &contest_log) {
    contest_log[MATCHES_KEY].push_back(json::array());
    return;
}

void append_match_to_contest_log(json &contest_log, json &match_log, Match match, ContestPlayer &player1, ContestPlayer &player2, int contest_round) {
    json j_match = json::object();

    j_match[ELAPSED_TIME_KEY] = match.elapsed_time;
    // update player data in the contest log
    int num_players = contest_log[PLAYERS_KEY].size();
    for (int i = 0; i < num_players; i++) {
        if ( contest_log[PLAYERS_KEY].at(i)[PLAYER_IDX_KEY] == player1.idx ) {
            contest_log[PLAYERS_KEY].at(i)[TOTAL_WINS_KEY]   = player1.total_wins;
            contest_log[PLAYERS_KEY].at(i)[TOTAL_LOSSES_KEY] = player1.total_losses;
            contest_log[PLAYERS_KEY].at(i)[TOTAL_TIES_KEY]   = player1.total_ties;

        } else if ( contest_log[PLAYERS_KEY].at(i)[PLAYER_IDX_KEY] == player2.idx ) {
            contest_log[PLAYERS_KEY].at(i)[TOTAL_WINS_KEY]   = player2.total_wins;
            contest_log[PLAYERS_KEY].at(i)[TOTAL_LOSSES_KEY] = player2.total_losses;
            contest_log[PLAYERS_KEY].at(i)[TOTAL_TIES_KEY]   = player2.total_ties;
        }
    }
    // initialize player 1 values for this match
    j_match[PLAYER_1_KEY] = json::object();
    j_match[PLAYER_1_KEY][PLAYER_IDX_KEY]   = player1.idx;
    j_match[PLAYER_1_KEY][LIVES_KEY]        = player1.lives;
    j_match[PLAYER_1_KEY][WINS_KEY]         = match.player1.wins;
    j_match[PLAYER_1_KEY][LOSSES_KEY]       = match.player1.losses;
    j_match[PLAYER_1_KEY][TIES_KEY]         = match.player1.ties;
    j_match[PLAYER_1_KEY][ERROR_KEY]        = match.player1.error;
    // initialize player 2 values
    j_match[PLAYER_2_KEY] = json::object();
    j_match[PLAYER_2_KEY][PLAYER_IDX_KEY]   = player2.idx;
    j_match[PLAYER_2_KEY][LIVES_KEY]        = player2.lives;
    j_match[PLAYER_2_KEY][WINS_KEY]         = match.player2.wins;
    j_match[PLAYER_2_KEY][LOSSES_KEY]       = match.player2.losses;
    j_match[PLAYER_2_KEY][TIES_KEY]         = match.player2.ties;
    j_match[PLAYER_2_KEY][ERROR_KEY]        = match.player2.error;

    // store last game played
    j_match[LAST_GAME_KEY] = json::object();
    if ( match_log[GAMES_KEY].size() > 0 ) {    // if a game has been played
        j_match[LAST_GAME_KEY] = match_log[GAMES_KEY].back();
    }

    contest_log[MATCHES_KEY].at(contest_round).push_back(j_match);
    return;
}


/* ─────────────────────── *
 * MATCH LOGGING FUNCTIONS *
 * ─────────────────────── */

void log_match(json &match_log, string log_dir) {
    // save match_json into match_log.json file
    ofstream outfile((log_dir + MATCH_LOG).c_str());
    outfile << match_log << endl;
    outfile.close();
    return;
}

void make_match_json(json &match_log, int board_size) {
    // define basic values to the match log
    match_log = json::object();
    match_log[ELAPSED_TIME_KEY] = 0;
    match_log[BOARD_SIZE_KEY] = board_size;
    match_log[PLAYER_1_KEY] = json::object();
    match_log[PLAYER_2_KEY] = json::object();
    match_log[GAMES_KEY] = json::array();
    return;
}

void merge_match_struct_to_log(json &match_log, Match match) {
    match_log[ELAPSED_TIME_KEY] = match.elapsed_time;

    // initialize values for player 1
    match_log[PLAYER_1_KEY][AI_NAME_KEY]        = match.player1.ai_name;
    match_log[PLAYER_1_KEY][AUTHOR_NAMES_KEY]   = match.player1.author_name;
    match_log[PLAYER_1_KEY][WINS_KEY]           = match.player1.wins;
    match_log[PLAYER_1_KEY][LOSSES_KEY]         = match.player1.losses;
    match_log[PLAYER_1_KEY][TIES_KEY]           = match.player1.ties;
    match_log[PLAYER_1_KEY][ERROR_KEY]          = match.player1.error;

    // initialize values for player 2
    match_log[PLAYER_2_KEY][AI_NAME_KEY]        = match.player2.ai_name;
    match_log[PLAYER_2_KEY][AUTHOR_NAMES_KEY]   = match.player2.author_name;
    match_log[PLAYER_2_KEY][WINS_KEY]           = match.player2.wins;
    match_log[PLAYER_2_KEY][LOSSES_KEY]         = match.player2.losses;
    match_log[PLAYER_2_KEY][TIES_KEY]           = match.player2.ties;
    match_log[PLAYER_2_KEY][ERROR_KEY]          = match.player2.error;
    return;
}

json get_step_through_match_log(json &match_log, vector<int> game_indexes) {
    json new_match_log = json::object();
    new_match_log[GAMES_KEY] = json::array();
    
	int num_games = game_indexes.size();
	for (int i = 0; i < num_games; i++) {
        json game_log = get_step_through_game_log(match_log[GAMES_KEY].at(game_indexes.at(i)));
        new_match_log[GAMES_KEY].push_back(game_log);
	}

    return new_match_log;
}

json get_step_through_game_log(json &game_log) {

    json new_log = json::object();
    new_log[PLAYER_1_KEY] = json::array();
    new_log[PLAYER_2_KEY] = json::array();

    // each player should have the same num ships.
    int ships_len = (int)game_log[PLAYER_1_KEY][SHIPS_KEY].size();
    for (int i = 0; i < ships_len; i++) {
        new_log[PLAYER_1_KEY].push_back(game_log[PLAYER_1_KEY][SHIPS_KEY].at(i));
        new_log[PLAYER_2_KEY].push_back(game_log[PLAYER_2_KEY][SHIPS_KEY].at(i));
    }

    // each player should have the same num shots.
    int shots_len = (int)game_log[PLAYER_1_KEY][SHOTS_KEY].size();
    for (int i = 0; i < shots_len; i++) {
        new_log[PLAYER_1_KEY].push_back(game_log[PLAYER_1_KEY][SHOTS_KEY].at(i));
        new_log[PLAYER_2_KEY].push_back(game_log[PLAYER_2_KEY][SHOTS_KEY].at(i));
    }

    return new_log;
}


/* ────────────────────── *
 * GAME LOGGING FUNCTIONS *
 * ────────────────────── */

json make_game_json() {
    json game_log = json::object();
    
    // initialize player1 values
    game_log[PLAYER_1_KEY] = json::object();
    game_log[PLAYER_1_KEY][SHIPS_KEY] = json::array();
    game_log[PLAYER_1_KEY][SHOTS_KEY] = json::array();
    game_log[PLAYER_1_KEY][OUTCOME_KEY] = 0;

    // initialize player2 values
    game_log[PLAYER_2_KEY] = json::object();
    game_log[PLAYER_2_KEY][SHIPS_KEY] = json::array();
    game_log[PLAYER_2_KEY][SHOTS_KEY] = json::array();
    game_log[PLAYER_2_KEY][OUTCOME_KEY] = 0;

    return game_log;
}

void append_ship_to_json(json &game_log, Ship &ship, PlayerNum player) {
    json j_ship = json::object();
    // define a ship json and store it
    j_ship[ROW_KEY] = ship.row;
    j_ship[COL_KEY] = ship.col;
    j_ship[LEN_KEY] = ship.len;
    j_ship[DIR_KEY] = ship.dir;
    if ( player == PLAYER_1 ) game_log[PLAYER_1_KEY][SHIPS_KEY].push_back(j_ship);
    else                      game_log[PLAYER_2_KEY][SHIPS_KEY].push_back(j_ship);
    return;
}

void append_shot_to_json(json &game_log, Shot &shot, PlayerNum player) {
    json j_shot = json::object();
    j_shot[ROW_KEY] = shot.row;
    j_shot[COL_KEY] = shot.col;
    j_shot[VALUE_KEY] = shot.value;
    if ( player == PLAYER_1 ) game_log[PLAYER_1_KEY][SHOTS_KEY].push_back(j_shot);
    else                      game_log[PLAYER_2_KEY][SHOTS_KEY].push_back(j_shot);
    return;
}

void store_dead_ship_to_json(json &game_log, Ship ship, PlayerNum player) {
    char player_key[] = PLAYER_1_KEY, opponent_key[] = PLAYER_2_KEY;
    if ( player == PLAYER_2 ) {
        strncpy(player_key, PLAYER_2_KEY, sizeof(player_key));
        strncpy(opponent_key, PLAYER_1_KEY, sizeof(opponent_key));
    }

    int num_ships = game_log[player_key][SHIPS_KEY].size();
    Ship ship_check;
    // for all ships in player's ships
    for (int i = 0; i < num_ships; i++) {
        ship_check.row = game_log[player_key][SHIPS_KEY].at(i)[ROW_KEY];
        ship_check.col = game_log[player_key][SHIPS_KEY].at(i)[COL_KEY];
        ship_check.len = game_log[player_key][SHIPS_KEY].at(i)[LEN_KEY];
        ship_check.dir = (Direction)game_log[player_key][SHIPS_KEY].at(i)[DIR_KEY];
        // check if all values equal. Ideally, just the row check should be good enough, but just in case I check them all.
        if ( ship_check.row == ship.row && ship_check.col == ship.col && ship_check.len == ship.len && ship_check.dir == ship.dir ) {
            // opponent's shot killed your ship.
            game_log[opponent_key][SHOTS_KEY].back()[INDEX_SHIP_KEY] = i;
            break;
        }
    }
    return;
}

void store_game_result_to_json(json &game_log, GameResult outcome1, GameResult outcome2) {
    game_log[PLAYER_1_KEY][OUTCOME_KEY] = outcome1;
    game_log[PLAYER_2_KEY][OUTCOME_KEY] = outcome2;
    return;
}

json get_step_through_log(json &game_log) {
    json new_log = json::object();
    new_log[PLAYER_1_KEY] = json::array();
    new_log[PLAYER_2_KEY] = json::array();

    // each player should have the same num ships.
    int ships_len = (int)game_log[PLAYER_1_KEY][SHIPS_KEY].size();
    for (int i = 0; i < ships_len; i++) {
        new_log[PLAYER_1_KEY].push_back(game_log[PLAYER_1_KEY][SHIPS_KEY].at(i));
        new_log[PLAYER_2_KEY].push_back(game_log[PLAYER_2_KEY][SHIPS_KEY].at(i));
    }

    // each player should have the same num shots.
    int shots_len = (int)game_log[PLAYER_1_KEY][SHOTS_KEY].size();
    for (int i = 0; i < shots_len; i++) {
        new_log[PLAYER_1_KEY].push_back(game_log[PLAYER_1_KEY][SHOTS_KEY].at(i));
        new_log[PLAYER_2_KEY].push_back(game_log[PLAYER_2_KEY][SHOTS_KEY].at(i));
    }

    return new_log;
}


/* ───────────────────── *
 * LOG PARSING FUNCTIONS *
 * ───────────────────── */

void move_ship_to_struct(Ship &ship, json &ship_j) {
	ship.row = ship_j[ROW_KEY];
	ship.col = ship_j[COL_KEY];
	ship.len = ship_j[LEN_KEY];
	ship.dir = (Direction)ship_j[DIR_KEY];
	return;
}

void move_shot_to_struct(Shot &shot, json &shot_j) {
	shot.row = shot_j[ROW_KEY];
	shot.col = shot_j[COL_KEY];
	shot.value = (BoardValue)shot_j[VALUE_KEY];
	return;
}


/* ──────────────────────── *
 * LOG VALIDATION FUNCTIONS *
 * ──────────────────────── */

bool validate_contest_log(json &c) {
	try {
		// check for basic values
		if ( !c.contains(BOARD_SIZE_KEY) ||
			 !c.contains(PLAYERS_KEY) ||
			 !c.contains(MATCHES_KEY) ) throw;

		int num_players = c[PLAYERS_KEY].size();
        // check values for each contest player
		for (int i = 0; i < num_players; i++) {
			if ( !c[PLAYERS_KEY].at(i).contains(PLAYER_IDX_KEY) ||
				 !c[PLAYERS_KEY].at(i).contains(AI_NAME_KEY) ||
				 !c[PLAYERS_KEY].at(i).contains(AUTHOR_NAMES_KEY) ||
				 !c[PLAYERS_KEY].at(i).contains(TOTAL_WINS_KEY) ||
				 !c[PLAYERS_KEY].at(i).contains(TOTAL_LOSSES_KEY) ||
				 !c[PLAYERS_KEY].at(i).contains(TOTAL_TIES_KEY) ) throw;
		}

		int num_contest_rounds = c[MATCHES_KEY].size();
        // check values for each contest match
		for (int i = 0; i < num_contest_rounds; i++) {
            int num_matches = c[MATCHES_KEY].at(i).size();
            for (int j = 0; j < num_matches; j++) {
                if ( !c[MATCHES_KEY].at(i).at(j).contains(ELAPSED_TIME_KEY) || 
                     !c[MATCHES_KEY].at(i).at(j).contains(PLAYER_1_KEY) || 
                     !c[MATCHES_KEY].at(i).at(j).contains(PLAYER_2_KEY) || 
                     !c[MATCHES_KEY].at(i).at(j).contains(LAST_GAME_KEY) ) throw;
                if ( !validate_contest_match_player_log(c[MATCHES_KEY].at(i).at(j)[PLAYER_1_KEY]) || 
                     !validate_contest_match_player_log(c[MATCHES_KEY].at(i).at(j)[PLAYER_2_KEY]) ) throw;
                if ( !validate_game_log(c[MATCHES_KEY].at(i).at(j)[LAST_GAME_KEY]) ) throw;
            }
			
		}
	} catch (...) {
		print_error("Invalid contest log!", __FILE__, __LINE__);
		return false;
	}
	return true;
}

bool validate_contest_match_player_log(json &p) {
	return ( p.contains(PLAYER_IDX_KEY) && 
			 p.contains(LIVES_KEY) && 
			 p.contains(WINS_KEY) && 
			 p.contains(LOSSES_KEY) && 
			 p.contains(TIES_KEY) && 
			 p.contains(ERROR_KEY) );
}

bool validate_match_log(json &m) {
	try {
		// check for basic values
		if ( !m.contains(ELAPSED_TIME_KEY) || 
			 !m.contains(BOARD_SIZE_KEY) || 
			 !m.contains(PLAYER_1_KEY) || 
			 !m.contains(PLAYER_2_KEY) || 
			 !m.contains(GAMES_KEY)) throw;
        // check each match player
		if ( !validate_match_player_log(m[PLAYER_1_KEY]) ||
			 !validate_match_player_log(m[PLAYER_2_KEY]) ) throw;

		int num_games = m[GAMES_KEY].size();
        // validate each game stored for the match
		for (int i = 0; i < num_games; i++) {
			if ( !validate_game_log(m[GAMES_KEY].at(i)) ) throw;
		}
	} catch (...) {
		print_error("Invalid match log!", __FILE__, __LINE__);
		return false;
	}
	return true;
}

bool validate_match_player_log(json &p) {
	return ( p.contains(AI_NAME_KEY) && 
			 p.contains(AUTHOR_NAMES_KEY) && 
			 p.contains(WINS_KEY) && 
			 p.contains(LOSSES_KEY) && 
			 p.contains(TIES_KEY) && 
             p.contains(ERROR_KEY) );
}

bool validate_game_log(json &g) {
	// check for basic values.
	if ( !g.contains(PLAYER_1_KEY) ||
		 !g.contains(PLAYER_2_KEY) ) return false;
    // validate each player values for the games.
	if ( !validate_game_player_log(g[PLAYER_1_KEY]) || 
		 !validate_game_player_log(g[PLAYER_2_KEY]) ) return false;
    
	int num_ships,
        num_ships1 = g[PLAYER_1_KEY][SHIPS_KEY].size(),
        num_ships2 = g[PLAYER_2_KEY][SHIPS_KEY].size();
    
    // if the players don't have the equal number of ships.
    if ( num_ships1 != num_ships2 ) return false;
    num_ships = num_ships1;
    // check ships for players 1 and 2.
	for (int i = 0; i < num_ships; i++) {
		if ( !validate_ship_log(g[PLAYER_1_KEY][SHIPS_KEY].at(i)) ) return false;
		if ( !validate_ship_log(g[PLAYER_2_KEY][SHIPS_KEY].at(i)) ) return false;
	}
	int num_shots,
        num_shots1 = g[PLAYER_1_KEY][SHOTS_KEY].size(),
        num_shots2 = g[PLAYER_2_KEY][SHOTS_KEY].size();
    
    // if the players don't have the equal number of shots.
    if ( num_shots1 != num_shots2 ) return false;
    num_shots = num_shots1;

    // check shots for players 1 and 2.
	for (int i = 0; i < num_shots; i++) {
		if ( !validate_shot_log(g[PLAYER_1_KEY][SHOTS_KEY].at(i)) ) return false;
		if ( !validate_shot_log(g[PLAYER_2_KEY][SHOTS_KEY].at(i)) ) return false;
	}
	return true;
}

bool validate_game_player_log(json &p) {
	return ( p.contains(SHIPS_KEY) &&
			 p.contains(SHOTS_KEY) && 
			 p.contains(OUTCOME_KEY) );
}

bool validate_ship_log(json &s) {
	return ( s.contains(ROW_KEY) && 
			 s.contains(COL_KEY) && 
			 s.contains(LEN_KEY) &&
			 s.contains(DIR_KEY) );
}

bool validate_shot_log(json &s) {
	return ( s.contains(ROW_KEY) && 
			 s.contains(COL_KEY) && 
			 s.contains(VALUE_KEY) );
}

