/**
 * @file logger.cpp
 * @author Matthew Getgen
 * @brief Battleships Logger Logic
 * @date 2023-09-06
 */

#include "logger.h"


/* ───────────────────── *
 * CONTEST LOG FUNCTIONS *
 * ───────────────────── */

void save_contest_log(ContestLog &contest, const string &system_dir) {
    json log = convert_contest_log(contest);
    const string contest_log_file = system_dir + LOGS_DIR + CONTEST_LOG;
    ofstream outfile(contest_log_file.c_str());
    outfile << log << endl;
    outfile.close();
    return;
}

ContestLog open_contest_log(const string &system_dir) {
    ContestLog contest;
    const string contest_log_file = system_dir + LOGS_DIR + CONTEST_LOG;
    ifstream infile(contest_log_file.c_str());
    if ( !infile.is_open() || infile.fail() ) {
        print_error("contest_log.json file doesn't exist!", __FILE__, __LINE__);
        exit(1);
    }
    if ( !json::accept(infile) ) {
        print_error("Invalid JSON found in contest_log.json file!", __FILE__, __LINE__);
        exit(1);
    }
    infile.seekg(0, ios::beg);

    json log = json::parse(infile);
    infile.close();
    if ( !validate_contest_log(contest, log) ) {
        print_error("Invalid contest_log.json file!", __FILE__, __LINE__);
        exit(1);
    }
    return contest;
}

json convert_contest_log(ContestLog &contest) {
    json log = json::object();
    log[BOARD_SIZE_KEY] = contest.board_size;
    log[PLAYERS_KEY] = json::array();
    log[ROUNDS_KEY] = json::array();

    for (int i = 0; i < (int)contest.players.size(); i++) {
        log[PLAYERS_KEY].push_back(convert_contest_player(contest.players.at(i)));
    }

    for (int i = 0; i < (int)contest.rounds.size(); i++) {
        log[ROUNDS_KEY].push_back(convert_contest_round(contest.rounds.at(i)));
    }
    return log;
}

bool validate_contest_log(ContestLog &contest, json &log) {
    bool valid = 
        check_integer(log, BOARD_SIZE_KEY) &&
        check_array(log, PLAYERS_KEY) &&
        check_array(log, ROUNDS_KEY);
    if ( !valid ) {
        return false;
    }
    contest.board_size = (int)log[BOARD_SIZE_KEY];

    for (int i = 0; i < (int)log[PLAYERS_KEY].size(); i++) {
        ContestPlayer player;
        if ( !validate_contest_player_log(player, log[PLAYERS_KEY].at(i)) ) {
            return false;
        }
        contest.players.push_back(player);
    }

    for (int i = 0; i < (int)log[ROUNDS_KEY].size(); i++) {
        ContestRound round;
        if ( !validate_contest_round_log(round, log[ROUNDS_KEY].at(i)) ) {
            return false;
        }
        contest.rounds.push_back(round);
    }
    return true;
}

json convert_contest_player(ContestPlayer &player) {
    json log = json::object();
    log[AI_NAME_KEY] = player.ai_name;
    log[AUTHOR_NAMES_KEY] = player.author_name;
    log[LIVES_KEY] = player.lives;
    log[PLAYED_KEY] = player.played;
    log[WINS_KEY] = player.stats.wins;
    log[LOSSES_KEY] = player.stats.losses;
    log[TIES_KEY] = player.stats.ties;
    log[TOTAL_WINS_KEY] = player.stats.total_wins;
    log[TOTAL_LOSSES_KEY] = player.stats.total_losses;
    log[TOTAL_TIES_KEY] = player.stats.total_ties;
    log[ERROR_KEY] = convert_error(player.error);
    return log;
}

