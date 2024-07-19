/**
 * @file logger.h
 * @author Matthew Getgen
 * @brief Battleships Logger header file
 * @date 2023-09-06
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iomanip>

#include "../defines.h"
#include "../json.hpp"

using json = nlohmann::json;


/* ───────────────────── *
 * CONTEST LOG FUNCTIONS *
 * ───────────────────── */

/// @brief Converts ContestLog struct into JSON object and save into file.
/// @param contest ContestLog struct to store.
/// @param contest_log_file Working directory path.
void save_contest_log(ContestLog &contest, const string &system_dir);

/// @brief Opens, reads, and validates contest log file as a valid ContestLog struct.
/// @param contest_log_file Working directory path.
/// @return ContestLog struct with data parsed into it.
ContestLog open_contest_log(const string &system_dir);

/// @brief Converts ContestLog struct into JSON.
/// @param contest ContestLog struct to convert from.
/// @return JSON object from a ContestLog struct.
json convert_contest_log(ContestLog &contest);

/// @brief Validates that a JSON object is a ContestLog struct.
/// @param contest ContestLog struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ContestLog, false if not.
bool validate_contest_log(ContestLog &contest, json &log);

/// @brief Converts ContestPlayer struct into JSON.
/// @param player ContestPlayer struct to convert from.
/// @return JSON object from a ContestPlayer struct.
json convert_contest_player(ContestPlayer &player);

/// @brief Validates that a JSON object is a ContestPlayer struct.
/// @param player ContestPlayer struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ContestPlayer, false if not.
bool validate_contest_player_log(ContestPlayer &player, json &log);

/// @brief Converts ContestRound struct into JSON.
/// @param round ContestRound struct to convert from.
/// @return JSON object from a ContestRound struct.
json convert_contest_round(ContestRound &round);

/// @brief Validates that a JSON object is a ContestRound struct.
/// @param round ContestRound struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ContestRound, false if not.
bool validate_contest_round_log(ContestRound &round, json &log);

/// @brief Converts ContestMatch struct into JSON.
/// @param match ContestMatch struct to convert from.
/// @return JSON object from a ContestMatch struct.
json convert_contest_match(ContestMatch &match);

/// @brief Validates that a JSON object is a ContestMatch struct.
/// @param match ContestMatch struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ContestMatch, false if not.
bool validate_contest_match_log(ContestMatch &match, json &log);

/// @brief Converts ContestMatchPlayer struct into JSON.
/// @param player ContestMatchPlayer struct to convert from.
/// @return JSON object from a ContestMatchPlayer struct.
json convert_contest_match_player(ContestMatchPlayer &player);

/// @brief Validates that a JSON object is a ContestMatchPlayer struct.
/// @param player ContestMatchPlayer struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ContestMatchPlayer, false if not.
bool validate_contest_match_player_log(ContestMatchPlayer &player, json &log);


/* ─────────────────── *
 * MATCH LOG FUNCTIONS *
 * ─────────────────── */

/// @brief Converts MatchLog struct into JSON object and save to file.
/// @param match MatchLog struct to store.
/// @param match_log_file Working directory path.
void save_match_log(MatchLog &match, const string &system_dir);

/// @brief Opens, reads, and validates match log file as a valid MatchLog struct.
/// @param match_log_file Working directory path.
/// @return MatchLog struct with data parsed into it.
MatchLog open_match_log(const string &system_dir);

/// @brief Converts MatchLog struct into JSON.
/// @param match MatchLog struct to convert from.
/// @return JSON object from a MatchLog struct.
json convert_match_log(MatchLog &match);

/// @brief Validates that a JSON object is a MatchLog struct.
/// @param match MatchLog struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid MatchLog, false if not.
bool validate_match_log(MatchLog &match, json &log);

/// @brief Converts MatchPlayer struct into JSON.
/// @param player MatchPlayer struct to convert from.
/// @return JSON object from a MatchPlayer struct.
json convert_match_player(MatchPlayer &player);

