/**
 * @file stats.c
 * @author Matthew Getgen
 * @brief Battleships Statistics calculations.
 * @date 2026-08-27
 */

#include <math.h>
#include <string.h>
#include "arena.c"

BSHIP_DEFINE_ARRAY_PUSH(BShip_BaseGameStatsArray, BShip_BaseGameStats)
BSHIP_DEFINE_ARRAY_PUSH(BShip_DerivedGameStatsArray, BShip_DerivedGameStats)

void BShip_BaseAIStats_From_ShotValue(
    BShip_BaseAIMatchStats *match, BShip_BaseAIGameStats *game, BShip_BoardValue value)
{
    switch (value)
    {
    case BSHIP_WATER:
    case BSHIP_SHIP:
    case BSHIP_KILL:
        assert(false);
        break;
    case BSHIP_HIT:
        game->hits++;
        match->hits++;
        break;
    case BSHIP_MISS:
        game->misses++;
        match->misses++;
        break;
    case BSHIP_DUPLICATE_HIT:
        game->duplicate_hits++;
        match->duplicate_hits++;
        break;
    case BSHIP_DUPLICATE_MISS:
        game->duplicate_misses++;
        match->duplicate_misses++;
        break;
    case BSHIP_DUPLICATE_KILL:
        game->duplicate_kills++;
        match->duplicate_kills++;
        break;
    }
}

BShip_BaseMatchStats BShip_BaseMatchStats_Get(BShip_Arena *arena, BShip_MatchData match)
{
    assert(arena != NULL);
    BShip_BaseMatchStats stats = {
        .game_stats = {
            .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_BaseGameStats, match.game_indexes.length),
            .capacity = match.game_indexes.length,
        },
    };
    if (stats.game_stats.buffer == NULL)
    {
        return stats;
    }

    BShip_BaseGameStats game = {0};
    for (size_t i = 0; i < match.events.length; i++)
    {
        BShip_Event event = match.events.buffer[i];
        switch (event.type)
        {
        case BSHIP_EVENT_NONE:
            break;
        case BSHIP_EVENT_GAME_START:
            memset(&game, 0, sizeof(game));
            break;
        case BSHIP_EVENT_SHIP_PLACEMENT:
            game.ships_placed++;
            stats.ships_placed++;
            BShip_Ship ai1_ship = BShip_Ship_From_CompactShip(event.value.compact.ai1_ship);
            game.ship_cells += ai1_ship.length;
            stats.ship_cells += ai1_ship.length;
            break;
        case BSHIP_EVENT_SHOT_RESULT:
        {
            BShip_Shot ai1_shot = BShip_Shot_From_CompactShot(event.value.compact.ai1_shot);
            BShip_Shot ai2_shot = BShip_Shot_From_CompactShot(event.value.compact.ai2_shot);
            BShip_BaseAIStats_From_ShotValue(&stats.ai1, &game.ai1, ai1_shot.value);
            BShip_BaseAIStats_From_ShotValue(&stats.ai2, &game.ai2, ai2_shot.value);
            if (event.value.compact.ai1_ship > 0)
            {
                game.ai2.ships_killed++;
                stats.ai2.ships_killed++;
            }
            if (event.value.compact.ai2_ship > 0)
            {
                game.ai1.ships_killed++;
                stats.ai1.ships_killed++;
            }
        } break;
        case BSHIP_EVENT_GAME_RESULT:
            game.ai1_game_result = event.value.ai1_game_result;
            switch (game.ai1_game_result)
            {
            case BSHIP_WIN:
                stats.ai1_wins++;
                break;
            case BSHIP_LOSS:
                stats.ai1_losses++;
                break;
            case BSHIP_TIE:
                stats.ai1_ties++;
                break;
            }
            BShip_BaseGameStatsArray_Push(&stats.game_stats, game);
            break;
        }
    }

    return stats;
}

typedef struct {
    float *buffer;
    uint32_t length;
    uint32_t capacity;
} BShip_F32Array;

BSHIP_DEFINE_ARRAY_PUSH(BShip_F32Array, float)

BShip_F32Array BShip_F32Array_Initialize(BShip_Arena *arena, uint32_t capacity)
{
    BShip_F32Array array  = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, float, capacity),
        .capacity = capacity,
    };
    if (array.buffer == NULL)
    {
        array.capacity = 0;
    }
    return array;
}

BShip_DerivedGameStat BShip_DerivedGameStat_Calculate(uint32_t numerator, uint32_t denominator, bool ratio)
{
    float multiplier = ratio ? 1.0f : 100.0f;
    BShip_DerivedGameStat stat = {
        .numerator = numerator,
        .denominator = denominator,
        .value = ((float)numerator / (float)denominator) * multiplier,
    };
    return stat;
}

BShip_DerivedGameStat BShip_DerivedGameStat_CalcAndPush(BShip_F32Array *array, uint32_t numerator, uint32_t denominator, bool ratio)
{
    BShip_DerivedGameStat stat = BShip_DerivedGameStat_Calculate(numerator, denominator, ratio);
    BShip_F32Array_Push(array, stat.value);
    return stat;
}

