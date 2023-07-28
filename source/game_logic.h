/**
 * @file game_logic.h
 * @author Matthew Getgen and Joey Gorski
 * @brief Battleships Game Logic Definitions
 * @date 2022-06-20
 */

#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <vector>
#include <cstdlib>
#include <ctime>

#include "defines.h"
#include "server.h"
#include "message.h"
#include "board.h"
#include "logger.h"
#include "display.h"

#define MAX_ROUNDS 200
#define PLAYER_1_NAME "PLAYER 1"
#define PLAYER_2_NAME "PLAYER 2"


/* ─────────────────────── *
 * CONTEST LOGIC FUNCTIONS *
 * ─────────────────────── */

/**
 * @brief Runs the contest and handles the results of matches.
 * @param board_size size of the board.
 * @param num_games number of games to play.
 * @param connect connection data structure of the socket.
 * @param socket_name socket name for players to connect to.
 * @param execs vector of players in the contest.
 * @param contest_log Contest JSON to store contest values into.
 * @param delay delay of time between actions during display.
 * @param display_now whether to display games now (true) or after (false).
 */
void run_contest(int board_size, int num_games, Connection connect, string socket_name, vector<tuple<string, string>> &execs, json &contest_log, int delay, bool display_now);
void run_contest_2(int board_size, int num_games, Connection connect, string socket_name, vector<tuple<string, string>> &execs, json &contest_log, int delay, bool display_now);

void initialize_players(vector<tuple<string, string>> &execs, vector<ContestPlayer> &alive_players, vector<ContestPlayer> &dead_players, Connection connect, string socket_name);

/**
 * @brief Runs, connects, and receives a hello message from an AI to see if they work,
 * and stores their name in a buffer. If a player fails this they do not get added to the contest.
 * @param player player struct to test for basic functionality.
 * @param connect connection data on the socket.
 * @param socket_name name of the socket to pass to players.
 * @return NO_ERR on valid, BAD_FORK, BAD_CONNECT, BAD_RECV, BAD_HELLO_MSG on failure. 
 */
ErrorNum wake_up_test(ContestPlayer &player, Connection connect, string socket_name);

Match handle_match_process(ContestMatch &cmatch, json &match_log, int board_size, int num_games, Connection connect, string socket_name, int delay, bool display_now);

/**
 * @brief After each match, the total wins/losses/ties from the match is stored in the contest player.
 * @param cplayer contest player to store w/l/t to store values to.
 * @param mplayer match player to get w/l/t values from.
 */
void append_match_player_to_contest(ContestPlayer &cplayer, MatchPlayer mplayer);

/**
 * @brief Based on the result of the match, will move players to either passive or dead.
 * @param active_players vector of players to move from.
 * @param passive_players vector of players to move to if haven't died.
 * @param dead_players vector of players to move to if are dead.
 * @param idx index of the active player to move.
 */
void move_player_by_state(vector<ContestPlayer> *active_players, vector<ContestPlayer> *passive_players, vector<ContestPlayer> *dead_players, int idx);


/* ───────────────────── *
 * MATCH LOGIC FUNCTIONS *
 * ───────────────────── */

/**
 * @brief Runs the match and handles all logic and messages for the players.
 * @param board_size size of the board.
 * @param num_games number of games to play.
 * @param connect connection data structure of the socket.
 * @param socket_name socket name for the players to connect to.
 * @param exec1 name of the player 1 AI executable.
 * @param exec2 name of the player 2 AI executable.
 * @param match_log JSON object that stores the match data to log.
 * @return Match data structure to return. Info about the match.
 */
Match run_match(int board_size, int num_games, Connection connect, string socket_name, string exec1, string exec2, json &match_log);


/* ──────────────────── *
 * GAME LOGIC FUNCTIONS *
 * ──────────────────── */

/**
 * @brief Runs the game and handles all logic and messages for the players.
 * @param match match data structure.
 * @param game_log JSON object to log the games to.
 * @param connect connection data structure.
 * @param boards initialized boards data structure.
 * @param msg1 message buffer for player 1.
 * @param msg2 message buffer for player 2.
 * @param num_ships number of ships for the game to use.
 * @param max_ship_len greatest size of a ship for the board size.
 * @param min_ship_len smallest size of a ship for the board size.
 * @return 0 if all is good, -1 if player1 failed, -2 if player2 failed, -3 if both failed.
 */