bool validate_contest_player_log(ContestPlayer &player, json &log) {
    bool valid =
        check_string(log, AI_NAME_KEY) &&
        check_string(log, AUTHOR_NAMES_KEY) &&
        check_integer(log, LIVES_KEY) &&
        check_bool(log, PLAYED_KEY) &&
        check_integer(log, WINS_KEY) &&
        check_integer(log, LOSSES_KEY) &&
        check_integer(log, TIES_KEY) &&
        check_integer(log, TOTAL_WINS_KEY) &&
        check_integer(log, TOTAL_LOSSES_KEY) &&
        check_integer(log, TOTAL_TIES_KEY) &&
        check_object(log, ERROR_KEY);
    if ( !valid ) {
        return false;
    }
    player.ai_name = log[AI_NAME_KEY];
    player.author_name = log[AUTHOR_NAMES_KEY];
    player.lives = (int)log[LIVES_KEY];
    player.played = (bool)log[PLAYED_KEY];
    player.stats.wins = (int)log[WINS_KEY];
    player.stats.losses = (int)log[LOSSES_KEY];
    player.stats.ties = (int)log[TIES_KEY];
    player.stats.total_wins = (int)log[TOTAL_WINS_KEY];
    player.stats.total_losses = (int)log[TOTAL_LOSSES_KEY];
    player.stats.total_ties = (int)log[TOTAL_TIES_KEY];
    
    if ( !validate_error_log(player.error, log[ERROR_KEY]) ) {
        return false;
    }
    return true;
}

json convert_contest_round(ContestRound &round) {
    json log = json::object();
    log[MATCHES_KEY] = json::array();

    for (int i = 0; i < (int)round.matches.size(); i++) {
        log[MATCHES_KEY].push_back(convert_contest_match(round.matches.at(i)));
    }
    return log;
}

bool validate_contest_round_log(ContestRound &round, json &log) {
    if ( !check_array(log, MATCHES_KEY) ) {
        return false;
    }

    for (int i = 0; i < (int)log[MATCHES_KEY].size(); i++) {
        ContestMatch match;
        if ( !validate_contest_match_log(match, log[MATCHES_KEY].at(i)) ) {
            return false;
        }
        round.matches.push_back(match);
    }
    return true;
}

json convert_contest_match(ContestMatch &match) {
    json log = json::object();
    log[ELAPSED_TIME_KEY] = match.elapsed_time;
    log[PLAYER_1_KEY] = convert_contest_match_player(match.player1);
    log[PLAYER_2_KEY] = convert_contest_match_player(match.player2);
    log[LAST_GAME_KEY] = convert_game_log(match.last_game);
    return log;
}

bool validate_contest_match_log(ContestMatch &match, json &log) {
    bool valid =
        check_float(log, ELAPSED_TIME_KEY) &&
        check_object(log, PLAYER_1_KEY) &&
        check_object(log, PLAYER_2_KEY) &&
        check_object(log, LAST_GAME_KEY);
    if ( !valid ) {
        return false;
    }
    match.elapsed_time = (float)log[ELAPSED_TIME_KEY];

    if ( !validate_contest_match_player_log(match.player1, log[PLAYER_1_KEY]) ) {
        return false;
    }
    if ( !validate_contest_match_player_log(match.player2, log[PLAYER_2_KEY]) ) {
        return false;
    }
    if ( !validate_game_log(match.last_game, log[LAST_GAME_KEY]) ) {
        return false;
    }
    return true;
}

json convert_contest_match_player(ContestMatchPlayer &player) {
    json log = json::object();
    log[PLAYER_IDX_KEY] = player.player_idx;
    log[GAME_RESULT_KEY] = player.match_result;
    log[STATS_KEY] = convert_match_stats(player.stats);
    log[ERROR_KEY] = convert_error(player.error);
    return log;
}

bool validate_contest_match_player_log(ContestMatchPlayer &player, json &log) {
    bool valid =
        check_integer(log, PLAYER_IDX_KEY) &&
        check_integer(log, GAME_RESULT_KEY) &&
        check_object(log, STATS_KEY) &&
        check_object(log, ERROR_KEY);
    if ( !valid ) {
        return false;
    }
    player.player_idx = (int)log[PLAYER_IDX_KEY];
    player.match_result = (GameResult)log[GAME_RESULT_KEY];

    if ( !validate_match_stats_log(player.stats, log[STATS_KEY]) ) {
        return false;
    }

    if ( !validate_error_log(player.error, log[ERROR_KEY]) ) {
        return false;
    }
    return true;
}


/* ─────────────────── *
 * MATCH LOG FUNCTIONS *
 * ─────────────────── */

void save_match_log(MatchLog &match, const string &system_dir) {
    json log = convert_match_log(match);
    const string match_log_file = system_dir + LOGS_DIR + MATCH_LOG;
    ofstream outfile(match_log_file.c_str());
    outfile << log << endl;
    outfile.close();

    return;
}

