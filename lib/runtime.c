/**
 * @file runtime.c
 * @author Matthew Getgen
 * @brief Runtime logic for running games, matches, and contests.
 * @date 2026-05-27
 */

#define _GNU_SOURCE
#include <math.h>

#include "stats.c"
#include "message.c"
#include "log.c"
#include "game.c"
#include "contest.c"

#define BSHIP_CONNECTION_TIMEOUT_MILLISECONDS 500
#define BSHIP_MESSAGE_TIMEOUT_MILLISECONDS 500

BShip_CompactShip BShip_CompactShip_From_Ship(BShip_Ship ship)
{
    assert(ship.row <= 0xF);
    assert(ship.column <= 0xF);
    assert(ship.length <= 0x7);
    assert(ship.direction <= 0x1);
    BShip_CompactShip compact = ship.row & 0xF;
    compact |= ((ship.column & 0xF) << 4);
    compact |= ((ship.length & 0x7) << 8);
    compact |= ((ship.direction & 0x1) << 11);
    return compact;
}

BShip_Ship BShip_Ship_From_CompactShip(BShip_CompactShip compact)
{
    BShip_Ship ship = {
        .row = compact & 0xF,
        .column = (compact >> 4) & 0xF,
        .length = (compact >> 8) & 0x7,
        .direction = (BShip_Direction)(compact >> 11) & 0x1,
    };
    return ship;
}

BShip_CompactShot BShip_CompactShot_From_Shot(BShip_Shot shot)
{
    assert(shot.row <= 0xF);
    assert(shot.column <= 0xF);
    assert(shot.value <= 0xF);
    BShip_CompactShot compact = shot.row & 0xF;
    compact |= ((shot.column & 0xF) << 4);
    compact |= ((shot.value & 0xF) << 8);
    return compact;
}

BShip_Shot BShip_Shot_From_CompactShot(BShip_CompactShot compact)
{
    BShip_Shot shot = {
        .row = compact & 0xF,
        .column = (compact >> 4) & 0xF,
        .value = (BShip_BoardValue)(compact >> 8) & 0xF,
    };
    return shot;
}

BShip_Board BShip_Board_Allocate(BShip_Arena *arena, uint8_t board_size)
{
    assert(arena != NULL);
    assert(BoardSize_Valid(board_size));
    BShip_Board board = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint8_t, board_size * board_size),
        .size = board_size,
    };
    if (board.buffer != NULL)
    {
        memset(board.buffer, (uint8_t)BSHIP_WATER, board.size * board.size);
    }
    return board;
}

typedef enum {
    GAME_STATE_EXPECTING_SHIPS,
    GAME_STATE_EXPECTING_SHOT,
    GAME_STATE_GAMES_OVER,
} GameStateType;

typedef struct {
    BShip_AIFileData file_data;
    BShip_AIConnection *conn;
    BShip_Timer *deadline;
    BShip_Message message;
    BShip_Board board;
    BShip_ShipArray ships;
    BShip_U8Array ship_lengths;
    BShip_U8Array alive_ships;
    BShip_U8Array dead_ships;
    uint32_t hit_counter;
} AIMatchState;

struct BShip_MatchState {
    AIMatchState ai1;
    AIMatchState ai2;
    BShip_Timer *start_timer;
    BShip_Timer *end_timer;
    uint32_t shot_counter;
    uint32_t game_shot_count_max;
    GameStateType game_state;
    bool debug;
    bool next_shot;
};

bool AIMatchState_PushTempValues(BShip_Arena *temp_arena, AIMatchState *state, uint8_t board_size)
{
    state->conn = BShip_Arena_Push(temp_arena, BShip_AIConnection_GetSize());
    state->deadline = BShip_Arena_Push(temp_arena, BShip_Timer_GetSize());
    if (state->conn == NULL || state->deadline == NULL)
    {
        return false;
    }

    state->message.buffer = BSHIP_ARENA_PUSH_ARRAY(temp_arena, char, BSHIP_MESSAGE_SIZE);
    state->message.length = 0;
    if (state->message.buffer == NULL)
    {
        return false;
    }

    state->board = BShip_Board_Allocate(temp_arena, board_size);
    if (state->board.buffer == NULL)
    {
        return false;
    }

    uint8_t ship_count_max = ShipCountMax_From_BoardSize(board_size);
    state->ships.buffer = BSHIP_ARENA_PUSH_ARRAY(temp_arena, BShip_Ship, ship_count_max);
    state->ships.capacity = ship_count_max;
    state->ship_lengths.buffer = BSHIP_ARENA_PUSH_ARRAY(temp_arena, uint8_t, ship_count_max);
    state->ship_lengths.capacity = ship_count_max;
    state->alive_ships.buffer = BSHIP_ARENA_PUSH_ARRAY(temp_arena, uint8_t, ship_count_max);
    state->alive_ships.capacity = ship_count_max;
    state->dead_ships.buffer = BSHIP_ARENA_PUSH_ARRAY(temp_arena, uint8_t, ship_count_max);
    state->dead_ships.capacity = ship_count_max;
    if (state->ships.buffer == NULL || state->ship_lengths.buffer == NULL ||
        state->alive_ships.buffer == NULL || state->dead_ships.buffer == NULL)
    {
        return false;
    }
    return true;
}