BShip_DerivedMatchStat BShip_DerivedMatchStat_Calculate(BShip_F32Array array)
{
    assert(array.length > 0);
    BShip_DerivedMatchStat stat = {0};
    float sum = 0.0f;
    stat.min = array.buffer[0];
    for (size_t i = 0; i < array.length; i++)
    {
        float value = array.buffer[i];
        stat.min = stat.min < value ? stat.min : value;
        stat.max = stat.max > value ? stat.max : value;
        sum += value;
    }
    stat.avg = sum / (float)array.length;

    // NOTE(mattg): Now that we have avg we can calc stddev
    float squared_difference_sum = 0.0f;
    for (size_t i = 0; i < array.length; i++)
    {
        float value = array.buffer[i];
        squared_difference_sum += powf(value - stat.avg, 2.0f);
    }
    stat.stddev = sqrtf(squared_difference_sum / array.length);
    
    return stat;
}

void BShip_DerivedMatchStats_Calculate(BShip_Arena *arena, BShip_DerivedMatchStats *stats,
    BShip_BaseMatchStats base_match_stats, uint8_t board_size, bool exclude_game_stats)
{
    assert(stats != NULL);
    uint8_t board_cells = board_size * board_size;
    uint32_t num_games = base_match_stats.ai1_wins + base_match_stats.ai1_losses + base_match_stats.ai1_ties;
    stats->ai1.wins = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_wins, num_games, false);
    stats->ai2.wins = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_losses, num_games, false);
    stats->ai1.losses = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_losses, num_games, false);
    stats->ai2.losses = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_wins, num_games, false);
    stats->ai1.ties = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_ties, num_games, false);
    stats->ai2.ties = BShip_DerivedGameStat_Calculate(base_match_stats.ai1_ties, num_games, false);

    BShip_F32Array ai1_hit_rate_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_hit_rate_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai1_duplicate_shots_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_duplicate_shots_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai1_useful_shot_ratio_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_useful_shot_ratio_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai1_amount_board_shot_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_amount_board_shot_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai1_ships_killed_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_ships_killed_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai1_ship_cells_hit_array = BShip_F32Array_Initialize(arena, num_games);
    BShip_F32Array ai2_ship_cells_hit_array = BShip_F32Array_Initialize(arena, num_games);

    for (size_t i = 0; i < base_match_stats.game_stats.length; i++)
    {
        BShip_BaseGameStats base_game_stats = base_match_stats.game_stats.buffer[i];
        uint32_t ai1_duplicates = base_game_stats.ai1.duplicate_hits + base_game_stats.ai1.duplicate_misses + base_game_stats.ai1.duplicate_kills;
        uint32_t ai2_duplicates = base_game_stats.ai2.duplicate_hits + base_game_stats.ai2.duplicate_misses + base_game_stats.ai2.duplicate_kills;
        uint32_t num_shots = base_game_stats.ai1.hits + base_game_stats.ai1.misses + ai1_duplicates;
        BShip_DerivedGameStats game_stats = {0};
        // result
        BShip_GameResult ai2_game_result =  BSHIP_WIN;
        switch (base_game_stats.ai1_game_result)
        {
        case BSHIP_WIN:
            ai2_game_result = BSHIP_LOSS;
            break;
        case BSHIP_LOSS:
            ai2_game_result = BSHIP_WIN;
            break;
        case BSHIP_TIE:
            ai2_game_result = BSHIP_TIE;
            break;
        }
        game_stats.ai1.result.numerator = (uint32_t)base_game_stats.ai1_game_result;
        game_stats.ai2.result.numerator = (uint32_t)ai2_game_result;
        // hit rate
        game_stats.ai1.hit_rate = BShip_DerivedGameStat_CalcAndPush(&ai1_hit_rate_array, base_game_stats.ai1.hits, num_shots, false);
        game_stats.ai2.hit_rate = BShip_DerivedGameStat_CalcAndPush(&ai2_hit_rate_array, base_game_stats.ai2.hits, num_shots, false);
        // duplicate shots
        game_stats.ai1.duplicate_shots = BShip_DerivedGameStat_CalcAndPush(&ai1_duplicate_shots_array, ai1_duplicates, num_shots, false);
        game_stats.ai2.duplicate_shots = BShip_DerivedGameStat_CalcAndPush(&ai2_duplicate_shots_array, ai2_duplicates, num_shots, false);
        // useful shot ratio
        uint32_t ai1_useless_shots = base_game_stats.ai1.misses + ai1_duplicates;
        uint32_t ai2_useless_shots = base_game_stats.ai2.misses + ai2_duplicates;
        game_stats.ai1.useful_shot_ratio = BShip_DerivedGameStat_CalcAndPush(&ai1_useful_shot_ratio_array, base_game_stats.ai1.hits, ai1_useless_shots, true);
        game_stats.ai2.useful_shot_ratio = BShip_DerivedGameStat_CalcAndPush(&ai2_useful_shot_ratio_array, base_game_stats.ai2.hits, ai2_useless_shots, true);
        // amount board shot
        uint32_t ai1_unique_shots = base_game_stats.ai1.hits + base_game_stats.ai1.misses;
        uint32_t ai2_unique_shots = base_game_stats.ai2.hits + base_game_stats.ai2.misses;
        game_stats.ai1.amount_board_shot = BShip_DerivedGameStat_CalcAndPush(&ai1_amount_board_shot_array, ai1_unique_shots, board_cells, false);
        game_stats.ai2.amount_board_shot = BShip_DerivedGameStat_CalcAndPush(&ai2_amount_board_shot_array, ai2_unique_shots, board_cells, false);
        // ships killed
        game_stats.ai1.ships_killed = BShip_DerivedGameStat_CalcAndPush(&ai1_ships_killed_array, base_game_stats.ai1.ships_killed, base_game_stats.ships_placed, false);
        game_stats.ai2.ships_killed = BShip_DerivedGameStat_CalcAndPush(&ai2_ships_killed_array, base_game_stats.ai2.ships_killed, base_game_stats.ships_placed, false);
        // ship cells hit
        game_stats.ai1.ship_cells_hit = BShip_DerivedGameStat_CalcAndPush(&ai1_ship_cells_hit_array, base_game_stats.ai1.hits, base_game_stats.ship_cells, false);
        game_stats.ai2.ship_cells_hit = BShip_DerivedGameStat_CalcAndPush(&ai2_ship_cells_hit_array, base_game_stats.ai2.hits, base_game_stats.ship_cells, false);
        
        if (!exclude_game_stats)
        {
            BShip_DerivedGameStatsArray_Push(&stats->game_stats, game_stats);
        }
    }
    
    stats->ai1.hit_rate = BShip_DerivedMatchStat_Calculate(ai1_hit_rate_array);
    stats->ai2.hit_rate = BShip_DerivedMatchStat_Calculate(ai2_hit_rate_array);
    stats->ai1.duplicate_shots = BShip_DerivedMatchStat_Calculate(ai1_duplicate_shots_array);
    stats->ai2.duplicate_shots = BShip_DerivedMatchStat_Calculate(ai2_duplicate_shots_array);
    stats->ai1.useful_shot_ratio = BShip_DerivedMatchStat_Calculate(ai1_useful_shot_ratio_array);
    stats->ai2.useful_shot_ratio = BShip_DerivedMatchStat_Calculate(ai2_useful_shot_ratio_array);
    stats->ai1.amount_board_shot = BShip_DerivedMatchStat_Calculate(ai1_amount_board_shot_array);
    stats->ai2.amount_board_shot = BShip_DerivedMatchStat_Calculate(ai2_amount_board_shot_array);
    stats->ai1.ships_killed = BShip_DerivedMatchStat_Calculate(ai1_ships_killed_array);
    stats->ai2.ships_killed = BShip_DerivedMatchStat_Calculate(ai2_ships_killed_array);
    stats->ai1.ship_cells_hit = BShip_DerivedMatchStat_Calculate(ai1_ship_cells_hit_array);
    stats->ai2.ship_cells_hit = BShip_DerivedMatchStat_Calculate(ai2_ship_cells_hit_array);
}