int run_game(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2, int num_ships, int max_ship_len, int min_ship_len);


/* ──────────────────── *
 * SHIP LOGIC FUNCTIONS *
 * ──────────────────── */

/**
 * @brief Informs both players to place a ship, and validates the placement of that ship.
 * @param match match data structure.
 * @param game_log JSON object to store ships made by the players.
 * @param connect connection data structure.
 * @param boards information on the boards.
 * @param msg1 message buffer for player 1.
 * @param msg2 message buffer for player 2.
 * @param ship1 player 1's ship data structure to store.
 * @param ship2 player 2's ship data structure to store.
 * @param ship_length length of the ships.
 * @return 0 if all is good, -1 if player1 failed, -2 if player2 failed, -3 if both failed.
 */
int handle_ship_placement(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2, Ship &ship1, Ship &ship2, int ship_length);

/**
 * @brief Checks player ship placement and makes sure it is valid (on board and not already taken).
 * @param board board of player to check ships on.
 * @param board_size size of the board.
 * @param ship ship data structure to check on board.
 * @param ship_length length of the ship to validate on.
 * @return NO_ERR if valid, BAD_SHIP if invalid.
 */
ErrorNum validate_ship_placement(char **board, int board_size, Ship &ship, int ship_length);


/* ──────────────────── *
 * SHOT LOGIC FUNCTIONS *
 * ──────────────────── */

/**
 * @brief Informs both players to take a shot and validates the shot.
 * Informs the players the value of their shot.
 * @param match match data structure.
 * @param game_log JSON object to store shots made by the players.
 * @param connect connection data structure.
 * @param boards information on the boards.
 * @param msg1 message buffer for player 1.
 * @param msg2 message buffer for player 2.
 * @return 0 if all is good, -1 if player1 failed, -2 if player2 failed, -3 if both failed.
 */
int handle_shot_placement(Match *match, json &game_log, Connection *connect, Board &boards, char *msg1, char *msg2);

/**
 * @brief Checks the player shot placement and makes sure it is valid (on board).
 * @param board_size size of the board.
 * @param shot shot data structure.
 * @return NO_ERR if valid, BAD_SHOT if invalid.
 */
ErrorNum validate_shot_placement(int board_size, Shot &shot);

/**
 * @brief Get the shot value for the shot made and store it in the shot structure.
 * @param opponent_board board of the opponent for that shot.
 * @param shot shot data structure. Used to access the row and col, and modify the shot value.
 */
void get_shot_value(char **opponent_board, Shot &shot);


/* ─────────────────── *
 * GAME OVER FUNCTIONS *
 * ─────────────────── */

/**
 * @brief Checks if the ships are alive or dead. Tells players about any dead ships.
 * @param match match data structure.
 * @param game_log JSON object to store a possible dead ship.
 * @param connect connection data structure.
 * @param msg message buffer to send.
 * @param board board to check for ship values.
 * @param ships ships for the player.
 * @param player player number (1 or 2).
 * @return 0 if all is good, -1 if player1 failed, -2 if player2 failed, -3 if both failed.
 */
int check_ships_alive(Match *match, json &game_log, Connection *connect, char *msg, char **board, vector<Ship> &ships, PlayerNum player);

/**
 * @brief Checks if the status of the action was a success or not.
 * If a player has lost, it will determine which player has lost.
 * @param match match data structure.
 * @param p1_status status of player 1 action.
 * @param p2_status status of player 2 action.
 * @return 0 if all is good, -1 if player1 failed, -2 if player2 failed, -3 if both failed.
 */
int check_return(Match *match, ErrorNum p1_status, ErrorNum p2_status);

/**
 * @brief Looks at the result of the game and determines the winner.
 * @param match match data structure.
 * @param game_log JSON object to store the win/loss/tie values into.
 * @param ships1 ships for player 1.
 * @param ships2 ships for player 2.
 */
void calculate_winner(Match *match, json &game_log, vector<Ship> &ships1, vector<Ship> &ships2);

/**
 * @brief Sends the game_over message to both players.
 * @param connect connection data structure.
 * @param msg message buffer to send to both player (same message, only need one)
 */
void send_game_over_messages(Connection *connect, char *msg);

#endif