bool BShip_AIMatchData_PushNames(BShip_Arena *arena, BShip_AIMatchData *match)
{
    match->name = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->authors = BSHIP_ARENA_PUSH_ARRAY(arena, char, BSHIP_MESSAGE_NAME_SIZE_MAX);
    if (match->name == NULL || match->authors == NULL)
    {
        return false;
    }
    memset(match->name, 0, sizeof(char) * BSHIP_MESSAGE_NAME_SIZE_MAX);
    memset(match->authors, 0, sizeof(char) * BSHIP_MESSAGE_NAME_SIZE_MAX);
    return true;
}

bool BShip_LongTermValues_Initialize(BShip_Arena *arena, BShip_MatchData *match)
{
    match->game_indexes.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, uint32_t, match->game_count);
    if (match->game_indexes.buffer == NULL)
    {
        return false;
    }
    match->game_indexes.capacity = match->game_count;
    uint32_t ship_count_max = ShipCountMax_From_BoardSize(match->board_size) * match->game_count;
    uint32_t shot_count_max = match->board_size * match->board_size * match->game_count;
    uint32_t events_max = ship_count_max + shot_count_max + (match->game_count * 2);
    match->events.buffer = BSHIP_ARENA_PUSH_ARRAY(arena, BShip_Event, events_max);
    if (match->events.buffer == NULL)
    {
        return false;
    }
    match->events.capacity = events_max;
    if (!BShip_AIMatchData_PushNames(arena, &match->ai1) || !BShip_AIMatchData_PushNames(arena, &match->ai2))
    {
        return false;
    }
    return true;
}

bool HasErrored(BShip_MatchData *match)
{
    return (match->ai1.error.type != ERROR_SUCCESS || match->ai2.error.type != ERROR_SUCCESS);
}

bool GameIsOver(BShip_MatchData *match)
{
    return match->state->shot_counter >= match->state->game_shot_count_max ||
        (match->state->game_state == GAME_STATE_EXPECTING_SHOT && (
            match->state->ai1.alive_ships.length == 0 ||
            match->state->ai2.alive_ships.length == 0 ||
            match->state->next_shot == false
        )) || match->state->game_state == GAME_STATE_GAMES_OVER;
}

bool MatchIsOver(BShip_MatchData *match)
{
    return HasErrored(match) || (GameIsOver(match) && match->game_indexes.length >= match->game_count);
}

bool MatchState_ReceiveMessages(BShip_MatchData *match)
{
    match->ai1.error.type = BShip_AIConnection_Receive(match->state->ai1.conn, &match->state->ai1.message,
        match->state->ai1.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai1.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        return true;
    }
    match->ai2.error.type = BShip_AIConnection_Receive(match->state->ai2.conn, &match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        return true;
    }

    return HasErrored(match);
}

bool MatchState_SendMessages(BShip_MatchData *match, bool equal_message)
{
    match->ai1.error.type = BShip_AIConnection_Send(match->state->ai1.conn, match->state->ai1.message,
        match->state->ai1.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai1.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        return true;
    }
    match->ai2.error.type = BShip_AIConnection_Send(match->state->ai2.conn,
        equal_message ? match->state->ai1.message : match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        return true;
    }

    return HasErrored(match);
}

void MatchState_PlaceShips(BShip_MatchData *match)
{
    BShip_U32Array_Push(&match->game_indexes, match->events.length);

    match->state->next_shot = true;
    match->state->shot_counter = 0;
    match->state->ai1.hit_counter = 0;
    match->state->ai2.hit_counter = 0;

    match->state->ai1.ships.length = 0;
    match->state->ai2.ships.length = 0;
    match->state->ai1.ship_lengths.length = 0;
    match->state->ai2.ship_lengths.length = 0;
    match->state->ai1.alive_ships.length = 0;
    match->state->ai2.alive_ships.length = 0;
    match->state->ai1.dead_ships.length = 0;
    match->state->ai2.dead_ships.length = 0;

    size_t board_capacity = match->board_size * match->board_size;
    memset(match->state->ai1.board.buffer, (uint8_t)BSHIP_WATER, sizeof(uint8_t) * board_capacity);
    memset(match->state->ai2.board.buffer, (uint8_t)BSHIP_WATER, sizeof(uint8_t) * board_capacity);

    ShipLengths_Calculate(&match->state->ai1.ship_lengths, match->board_size);
    // NOTE(mattg): we need a copy of this to use when comparing ship lengths for both AIs.
    memcpy(match->state->ai2.ship_lengths.buffer, match->state->ai1.ship_lengths.buffer,
        match->state->ai1.ship_lengths.length * sizeof(uint8_t));
    match->state->ai2.ship_lengths.length = match->state->ai1.ship_lengths.length;

    BShip_Event game_start_event = {
        .type = BSHIP_EVENT_GAME_START,
    };
    BShip_EventArray_Push(&match->events, game_start_event);

    BShip_Message_PlaceShips_Create(&match->temp_arena, &match->state->ai1.message,
        match->state->ai1.ship_lengths.buffer, match->state->ai1.ship_lengths.length);

    if (MatchState_SendMessages(match, true))
    {
        match->state->game_state = GAME_STATE_GAMES_OVER;
    }
    else
    {
        match->state->game_state = GAME_STATE_EXPECTING_SHIPS;
    }
}

