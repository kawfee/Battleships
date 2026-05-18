/**
 * @file contest.c
 * @author Matthew Getgen
 * @brief Logic for running a game.
 * @date 2026-05-12
 */

#include "match.c"

void BShip_RunContest(const char *socket_path, const char *ai_paths[], uint32_t ai_paths_length,
    uint8_t board_size, uint32_t games_per_match, BShip_ContestAlgorithm algorithm, bool debug)
{
    if (socket_path == NULL || ai_paths == NULL)
    {
        return;
    }
    else if (board_size < BSHIP_BOARD_SIZE_MIN || board_size > BSHIP_BOARD_SIZE_MAX)
    {
        return;
    }
    else if (games_per_match < BSHIP_GAMES_PER_MATCH_MIN || games_per_match > BSHIP_GAMES_PER_MATCH_MAX)
    {
        return;
    }


}