MatchLog open_match_log(const string &system_dir) {
    MatchLog match;
    const string match_log_file = system_dir + LOGS_DIR + MATCH_LOG;
    ifstream infile(match_log_file.c_str());

    if ( !infile.is_open() || infile.fail() ) {
        print_error("match_log.json file doesn't exist!", __FILE__, __LINE__);
        exit(1);
    }
    if ( !json::accept(infile) ) {
        print_error("Invalid JSON found in match_log.json file!", __FILE__, __LINE__);
        exit(1);
    }
    infile.seekg(0, ios::beg);

    json log = json::parse(infile);
    infile.close();
    if ( !validate_match_log(match, log) ) {
        print_error("Invalid match_log.json file!", __FILE__, __LINE__);
        exit(1);
    }
    return match;
}

json convert_match_log(MatchLog &match) {
    json log = json::object();
    log[BOARD_SIZE_KEY] = match.board_size;
    log[ELAPSED_TIME_KEY] = match.elapsed_time;
    log[PLAYER_1_KEY] = convert_match_player(match.player1);
    log[PLAYER_2_KEY] = convert_match_player(match.player2);
    log[GAMES_KEY] = json::array();
    for (int i = 0; i < (int)match.games.size(); i++) {
        log[GAMES_KEY].push_back(convert_game_log(match.games.at(i)));
    }

    return log;
}

bool validate_match_log(MatchLog &match, json &log) {
    bool valid = 
        check_integer(log, BOARD_SIZE_KEY) &&
        check_float(log, ELAPSED_TIME_KEY) &&
        check_object(log, PLAYER_1_KEY) &&
        check_object(log, PLAYER_2_KEY) &&
        check_array(log, GAMES_KEY);
    if ( !valid ) return false;

    match.board_size = (int)log[BOARD_SIZE_KEY];
    match.elapsed_time = (float)log[ELAPSED_TIME_KEY];
    validate_match_player_log(match.player1, log[PLAYER_1_KEY]);
    validate_match_player_log(match.player2, log[PLAYER_2_KEY]);

    for (int i = 0; i < (int)log[GAMES_KEY].size(); i++) {
        GameLog game;
        if ( !validate_game_log(game, log[GAMES_KEY].at(i)) ) {
            return false;
        }
        match.games.push_back(game);
    }
    return true;
}

json convert_match_player(MatchPlayer &player) {
    json log = json::object();
    log[AI_NAME_KEY] = player.ai_name.c_str();
    log[AUTHOR_NAMES_KEY] = player.author_name.c_str();
    log[STATS_KEY] = convert_match_stats(player.stats);
    log[ERROR_KEY] = convert_error(player.error);

    return log;
}

bool validate_match_player_log(MatchPlayer &player, json &log) {
    bool valid = 
        check_string(log, AI_NAME_KEY) &&
        check_string(log, AUTHOR_NAMES_KEY) &&
        check_object(log, STATS_KEY) &&
        check_object(log, ERROR_KEY);
    if ( !valid ) return false;

    player.ai_name = log[AI_NAME_KEY];
    player.author_name = log[AUTHOR_NAMES_KEY];

    if ( !validate_match_stats_log(player.stats, log[STATS_KEY]) ) {
        return false;
    }

    if ( !validate_error_log(player.error, log[ERROR_KEY]) ) {
        return false;
    }
    
    return true;
}

json convert_match_stats(MatchStats &stats) {
    json log = json::object();
    log[WINS_KEY] = stats.wins;
    log[LOSSES_KEY] = stats.losses;
    log[TIES_KEY] = stats.ties;
    log[NUM_BOARD_SHOT_KEY] = stats.total_num_board_shot;
    log[NUM_HITS_KEY] = stats.total_hits;
    log[NUM_MISSES_KEY] = stats.total_misses;
    log[NUM_DUPLICATES_KEY] = stats.total_duplicates;
    log[SHIPS_KILLED_KEY] = stats.total_ships_killed;

    return log;
}

