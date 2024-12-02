/**
 * @file contest_logic.h
 * @author Matthew Getgen
 * @brief Battleships Contest Logic
 * @date 2023-08-28
 */

#ifndef CONTEST_LOGIC_H
#define CONTEST_LOGIC_H

#include "match_logic.h"

/// @brief Manages and plays a contest between multiple players.
/// @param connect Connection struct to use throughout contest.
/// @param options Options to use during contest.
/// @param socket_name Name of socket to connect over.
/// @return ContestLog struct with contest values stored.
ContestLog run_contest(
    Connection &connect,
    ContestOptions &options,
    const char *socket_name
);

/// @brief Creates ContestPlayer structs for each player. Also checks
/// the status of the wake up test.
/// Players that can't pass the wake up test are added to the contest,
/// but don't compete.
/// @param contest ContestLog struct to store players into.
/// @param connect Connection struct to connect to players with.
/// @param execs List of executables in the contest.
/// @param socket_name Name of socket to connect over.
void initialize_players(
    ContestLog &contest,
    Connection &connect,
    vector<Executable> &execs,
    const char *socket_name
);

/// @brief Wakes every player process up to make sure they can connect
/// to a socket and send a hello message.
/// @param player ContestPlayer struct to save wake up data to.
/// @param connect Connection struct to connect the player with.
/// @param socket_name Name of socket to connect over.
void wake_up_test(
    ContestPlayer &player,
    Connection &connect,
    const char *socket_name
);

/// @brief Manages the contest at the round level. This is the
/// main loop of the contest (It's a do-while loop baby).
/// The main loop ends when there's only 1 player left.
/// @param contest ContestLog struct to store contest data into.
/// @param connect Connection struct to use throughout contest.
/// @param options Options to use during contest.
/// @param socket_name Name of socket to connect over.
void run_standard_contest(
    ContestLog &contest,
    Connection &connect,
    ContestOptions &options,
    const char *socket_name
);

/// @brief Iterates over all players in the contest and creates a
/// working list of living players.
/// @param players ContestPlayer list to iterate over.
/// @param round_players ContestMatchPlayer list to store living players to.
void append_alive_players_to_round(
    vector<ContestPlayer> &players,
    vector<ContestMatchPlayer> &round_players
);

/// @brief Manages a single round of a contest. Also manages
/// displaying round info if applicable.
/// @param contest ContestLog struct to store round data to.
/// @param round_players ContestMatchPlayer list calculated by
/// append_alive_players_to_round.
/// @param connect Connection struct to use throughout contest.
/// @param options Options to use during contest.
/// @param socket_name Name of socket to connect over.
void handle_contest_round(
    ContestLog &contest,
    vector<ContestMatchPlayer> &round_players,
    Connection &connect,
    ContestOptions &options,
    const char *socket_name
);

/// @brief Randomly chooses a bye player, if there's an odd amount.
/// Chooses based on the players with the lowest last_bye_round.
/// @param contest Contest struct to access players from.
/// @param round Round players to set which player is on bye.
/// @param round_num Number for the round it is.
/// @param round_players Active Players that could be on bye.
void choose_bye_player(
    ContestLog &contest,
    ContestRound &round,
    int round_num,
    vector<ContestMatchPlayer> &round_players
);

/// @brief Randomly selects from all round players and chooses who to play
/// against.
/// Players matched up are removed from the round_players list.
/// If there's an odd number of players, 1 player will be left in this list.
/// @param round ContestRound struct to store matches to.
/// @param round_players ContestMatchPlayer list of players in this round.
void randomly_set_match_opponents(
    ContestRound &round,
    vector<ContestMatchPlayer> &round_players
);

/// @brief Collects all stats, errors, and sets lives based on losses/ties.
/// Collects from contest match player into contest player.
/// @param c_player ContestPlayer struct to store values into after a match.
/// @param m_player ContestMatchPlayer struct get values from.
void collect_contest_player_stats(
    ContestPlayer &c_player,
    ContestMatchPlayer &m_player
);

/// @brief Manages a single match in a round. Also manages displaying
/// a match if applicable.
/// @param c_match ContestMatch struct to store MatchLog info into.
/// @param connect Connection struct to use during match.
/// @param contest_options Options to use during contest.
/// Applied to match options.
/// @param socket_name Name of socket to connect over.
void handle_contest_match(
    ContestMatch &c_match,
    Connection &connect,
    ContestOptions &contest_options,
    const char *socket_name
);

/// @brief Collects all stats and errors from a match player into
/// a contest match player.
/// @param c_player ContestMatchPlayer struct to store values into.
/// @param m_player MatchPlayer struct to get values from.
void collect_match_player_stats(
    ContestMatchPlayer &c_player,
    MatchPlayer &m_player
);

#endif