void MatchState_AcceptShips(BShip_MatchData *match)
{
    if (MatchState_ReceiveMessages(match))
    {
        goto on_error;
    }

    match->ai1.error = BShip_Message_ShipsPlaced_Parse(&match->temp_arena, match->state->ai1.message,
        &match->state->ai1.ships, match->state->ai1.ship_lengths.length);
    match->ai2.error = BShip_Message_ShipsPlaced_Parse(&match->temp_arena, match->state->ai2.message,
        &match->state->ai2.ships, match->state->ai2.ship_lengths.length);
    if (HasErrored(match))
    {
        goto on_error;
    }
    assert(match->state->ai1.ships.length == match->state->ai1.ship_lengths.length);
    assert(match->state->ai2.ships.length == match->state->ai2.ship_lengths.length);

    size_t ship_count = match->state->ai1.ships.length;
    for (size_t i = 0; i < ship_count; i++)
    {
        match->ai1.error = ValidateAndStoreShip(match->state->ai1.board, match->state->ai1.ships.buffer[i],
            &match->state->ai1.ship_lengths);
        match->ai2.error = ValidateAndStoreShip(match->state->ai2.board, match->state->ai2.ships.buffer[i],
            &match->state->ai2.ship_lengths);
        if (HasErrored(match))
        {
            goto on_error;
        }
        BShip_U8Array_Push(&match->state->ai1.alive_ships, i);
        BShip_U8Array_Push(&match->state->ai2.alive_ships, i);

        BShip_Event ship_place_event = {
            .type = BSHIP_EVENT_SHIP_PLACEMENT,
            .value.compact.ai1_ship = BShip_CompactShip_From_Ship(match->state->ai1.ships.buffer[i]),
            .value.compact.ai2_ship = BShip_CompactShip_From_Ship(match->state->ai2.ships.buffer[i]),
        };
        BShip_EventArray_Push(&match->events, ship_place_event);
    }

    match->state->game_state = GAME_STATE_EXPECTING_SHOT;
    return;
on_error:
    match->state->game_state = GAME_STATE_GAMES_OVER;
}

BShip_Ship *AIMatchState_FindDeadShip(AIMatchState *state)
{
    uint32_t dead_ship_index = 0;
    if (FindAndStoreDeadShip(state->board, state->ships, &state->alive_ships, &state->dead_ships,
            &dead_ship_index))
    {
        return &state->ships.buffer[dead_ship_index];
    }
    return NULL;
}

void MatchState_AcceptShot(BShip_MatchData *match)
{
    if (MatchState_ReceiveMessages(match))
    {
        goto on_error;
    }

    BShip_Shot ai1_shot = {0}, ai2_shot = {0};
    match->ai1.error = BShip_Message_ShotTaken_Parse(&match->temp_arena, match->state->ai1.message, &ai1_shot);
    match->ai2.error = BShip_Message_ShotTaken_Parse(&match->temp_arena, match->state->ai2.message, &ai2_shot);
    if (HasErrored(match))
    {
        goto on_error;
    }

    // NOTE(mattg): Shots are stored in the opponent's board
    match->ai1.error = ValidateAndStoreShot(match->state->ai2.board, &ai1_shot);
    match->ai2.error = ValidateAndStoreShot(match->state->ai1.board, &ai2_shot);
    if (HasErrored(match))
    {
        goto on_error;
    }

    BShip_Ship *ai1_dead_ship = NULL, *ai2_dead_ship = NULL;
    if (ai2_shot.value == BSHIP_HIT)
    {
        match->state->ai2.hit_counter++;
        ai1_dead_ship = AIMatchState_FindDeadShip(&match->state->ai1);
    }
    if (ai1_shot.value == BSHIP_HIT)
    {
        match->state->ai1.hit_counter++;
        ai2_dead_ship = AIMatchState_FindDeadShip(&match->state->ai2);
    }


    BShip_Event shot_result_event = {
        .type = BSHIP_EVENT_SHOT_RESULT,
        .value.compact.ai1_shot = BShip_CompactShot_From_Shot(ai1_shot),
        .value.compact.ai2_shot = BShip_CompactShot_From_Shot(ai2_shot),
        .value.compact.ai1_ship = ai1_dead_ship != NULL ? BShip_CompactShip_From_Ship(*ai1_dead_ship) : 0,
        .value.compact.ai2_ship = ai2_dead_ship != NULL ? BShip_CompactShip_From_Ship(*ai2_dead_ship) : 0,
    };
    BShip_EventArray_Push(&match->events, shot_result_event);

    match->state->shot_counter++;

    match->state->next_shot = true;
    if (match->state->shot_counter >= match->state->game_shot_count_max ||
        match->state->ai1.alive_ships.length == 0 ||
        match->state->ai2.alive_ships.length == 0)
    {
        match->state->next_shot = false;
    }

    BShip_Message_ShotResult_Create(&match->temp_arena, &match->state->ai1.message, ai1_shot, ai2_shot,
        ai1_dead_ship, ai2_dead_ship, match->state->next_shot);
    if (MatchState_SendMessages(match, true))
    {
        goto on_error;
    }

    match->state->game_state = GAME_STATE_EXPECTING_SHOT;

    return;
on_error:
    match->state->game_state = GAME_STATE_GAMES_OVER;
}