/// @brief Validates that a JSON object is a MatchPlayer struct.
/// @param player MatchPlayer struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid MatchPlayer, false if not.
bool validate_match_player_log(MatchPlayer &player, json &log);

/// @brief Converts MatchStats struct into JSON.
/// @param stats MatchStats struct to convert from.
/// @return JSON object from a MatchStats struct.
json convert_match_stats(MatchStats &stats);

/// @brief Validates that a JSON object is a MatchStats struct.
/// @param stats MatchStats struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid MatchStats, false if not.
bool validate_match_stats_log(MatchStats &stats, json &log);

/// @brief Converts Error struct into JSON.
/// @param error Error struct to convert from.
/// @return JSON object from a Error struct.
json convert_error(Error &error);

/// @brief Validates that a JSON object is a Error struct.
/// @param error Error struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid Error, false if not.
bool validate_error_log(Error &error, json &log);


/* ────────────────── *
 * GAME LOG FUNCTIONS *
 * ────────────────── */

/// @brief Converts GameLog struct into JSON.
/// @param game GameLog struct to convert from.
/// @return JSON object from a GameLog struct.
json convert_game_log(GameLog &game);

/// @brief Validates that a JSON object is a GameLog struct.
/// @param game GameLog struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid GameLog, false if not.
bool validate_game_log(GameLog &game, json &log);

/// @brief Converts GamePlayer struct into JSON.
/// @param player GamePlayer struct to convert from.
/// @return JSON object from a GamePlayer struct.
json convert_game_player(GamePlayer &player);

/// @brief Validates that a JSON object is a GamePlayer struct.
/// @param player GamePlayer struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid GamePlayer, false if not.
bool validate_game_player_log(GamePlayer &player, json &log);

/// @brief Converts GameStats struct into JSON.
/// @param stats GameStats struct to convert from.
/// @return JSON object from a GameStats struct.
json convert_game_stats(GameStats &stats);

/// @brief Validates that a JSON object is a GameStats struct.
/// @param stats GameStats struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid GameStats, false if not.
bool validate_game_stats_log(GameStats &stats, json &log);

/// @brief Converts ship struct into JSON.
/// @param ship Ship struct to convert from.
/// @return JSON object from a Ship struct.
json convert_ship(Ship &ship);

/// @brief Validates that a JSON object is a ship struct.
/// @param ship Ship struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid ship, false if not.
bool validate_ship_log(Ship &ship, json &log);

/// @brief Converts shot value into JSON.
/// @param shot Shot struct to convert from.
/// @return JSON object from a Shot struct.
json convert_shot(Shot &shot);

/// @brief Validates that a JSON object is a shot struct.
/// @param shot Shot struct to store validated data into.
/// @param log JSON object to validate from.
/// @return true if valid shot, false if not.
bool validate_shot_log(Shot &shot, json &log);


/* ──────────────────── *
 * CHECK JSON FUNCTIONS *
 * ──────────────────── */

/// @brief Checks if the value at key is an object.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if object, false if not.
bool check_object(json &log, const char *key);

/// @brief Checks if the value at key is an array.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if array, false if not.
bool check_array(json &log, const char *key);

/// @brief Checks if the value at key is a string.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if string, false if not.
bool check_string(json &log, const char *key);

/// @brief Checks if the value at key is a boolean.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if boolean, false if not.
bool check_bool(json &log, const char *key);

/// @brief Checks if the value at key is an integer.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if int, false if not.
bool check_integer(json &log, const char *key);

/// @brief Checks if the value at key is a float.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if float, false if not.
bool check_float(json &log, const char *key);

/// @brief Checks if a key is contained in a JSON log.
/// @param log JSON struct to check against.
/// @param key Key to check for.
/// @return true if contains, false if not.
bool check_contains(json &log, const char *key);

#endif