bool validate_match_stats_log(MatchStats &stats, json &log) {
    bool valid =
        check_integer(log, WINS_KEY) &&
        check_integer(log, LOSSES_KEY) &&
        check_integer(log, TIES_KEY) &&
        check_integer(log, NUM_BOARD_SHOT_KEY) &&
        check_integer(log, NUM_HITS_KEY) &&
        check_integer(log, NUM_MISSES_KEY) &&
        check_integer(log, NUM_DUPLICATES_KEY) &&
        check_integer(log, SHIPS_KILLED_KEY);
    if ( !valid ) {
        return false;
    }
    stats.wins = (int)log[WINS_KEY];
    stats.losses = (int)log[LOSSES_KEY];
    stats.ties = (int)log[TIES_KEY];
    stats.total_num_board_shot = (int)log[NUM_BOARD_SHOT_KEY];
    stats.total_hits = (int)log[NUM_HITS_KEY];
    stats.total_misses = (int)log[NUM_MISSES_KEY];
    stats.total_duplicates = (int)log[NUM_DUPLICATES_KEY];
    stats.total_ships_killed = (int)log[SHIPS_KILLED_KEY];
    return true;
}

json convert_error(Error &error) {
    json log = json::object();
    log[ERROR_TYPE_KEY] = error.type;
    switch (error.type) {
    case ErrHelloMessage:
    case ErrShipPlacedMessage:
    case ErrShotTakenMessage:
        log[MESSAGE_KEY] = error.message;
        break;
    case ErrShipLength:
    case ErrShipOffBoard:
    case ErrShipIntersect:
        log[SHIP_KEY] = convert_ship(error.ship);
        break;
    case ErrShotOffBoard:
        log[SHOT_KEY] = convert_shot(error.shot);
        break;
    default:
        break;
    }
    return log;
}

bool validate_error_log(Error &error, json &log) {
    if ( !check_integer(log, ERROR_TYPE_KEY) ) {
        return false;
    }
    error.type = (ErrorType) log[ERROR_TYPE_KEY];
    switch (error.type) {
    case ErrHelloMessage:
    case ErrShipPlacedMessage:
    case ErrShotTakenMessage:
        if ( !check_string(log, MESSAGE_KEY) ) return false;
        break;
    case ErrShipLength:
    case ErrShipOffBoard:
    case ErrShipIntersect:
        if ( !check_object(log, SHIP_KEY) ) return false;
        if ( !validate_ship_log(error.ship, log[SHIP_KEY]) ) return false;
        break;
    case ErrShotOffBoard:
        if ( !check_object(log, SHOT_KEY) ) return false;
        if ( !validate_shot_log(error.shot, log[SHOT_KEY]) ) return false;
        break;
    default:
        break;
    }
    return true;
}


/* ────────────────── *
 * GAME LOG FUNCTIONS *
 * ────────────────── */

json convert_game_log(GameLog &game) {
    json log = json::object();
    log[PLAYER_1_KEY] = convert_game_player(game.player1);    
    log[PLAYER_2_KEY] = convert_game_player(game.player2);    

    return log;
}

bool validate_game_log(GameLog &game, json &log) {
    bool valid = 
        check_object(log, PLAYER_1_KEY) &&
        check_object(log, PLAYER_2_KEY);
    if ( !valid ) return false;

    if ( !validate_game_player_log(game.player1, log[PLAYER_1_KEY]) ) {
        return false;
    }
    if ( !validate_game_player_log(game.player2, log[PLAYER_2_KEY]) ) {
        return false;
    }
    return true;
}

json convert_game_player(GamePlayer &player) {
    json log = json::object();
    log[SHIPS_KEY] = json::array();
    for (int i = 0; i < (int)player.ships.size(); i++) {
        log[SHIPS_KEY].push_back(convert_ship(player.ships.at(i)));
    }
    log[SHOTS_KEY] = json::array();
    for (int i = 0; i < (int)player.shots.size(); i++) {
        log[SHOTS_KEY].push_back(convert_shot(player.shots.at(i)));
    }
    log[STATS_KEY] = convert_game_stats(player.stats);
    log[ERROR_TYPE_KEY] = player.error.type;

    return log;
}

bool validate_game_player_log(GamePlayer &player, json &log) {
    bool valid = 
        check_array(log, SHIPS_KEY) &&
        check_array(log, SHOTS_KEY) &&
        check_object(log, STATS_KEY) &&
        check_integer(log, ERROR_TYPE_KEY);
    if ( !valid ) {
        return false;
    }

    for (int i = 0; i < (int)log[SHIPS_KEY].size(); i++) {
        Ship ship;
        if ( !validate_ship_log(ship, log[SHIPS_KEY].at(i)) ) {
            return false;
        }
        player.ships.push_back(ship);
    }
    for (int i = 0; i < (int)log[SHOTS_KEY].size(); i++) {
        Shot shot;
        if ( !validate_shot_log(shot, log[SHOTS_KEY].at(i)) ) {
            return false;
        }
        player.shots.push_back(shot);
    }
    if ( !validate_game_stats_log(player.stats, log[STATS_KEY]) ) {
        return false;
    }
    player.error.type = (ErrorType)log[ERROR_TYPE_KEY];
    return true;
}