void MatchState_CheckGameWinner(BShip_MatchData *match)
{
    assert(GameIsOver(match));

    BShip_GameResult ai1_game_result = BSHIP_WIN;
    // NOTE(mattg): game result precedence: (continue down the steps if both are equal)
    // 1. any error that occurs is a loss
    // 2. all ships are dead is a loss
    // 3. less alive ships than opponent is a loss
    // 4. fewer hits than opponent is a loss
    bool ai1_errored = match->ai1.error.type != ERROR_SUCCESS;
    bool ai2_errored = match->ai2.error.type != ERROR_SUCCESS;
    if      (ai1_errored && !ai2_errored) ai1_game_result = BSHIP_LOSS;
    else if (!ai1_errored && ai2_errored) ai1_game_result = BSHIP_WIN;
    else
    {
        size_t ai1_alive_count = match->state->ai1.alive_ships.length;
        size_t ai2_alive_count = match->state->ai2.alive_ships.length;
        bool ai1_has_ships = !!ai1_alive_count;
        bool ai2_has_ships = !!ai2_alive_count;
        if      (ai1_has_ships && !ai2_has_ships) ai1_game_result = BSHIP_WIN;
        else if (!ai1_has_ships && ai2_has_ships) ai1_game_result = BSHIP_LOSS;
        else if (ai1_alive_count > ai2_alive_count) ai1_game_result = BSHIP_WIN;
        else if (ai1_alive_count < ai2_alive_count) ai1_game_result = BSHIP_LOSS;
        else if (match->state->ai1.hit_counter > match->state->ai2.hit_counter) ai1_game_result = BSHIP_WIN;
        else if (match->state->ai1.hit_counter < match->state->ai2.hit_counter) ai1_game_result = BSHIP_LOSS;
        else ai1_game_result = BSHIP_TIE;
    }

    BShip_Event game_result_event = {
        .type = BSHIP_EVENT_GAME_RESULT,
        .value.ai1_game_result = ai1_game_result,
    };
    BShip_EventArray_Push(&match->events, game_result_event);
}

size_t BShip_Match_CalculateMemorySize(uint8_t board_size, uint32_t game_count)
{
    size_t ship_count_max = (size_t)ShipCountMax_From_BoardSize(board_size) * game_count;
    size_t shot_count_max = board_size * board_size * game_count;
    size_t event_count_max = ship_count_max + shot_count_max + (game_count * 2);
    size_t ai_name_mem_count = BSHIP_MESSAGE_NAME_SIZE_MAX * 4;
    return (sizeof(uint32_t) * game_count) + (sizeof(BShip_Event) * event_count_max) +
        (sizeof(uint8_t) * ai_name_mem_count);
}

BShip_Connection *BShip_Connection_Create(BShip_Arena *arena, char *socket_path)
{

    BSHIP_ARENA_TEMP_BEGIN(arena);
    BShip_Connection *conn = BShip_Arena_Push(arena, BShip_Connection_GetSize());
    if (conn != NULL)
    {
        if (!BShip_Connection_Open(conn, socket_path))
        {
            BSHIP_ARENA_TEMP_END(arena);
            conn = NULL;
        }
    }
    return conn;
}

void BShip_Connection_Destroy(BShip_Connection *conn)
{
    BShip_Connection_Close(conn);
}

bool BShip_Match_ShouldContinue(BShip_MatchData *match)
{
    return match != NULL && match->state != NULL && !HasErrored(match) &&
        match->state->game_state != GAME_STATE_GAMES_OVER;
}

BShip_MatchData BShip_Match_Initialize(BShip_Arena *arena,
    BShip_AIFileData ai1_file_data, BShip_AIFileData ai2_file_data,
    uint8_t board_size, uint32_t game_count, bool debug)
{
    BShip_MatchData match = {0};
    if (
        arena == NULL ||
        !BoardSize_Valid(board_size) ||
        !GameCount_Valid(game_count, board_size) ||
        ai1_file_data.file_path == NULL  || ai1_file_data.runtime_directory == NULL ||
        ai2_file_data.file_path == NULL  || ai2_file_data.runtime_directory == NULL ||
        !BShip_PathIsExecutable(ai1_file_data.file_path) ||
        !BShip_PathIsExecutable(ai2_file_data.file_path) ||
        !BShip_PathIsDirectory(ai1_file_data.runtime_directory) ||
        !BShip_PathIsDirectory(ai2_file_data.runtime_directory)
    )
    {
        goto on_error;
    }

    match.board_size = board_size;
    match.game_count = game_count;
    BShip_Arena_Initialize(&match.temp_arena, BSHIP_ARENA_BLOCK_SIZE_DEFAULT * 2);
    match.state = BShip_Arena_Push(&match.temp_arena, sizeof(BShip_MatchState));
    if (match.state == NULL)
    {
        goto on_error;
    }
    memset(match.state, 0, sizeof(BShip_MatchState));

    match.state->game_shot_count_max = match.board_size * match.board_size;
    match.state->debug = debug;
    match.state->ai1.file_data = ai1_file_data;
    match.state->ai2.file_data = ai2_file_data;

    // initialize all long-term values
    if (!BShip_LongTermValues_Initialize(arena, &match))
    {
        goto on_error;
    }

    // initialize all temp values
    match.state->start_timer = BShip_Arena_Push(&match.temp_arena, BShip_Timer_GetSize());
    match.state->end_timer = BShip_Arena_Push(&match.temp_arena, BShip_Timer_GetSize());
    if (match.state->start_timer == NULL || match.state->end_timer == NULL)
    {
        goto on_error;
    }
    if (!AIMatchState_PushTempValues(&match.temp_arena, &match.state->ai1, match.board_size) ||
        !AIMatchState_PushTempValues(&match.temp_arena, &match.state->ai2, match.board_size))
    {
        goto on_error;
    }

    return match;
on_error:
    if (match.state != NULL)
    {
        match.state->game_state = GAME_STATE_GAMES_OVER;
    }
    return match;
}

