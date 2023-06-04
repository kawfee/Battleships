/**
 * @file logger.h
 * @author Joey Gorski, Matthew Getgen
 * @brief Battleships Logger header file for Battleships Contes
 * @date 2022-09-17
 */

#ifndef LOGGER_H
#define LOGGER_H


#include <fstream>
#include <iomanip>

#include "defines.h"


/* ───────────────────────── *
 * CONTEST LOGGING FUNCTIONS *
 * ───────────────────────── */

/**
 * @brief stores the contest log into the contest_log.json file.
 * @param contest_log json to store into file.
 * @param log_dir log directory to store the contest_log.json file.
 */
void log_contest(json &contest_log, string log_dir);

/**
 * @brief Initialized the contest log JSON and stores basic info about the contest.
 * @param contest_log reference to the contest JSON object.
 * @param board_size size of the board.
 */
void make_contest_json(json &contest_log, int board_size);

void append_players_to_log(json &contest_log, vector<ContestPlayer> &players);

/**
 * @brief Takes a contest player struct and appends the data to the contest log.
 * @param contest_log reference to the contest JSON object.
 * @param player contest player struct to store.
 */
void append_player_to_log(json &contest_log, ContestPlayer player);

/**
 * @brief Adds a new round to the set of matches in the contest.
 * @param contest_log contest JSON to append to.
 */
void append_new_contest_round(json &contest_log);

/**
 * @brief Takes a match struct (returned from run_match()),
 * and a match JSON object and stores it in the contest log.
 * @param contest_log reference to the contest JSON object.
 * @param match_log reference to the match JSON object (used to access the last game).
 * @param match match struct to store into the contest JSON object.
 * @param player1 contest player struct of player1.
 * @param player2 contest player struct of player2.
 * @param contest_round the contest round number to append the match to (contest log stores matches by in an array of rounds, which is an array of matches in that round).
 */
void append_match_to_contest_log(json &contest_log, json &match_log, Match match, ContestPlayer &player1, ContestPlayer &player2, int contest_round);


/* ─────────────────────── *
 * MATCH LOGGING FUNCTIONS *
 * ─────────────────────── */

/**
 * @brief Stores the match log JSON after the match is over.
 * This is called at the end of the match, and will catch any match errors that occur.
 * @param match_log JSON object storing all the game data so far.
 * @param log_dir log directory to store the match_log.json file to 
 */
void log_match(json &match_log, string log_dir);

/**
 * @brief Initializes the match log JSON and stores basic info about the match.
 * @param match_log reference to the match JSON object.
 * @param board_size size of the board.
 */
void make_match_json(json &match_log, int board_size);

/**
 * @brief Stores all match data after a match into the log JSON.
 * @param match_log match JSON to store into.
 * @param match match data structure to get values from.
 */
void merge_match_struct_to_log(json &match_log, Match match);

/**
 * @brief Creates a step-through match log of entire match
 * with an array containing each game.
 * This step-through is only used for stepping through a game.
 * @param match_log JSON game log object.
 * @return JSON object storing the step-through log.
 */
json get_step_through_match_log(json &match_log, vector<int> game_indexes);

/**
 * @brief Creates a step-through game log with two arrays for each player.
 * The array is a merge of a game's ships and shots.
 * This step-through is only used for stepping through a game.
 * @param game_log JSON game log object.
 * @return JSON object storing the step-through log.
 */
json get_step_through_game_log(json &game_log);


/* ────────────────────── *
 * GAME LOGGING FUNCTIONS *
 * ────────────────────── */

/**
 * @brief Initializes the game log and stores all default data about the game.
 * @return JSON object for each game.
 */
json make_game_json();

/**
 * @brief Takes a ship that was placed and stores it in the log file.
 * @param game_log JSON game log object.
 * @param ship ship to store in JSON object.
 * @param player which player's ship it is.
 */
void append_ship_to_json(json &game_log, Ship &ship, PlayerNum player);

/**
 * @brief Takes a shot that was made and stores it in the log file.
 * @param game_log JSON game log object.
 * @param shot shot to store in JSON object.
 * @param player which player's shot to store.
 */
void append_shot_to_json(json &game_log, Shot &shot, PlayerNum player);

/**
 * @brief Takes a dead ship, compares it to the list of dead ships, and stores the index of that ship.
 * @param game_log JSON game log object.
 * @param ship ship to check against.
 * @param player which player's ship to mark as kill.
 */
void store_dead_ship_to_json(json &game_log, Ship ship, PlayerNum player);

/**
 * @brief Stores the result of the game for each player into the log file.
 * @param game_log JSON game log object.
 * @param outcome1 player 1's outcome.
 * @param outcome2 player 2's outcome.
 */
void store_game_result_to_json(json &game_log, GameResult outcome1, GameResult outcome2);

/**
 * @brief Creates a step-through log with two arrays for each player.
 * The array is a merge of a game's ships and shots.
 * This step-through is only used for stepping through a game.
 * @param game_log JSON game log object.
 * @return JSON object storing the step-through log.
 */
json get_step_through_log(json &game_log);


/* ───────────────────── *
 * LOG PARSING FUNCTIONS *
 * ───────────────────── */

/**
 * @brief Moves a ship stored in JSON to a ship struct.
 * @param ship ship data structure to store to.
 * @param ship_j ship JSON value to move from.
 */
void move_ship_to_struct(Ship &ship, json &ship_j);

/**
 * @brief Moves a shot stored in JSON to a shot struct.
 * @param shot shot data structure to store to.
 * @param shot_j shot JSON value to move from.
 */
void move_shot_to_struct(Shot &shot, json &shot_j);


/* ──────────────────────── *
 * LOG VALIDATION FUNCTIONS *
 * ──────────────────────── */

/**
 * @brief Validates the integrity of the contest log.
 * @param c contest JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_contest_log(json &c);

/**
 * @brief Validates the integrity of individual contest players.
 * @param p player JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_contest_match_player_log(json &p);

/**
 * @brief Validates the integrity of the match log.
 * @param m match JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_match_log(json &m);

/**
 * @brief Validates the integrity of the match player log.
 * @param p player JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_match_player_log(json &p);

/**
 * @brief Validates the integrity of the game log.
 * @param g game JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_game_log(json &g);

/**
 * @brief Validates the integrity of the game player log.
 * @param p player JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_game_player_log(json &p);

/**
 * @brief Validates the integrity of the ship log.
 * @param s ship JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_ship_log(json &s);

/**
 * @brief Validates the integrity of the shot log.
 * @param s shot JSON object.
 * @return true on valid, false on invalid.
 */
bool validate_shot_log(json &s);

 #endif

 