BShip_DerivedMatchStats BShip_DerivedMatchStats_Get(BShip_Arena *arena, BShip_MatchData match,
    bool exclude_game_stats)
{
    BShip_DerivedMatchStats stats = {0};
    if (!exclude_game_stats)
    {
        stats.game_stats.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_DerivedGameStats,
            match.game_indexes.length);
        stats.game_stats.capacity = match.game_indexes.length;
    }
    BSHIP_ARENA_TEMP_BEGIN(arena);
    BShip_BaseMatchStats base_match_stats = BShip_BaseMatchStats_Get(arena, match);
    BShip_DerivedMatchStats_Calculate(arena, &stats, base_match_stats, match.board_size, exclude_game_stats);
    BSHIP_ARENA_TEMP_END(arena);
    return stats;
}

BShip_DerivedMatchStats BShip_DerivedMatchStats_From_BaseMatchStats(BShip_Arena *arena,
    BShip_BaseMatchStats base_match_stats, uint8_t board_size, bool exclude_game_stats)
{
    BShip_DerivedMatchStats stats = {0};
    if (!exclude_game_stats)
    {
        stats.game_stats.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_DerivedGameStats,
            base_match_stats.game_stats.length);
        stats.game_stats.capacity = base_match_stats.game_stats.length;
    }

    BSHIP_ARENA_TEMP_BEGIN(arena);
    BShip_DerivedMatchStats_Calculate(arena, &stats, base_match_stats, board_size, exclude_game_stats);
    BSHIP_ARENA_TEMP_END(arena);

    return stats;
}