void BShip_Match_Start(BShip_MatchData *match, BShip_Connection *conn, char *socket_path)
{
    if (match == NULL || match->state == NULL || conn == NULL || socket_path == NULL)
    {
        goto on_error;
    }
    if (!BShip_Timer_Now(match->state->start_timer))
    {
        goto on_error;
    }

    match->ai1.error.type = BShip_AIConnection_StartProcess(match->state->ai1.conn, socket_path,
        match->state->ai1.file_data.file_path, match->state->ai1.file_data.runtime_directory);
    if (!BShip_Timer_In_Milliseconds(match->state->ai1.deadline, BSHIP_CONNECTION_TIMEOUT_MILLISECONDS))
    {
        goto on_error;
    }
    match->ai2.error.type = BShip_AIConnection_StartProcess(match->state->ai2.conn, socket_path,
        match->state->ai2.file_data.file_path, match->state->ai2.file_data.runtime_directory);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_CONNECTION_TIMEOUT_MILLISECONDS))
    {
        goto on_error;
    }
    if (HasErrored(match))
    {
        goto on_error;
    }

    match->ai1.error.type = BShip_AIConnection_Accept(match->state->ai1.conn, conn, match->state->ai1.deadline,
        match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai1.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        goto on_error;
    }
    match->ai2.error.type = BShip_AIConnection_Accept(match->state->ai2.conn, conn, match->state->ai2.deadline,
        match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS))
    {
        goto on_error;
    }
    if (HasErrored(match))
    {
        goto on_error;
    }

    if (MatchState_ReceiveMessages(match))
    {
        goto on_error;
    }

    match->ai1.error = BShip_Message_Hello_Parse(&match->temp_arena, match->state->ai1.message,
        match->ai1.name, match->ai1.authors);
    match->ai2.error = BShip_Message_Hello_Parse(&match->temp_arena, match->state->ai2.message,
        match->ai2.name, match->ai2.authors);
    if (HasErrored(match))
    {
        goto on_error;
    }
    match->ai1.ai_name_length = strnlen(match->ai1.name, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai1.author_name_length = strnlen(match->ai1.authors, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai2.ai_name_length = strnlen(match->ai2.name, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai2.author_name_length = strnlen(match->ai2.authors, BSHIP_MESSAGE_NAME_SIZE_MAX);

    BShip_Message_SetupMatch_Create(&match->temp_arena, &match->state->ai1.message,
        match->board_size, BSHIP_PLAYER_1);
    BShip_Message_SetupMatch_Create(&match->temp_arena, &match->state->ai2.message,
        match->board_size, BSHIP_PLAYER_2);

    if (MatchState_SendMessages(match, false))
    {
        goto on_error;
    }

    MatchState_PlaceShips(match);
    if (HasErrored(match) || match->state->game_state == GAME_STATE_GAMES_OVER)
    {
        goto on_error;
    }

    return;
on_error:
    if (match->state != NULL)
    {
        match->state->game_state = GAME_STATE_GAMES_OVER;
    }
}

void BShip_Match_Continue(BShip_MatchData *match)
{
    if (!BShip_Match_ShouldContinue(match))
    {
        goto on_end;
    }

    switch (match->state->game_state)
    {
    case GAME_STATE_EXPECTING_SHIPS:
        MatchState_AcceptShips(match);
        break;
    case GAME_STATE_EXPECTING_SHOT:
        MatchState_AcceptShot(match);
        break;
    case GAME_STATE_GAMES_OVER:
        assert(!BShip_Match_ShouldContinue(match));
        goto on_end;
        break;
    }
    if (GameIsOver(match))
    {
        MatchState_CheckGameWinner(match);
        if (match->game_indexes.length < match->game_count)
        {
            MatchState_PlaceShips(match);
        }
        else
        {
            match->state->game_state = GAME_STATE_GAMES_OVER;
        }
    }

    return;
on_end:
    match->state->game_state = GAME_STATE_GAMES_OVER;
}

void BShip_Match_Stop(BShip_MatchData *match, BShip_Arena *arena)
{
    if (match == NULL || match->state == NULL || arena == NULL)
    {
        return;
    }
    // TODO(mattg): Look at errors, if an AI has a message error, allocate a message buffer
    // and copy the message from the respective AI so we have it as an error.
    (void)arena;
    assert(match->state->game_state == GAME_STATE_GAMES_OVER);
    BShip_Message message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(&match->temp_arena, char, BSHIP_MESSAGE_SIZE),
    };
    if (message.buffer != NULL)
    {
        BShip_Message_MatchOver_Create(&match->temp_arena, &message);

        if (match->ai1.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(match->state->ai1.conn, message, match->state->ai1.deadline,
                match->state->debug);
        }
        if (match->ai2.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(match->state->ai2.conn, message, match->state->ai2.deadline,
                match->state->debug);
        }
    }

    BShip_AIConnection_Close(match->state->ai1.conn);
    BShip_AIConnection_Close(match->state->ai2.conn);

    if (!BShip_AIConnection_WaitProcess(match->state->ai1.conn, match->state->debug))
    {
        BShip_AIConnection_KillProcess(match->state->ai1.conn);
    }
    if (!BShip_AIConnection_WaitProcess(match->state->ai2.conn, match->state->debug))
    {
        BShip_AIConnection_KillProcess(match->state->ai2.conn);
    }

    if (BShip_Timer_Now(match->state->end_timer))
    {
        match->elapsed_time_ms = BShip_Timer_Difference_Milliseconds(match->state->start_timer, match->state->end_timer);
    }

    BShip_Arena_Destroy(&match->temp_arena);
}

BShip_MatchData BShip_HumanGame_Initialize(BShip_Arena *arena, BShip_AIFileData ai2_file_data,
    uint8_t board_size, bool debug)
{
    BShip_MatchData match = {0};
    if (
        arena == NULL ||
        !BoardSize_Valid(board_size) ||
        ai2_file_data.file_path == NULL || ai2_file_data.runtime_directory == NULL ||
        !BShip_PathIsExecutable(ai2_file_data.file_path) ||
        !BShip_PathIsDirectory(ai2_file_data.runtime_directory)
    )
    {
        goto on_error;
    }

    match.board_size = board_size;
    match.game_count = BShip_GameCountMin_From_BoardSize(board_size);
    BShip_Arena_Initialize(&match.temp_arena, BSHIP_ARENA_BLOCK_SIZE_DEFAULT * 2);
    match.state = BShip_Arena_Push(&match.temp_arena, sizeof(BShip_MatchState));
    if (match.state == NULL)
    {
        goto on_error;
    }
    memset(match.state, 0, sizeof(BShip_MatchState));

    match.state->game_shot_count_max = match.board_size * match.board_size;
    match.state->debug = debug;
    match.state->ai2.file_data = ai2_file_data;

    // initialize all long-term values
    if (!BShip_LongTermValues_Initialize(arena, &match))
    {
        goto on_error;
    }

    // initialize all temp values
    if (!AIMatchState_PushTempValues(&match.temp_arena, &match.state->ai1, match.board_size) ||
        !AIMatchState_PushTempValues(&match.temp_arena, &match.state->ai2, match.board_size))
    {
        goto on_error;
    }

    return match;
on_error:
    if (match.state != NULL)
    {
        match.state->game_state = GAME_STATE_GAMES_OVER;
    }
    return match;
}

void BShip_HumanGame_Start(BShip_MatchData *match, BShip_Connection *conn, char *socket_path,
    char *name, size_t name_length, char *authors, size_t authors_length)
{
    if (match == NULL || match->state == NULL || conn == NULL || socket_path == NULL)
    {
        goto on_error;
    }

    match->ai2.error.type = BShip_AIConnection_StartProcess(match->state->ai2.conn, socket_path,
        match->state->ai2.file_data.file_path, match->state->ai2.file_data.runtime_directory);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_CONNECTION_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    match->ai2.error.type = BShip_AIConnection_Accept(match->state->ai2.conn, conn, match->state->ai2.deadline,
        match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_CONNECTION_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    match->ai2.error.type = BShip_AIConnection_Receive(match->state->ai2.conn, &match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }
    if (name != NULL && name_length > 0)
    {
        memcpy(match->ai1.name, name,
            name_length < BSHIP_MESSAGE_NAME_SIZE_MAX ? name_length : BSHIP_MESSAGE_NAME_SIZE_MAX);
    }
    if (authors != NULL && authors_length > 0)
    {
        memcpy(match->ai1.authors, authors,
            authors_length < BSHIP_MESSAGE_NAME_SIZE_MAX ? authors_length : BSHIP_MESSAGE_NAME_SIZE_MAX);
    }

    match->ai2.error = BShip_Message_Hello_Parse(&match->temp_arena, match->state->ai2.message,
        match->ai2.name, match->ai2.authors);
    if (HasErrored(match))
    {
        goto on_error;
    }
    match->ai1.ai_name_length = strnlen(match->ai1.name, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai1.author_name_length = strnlen(match->ai1.authors, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai2.ai_name_length = strnlen(match->ai2.name, BSHIP_MESSAGE_NAME_SIZE_MAX);
    match->ai2.author_name_length = strnlen(match->ai2.authors, BSHIP_MESSAGE_NAME_SIZE_MAX);

    BShip_Message_SetupMatch_Create(&match->temp_arena, &match->state->ai2.message,
        match->board_size, BSHIP_PLAYER_2);

    match->ai2.error.type = BShip_AIConnection_Send(match->state->ai2.conn, match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    BShip_U32Array_Push(&match->game_indexes, match->events.length);

    match->state->next_shot = true;
    match->state->shot_counter = 0;
    match->state->ai1.hit_counter = 0;
    match->state->ai2.hit_counter = 0;

    match->state->ai1.ships.length = 0;
    match->state->ai2.ships.length = 0;
    match->state->ai1.ship_lengths.length = 0;
    match->state->ai2.ship_lengths.length = 0;
    match->state->ai1.alive_ships.length = 0;
    match->state->ai2.alive_ships.length = 0;
    match->state->ai1.dead_ships.length = 0;
    match->state->ai2.dead_ships.length = 0;

    size_t board_capacity = match->board_size * match->board_size;
    memset(match->state->ai1.board.buffer, (uint8_t)BSHIP_WATER, sizeof(uint8_t) * board_capacity);
    memset(match->state->ai2.board.buffer, (uint8_t)BSHIP_WATER, sizeof(uint8_t) * board_capacity);

    ShipLengths_Calculate(&match->state->ai1.ship_lengths, match->board_size);
    // NOTE(mattg): we need a copy of this to use when comparing ship lengths for both AIs.
    memcpy(match->state->ai2.ship_lengths.buffer, match->state->ai1.ship_lengths.buffer,
        match->state->ai1.ship_lengths.length * sizeof(uint8_t));
    match->state->ai2.ship_lengths.length = match->state->ai1.ship_lengths.length;

    BShip_Event game_start_event = {
        .type = BSHIP_EVENT_GAME_START,
    };
    BShip_EventArray_Push(&match->events, game_start_event);

    BShip_Message_PlaceShips_Create(&match->temp_arena, &match->state->ai2.message,
        match->state->ai2.ship_lengths.buffer, match->state->ai2.ship_lengths.length);

    match->ai2.error.type = BShip_AIConnection_Send(match->state->ai2.conn, match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    match->state->game_state = GAME_STATE_EXPECTING_SHIPS;
    return;
on_error:
    if (match->state != NULL)
    {
        match->state->game_state = GAME_STATE_GAMES_OVER;
    }
}

bool BShip_HumanGame_ShouldContinue(BShip_MatchData *match)
{
    return BShip_Match_ShouldContinue(match) && !GameIsOver(match);
}

BShip_U8Array BShip_HumanGame_GetShipLengths(BShip_MatchData *match)
{
    BShip_U8Array ship_lengths = {0};
    if (match == NULL || match->state == NULL || match->state->game_state != GAME_STATE_EXPECTING_SHIPS)
    {
        return ship_lengths;
    }
    assert(match->state->ai1.ship_lengths.buffer != NULL && match->state->ai1.ship_lengths.length > 0);

    size_t ship_lengths_size = match->state->ai1.ship_lengths.length * sizeof(uint8_t);
    ship_lengths.buffer = BSHIP_ARENA_PUSH_ARRAY(&match->temp_arena, uint8_t, ship_lengths_size);
    if (ship_lengths.buffer == NULL)
    {
        return ship_lengths;
    }
    ship_lengths.capacity = match->state->ai1.ship_lengths.length;
    memcpy(ship_lengths.buffer, match->state->ai1.ship_lengths.buffer, ship_lengths_size);
    ship_lengths.length = match->state->ai1.ship_lengths.length;
    return ship_lengths;
}

bool BShip_HumanGame_PlaceShips(BShip_MatchData *match, BShip_ShipArray ships)
{
    if (match == NULL || match->state == NULL || ships.buffer == NULL ||
        match->state->game_state != GAME_STATE_EXPECTING_SHIPS || HasErrored(match))
    {
        goto on_error;
    }
    match->ai2.error.type = BShip_AIConnection_Receive(match->state->ai2.conn, &match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    if (ships.length != match->state->ai1.ship_lengths.length)
    {
        match->ai1.error.type = ERROR_MESSAGE_SHIPS_PLACED_INVALID;
        goto on_error;
    }
    memcpy(match->state->ai1.ships.buffer, ships.buffer, sizeof(BShip_Ship) * ships.length);
    match->state->ai1.ships.length = ships.length;
    match->ai2.error = BShip_Message_ShipsPlaced_Parse(&match->temp_arena, match->state->ai2.message,
        &match->state->ai2.ships, match->state->ai2.ship_lengths.length);
    if (HasErrored(match))
    {
        goto on_error;
    }
    assert(match->state->ai1.ships.length == match->state->ai1.ship_lengths.length);
    assert(match->state->ai2.ships.length == match->state->ai2.ship_lengths.length);

    size_t ship_count = match->state->ai1.ships.length;
    for (size_t i = 0; i < ship_count; i++)
    {
        match->ai1.error = ValidateAndStoreShip(match->state->ai1.board, match->state->ai1.ships.buffer[i],
            &match->state->ai1.ship_lengths);
        match->ai2.error = ValidateAndStoreShip(match->state->ai2.board, match->state->ai2.ships.buffer[i],
            &match->state->ai2.ship_lengths);
        if (HasErrored(match))
        {
            goto on_error;
        }
        BShip_U8Array_Push(&match->state->ai1.alive_ships, i);
        BShip_U8Array_Push(&match->state->ai2.alive_ships, i);

        BShip_Event ship_place_event = {
            .type = BSHIP_EVENT_SHIP_PLACEMENT,
            .value.compact.ai1_ship = BShip_CompactShip_From_Ship(match->state->ai1.ships.buffer[i]),
            .value.compact.ai2_ship = BShip_CompactShip_From_Ship(match->state->ai2.ships.buffer[i]),
        };
        BShip_EventArray_Push(&match->events, ship_place_event);
    }

    match->state->game_state = GAME_STATE_EXPECTING_SHOT;
    return true;
on_error:
    match->state->game_state = GAME_STATE_GAMES_OVER;
    return false;
}

bool BShip_HumanGame_PlaceShot(BShip_MatchData *match, BShip_Shot shot)
{
    if (match == NULL || match->state == NULL ||
        match->state->game_state != GAME_STATE_EXPECTING_SHOT || HasErrored(match))
    {
        goto on_error;
    }
    match->ai2.error.type = BShip_AIConnection_Receive(match->state->ai2.conn, &match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    BShip_Shot ai1_shot = shot, ai2_shot = {0};
    match->ai2.error = BShip_Message_ShotTaken_Parse(&match->temp_arena, match->state->ai2.message, &ai2_shot);
    if (HasErrored(match))
    {
        goto on_error;
    }

    // NOTE(mattg): Shots are stored in the opponent's board
    match->ai1.error = ValidateAndStoreShot(match->state->ai2.board, &ai1_shot);
    match->ai2.error = ValidateAndStoreShot(match->state->ai1.board, &ai2_shot);
    if (HasErrored(match))
    {
        goto on_error;
    }

    BShip_Ship *ai1_dead_ship = NULL, *ai2_dead_ship = NULL;
    if (ai2_shot.value == BSHIP_HIT)
    {
        match->state->ai2.hit_counter++;
        ai1_dead_ship = AIMatchState_FindDeadShip(&match->state->ai1);
    }
    if (ai1_shot.value == BSHIP_HIT)
    {
        match->state->ai1.hit_counter++;
        ai2_dead_ship = AIMatchState_FindDeadShip(&match->state->ai2);
    }


    BShip_Event shot_result_event = {
        .type = BSHIP_EVENT_SHOT_RESULT,
        .value.compact.ai1_shot = BShip_CompactShot_From_Shot(ai1_shot),
        .value.compact.ai2_shot = BShip_CompactShot_From_Shot(ai2_shot),
        .value.compact.ai1_ship = ai1_dead_ship != NULL ? BShip_CompactShip_From_Ship(*ai1_dead_ship) : 0,
        .value.compact.ai2_ship = ai2_dead_ship != NULL ? BShip_CompactShip_From_Ship(*ai2_dead_ship) : 0,
    };
    BShip_EventArray_Push(&match->events, shot_result_event);

    match->state->shot_counter++;

    match->state->next_shot = true;
    if (match->state->shot_counter >= match->state->game_shot_count_max ||
        match->state->ai1.alive_ships.length == 0 ||
        match->state->ai2.alive_ships.length == 0)
    {
        match->state->next_shot = false;
    }

    BShip_Message_ShotResult_Create(&match->temp_arena, &match->state->ai2.message, ai1_shot, ai2_shot,
        ai1_dead_ship, ai2_dead_ship, match->state->next_shot);
    match->ai2.error.type = BShip_AIConnection_Send(match->state->ai2.conn, match->state->ai2.message,
        match->state->ai2.deadline, match->state->debug);
    if (!BShip_Timer_In_Milliseconds(match->state->ai2.deadline, BSHIP_MESSAGE_TIMEOUT_MILLISECONDS) ||
        HasErrored(match))
    {
        goto on_error;
    }

    if (match->state->next_shot)
    {
        match->state->game_state = GAME_STATE_EXPECTING_SHOT;
    }
    else
    {
        match->state->game_state = GAME_STATE_GAMES_OVER;
    }
    return true;
on_error:
    match->state->game_state = GAME_STATE_GAMES_OVER;
    return false;
}

bool BShip_HumanGame_Stop(BShip_MatchData *match, BShip_Arena *arena)
{
    if (match == NULL || match->state == NULL)
    {
        return false;
    }
    if (GameIsOver(match))
    {
        MatchState_CheckGameWinner(match);
    }
    else
    {
        BShip_Event game_result_event = {
            .type = BSHIP_EVENT_GAME_RESULT,
            .value.ai1_game_result = BSHIP_LOSS,
        };
        BShip_EventArray_Push(&match->events, game_result_event);
    }

    // TODO(mattg): Look at errors, if an AI has a message error, allocate a message buffer
    // and copy the message from the respective AI so we have it as an error.
    (void)arena;
    BShip_Message message = {
        .buffer = BSHIP_ARENA_PUSH_ARRAY(&match->temp_arena, char, BSHIP_MESSAGE_SIZE),
    };
    if (message.buffer != NULL)
    {
        BShip_Message_MatchOver_Create(&match->temp_arena, &message);
        if (match->ai2.error.type == ERROR_SUCCESS)
        {
            BShip_AIConnection_Send(match->state->ai2.conn, message, match->state->ai2.deadline,
                match->state->debug);
        }
    }

    BShip_AIConnection_Close(match->state->ai2.conn);

    if (!BShip_AIConnection_WaitProcess(match->state->ai2.conn, match->state->debug))
    {
        BShip_AIConnection_KillProcess(match->state->ai2.conn);
    }

    BShip_Arena_Destroy(&match->temp_arena);
    return true;
}
