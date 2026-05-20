/**
 * @file platform.h
 * @author Matthew Getgen
 * @brief Platform-defined functions that need to be supported on all applications for the library to function.
 * @date 2026-05-12
 */

#ifndef BSHIP_PLATFORM_H
#define BSHIP_PLATFORM_H

#include "../battleshipslib.h"

// NOTE(mattg): Also be sure to implement BShip_Arena_Initialize and BShip_Arena_Destroy.
// They are exposed via the library header, which is why they aren't here.

#define BSHIP_ARENA_TEMP_BEGIN(arena) \
    BShip_ArenaMark __mark = BShip_ArenaMark_Get(arena);

#define BSHIP_ARENA_TEMP_END(arena) \
    BShip_Arena_Rollback(arena, __mark);

void *BShip_Arena_Push(BShip_Arena *arena, size_t size);

void BShip_Arena_Reset(BShip_Arena *arena);

BShip_ArenaMark BShip_ArenaMark_Get(BShip_Arena *arena);

void BShip_Arena_Rollback(BShip_Arena *arena, BShip_ArenaMark mark);


typedef struct BShip_Connection BShip_Connection;

typedef struct BShip_AIConnection BShip_AIConnection;

BShip_Connection *BShip_Connection_Allocate(BShip_Arena *arena);

BShip_AIConnection *BShip_AIConnection_Allocate(BShip_Arena *arena);

int32_t BShip_Connection_Create(BShip_Connection *conn, const char *socket_path, bool debug);

void BShip_Connection_Close(BShip_Connection *conn);

BShip_ErrorType BShip_AIConnection_Accept(BShip_AIConnection *ai_conn, BShip_Connection *conn, bool debug);

void BShip_AIConnection_Close(BShip_AIConnection *conn);


BShip_ErrorType BShip_AIConnection_StartProcess(BShip_AIConnection *ai_conn, const char *ai_path, const char *socket_path);

bool BShip_AIConnection_WaitProcess(BShip_AIConnection *ai_conn);

void BShip_AIConnection_KillProcess(BShip_AIConnection *ai_conn);


BShip_ErrorType BShip_AIConnection_Send(BShip_AIConnection *ai_conn, BShip_Message message);

BShip_ErrorType BShip_AIConnection_Receive(BShip_AIConnection *ai_conn, BShip_Message *message);


#endif // BSHIP_PLATFORM_H