json convert_game_stats(GameStats &stats) {
    json log = json::object();
    log[GAME_RESULT_KEY] = stats.result;
    log[NUM_BOARD_SHOT_KEY] = stats.num_board_shot;
    log[NUM_HITS_KEY] = stats.hits;
    log[NUM_MISSES_KEY] = stats.misses;
    log[NUM_DUPLICATES_KEY] = stats.duplicates;
    log[SHIPS_KILLED_KEY] = stats.ships_killed;

    return log;
}

bool validate_game_stats_log(GameStats &stats, json &log) {
    bool valid = 
        check_integer(log, GAME_RESULT_KEY) &&
        check_integer(log, NUM_BOARD_SHOT_KEY) &&
        check_integer(log, NUM_HITS_KEY) &&
        check_integer(log, NUM_MISSES_KEY) &&
        check_integer(log, NUM_DUPLICATES_KEY) &&
        check_integer(log, SHIPS_KILLED_KEY);
    if ( !valid ) {
        return false;
    }

    stats.result = (GameResult)log[GAME_RESULT_KEY];
    stats.num_board_shot = (int)log[NUM_BOARD_SHOT_KEY];
    stats.hits = (int)log[NUM_HITS_KEY];
    stats.misses = (int)log[NUM_MISSES_KEY];
    stats.duplicates = (int)log[NUM_DUPLICATES_KEY];
    stats.ships_killed = (int)log[SHIPS_KILLED_KEY];
    return true;
}

json convert_ship(Ship &ship) {
    json log = json::object();
    log[ROW_KEY] = ship.row;
    log[COL_KEY] = ship.col;
    log[LEN_KEY] = ship.len;
    log[DIR_KEY] = ship.dir;

    return log;
}

bool validate_ship_log(Ship &ship, json &log) {
    bool valid = 
        check_integer(log, ROW_KEY) &&
        check_integer(log, COL_KEY) &&
        check_integer(log, LEN_KEY) &&
        check_integer(log, DIR_KEY);
    if ( !valid ) return false;

    ship.row = (int)log[ROW_KEY];
    ship.col = (int)log[COL_KEY];
    ship.len = (int)log[LEN_KEY];
    ship.dir = (Direction)log[DIR_KEY];
    return true;
}

json convert_shot(Shot &shot) {
    json log = json::object();
    log[ROW_KEY] = shot.row;
    log[COL_KEY] = shot.col;
    log[VALUE_KEY] = shot.value;
    if (shot.ship_sunk_idx != -1) 
        log[INDEX_SHIP_KEY] = shot.ship_sunk_idx;

    return log;
}

bool validate_shot_log(Shot &shot, json &log) {
    bool valid = 
        check_integer(log, ROW_KEY) &&
        check_integer(log, COL_KEY) &&
        check_integer(log, VALUE_KEY);
    if ( !valid ) return false;

    if ( check_contains(log, INDEX_SHIP_KEY) ) {
        if ( !check_integer(log, INDEX_SHIP_KEY) ) return false;
        shot.ship_sunk_idx = (int)log[INDEX_SHIP_KEY];
    } else {
        shot.ship_sunk_idx = -1;
    }

    shot.row = (int)log[ROW_KEY];
    shot.col = (int)log[COL_KEY];
    shot.value = (BoardValue)log[VALUE_KEY];

    return true;
}


/* ──────────────────── *
 * CHECK JSON FUNCTIONS *
 * ──────────────────── */

bool check_object(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_object();
}

bool check_array(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_array();
}

bool check_string(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_string();
}

bool check_bool(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_boolean();
}

bool check_integer(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_number_integer();
}

bool check_float(json &log, const char *key) {
    if ( !check_contains(log, key) ) {
        return false;
    }
    return log[key].is_number_float();
}

bool check_contains(json &log, const char *key) {
    return log.contains(key);
}
