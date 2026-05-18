/**
 * @file platform.h
 * @author Matthew Getgen
 * @brief Platform-defined functions that need to be supported on all applications for the library to function.
 * @date 2026-05-12
 */

#ifndef BSHIP_PLATFORM_H
#define BSHIP_PLATFORM_H

#include "../battleshipslib.h"

typedef struct BShip_Connection BShip_Connection;

typedef struct BShip_AIConnection BShip_AIConnection;

BShip_Connection *BShip_Connection_Allocate(void);

void BShip_Connection_Deallocate(BShip_Connection *conn);

BShip_AIConnection *BShip_AIConnection_Allocate(void);

void BShip_AIConnection_Deallocate(BShip_AIConnection *ai_conn);

int32_t BShip_Connection_Create(BShip_Connection *conn, const char *socket_path, bool debug);

void BShip_Connection_Close(BShip_Connection *conn);

BShip_ErrorType BShip_AIConnection_Accept(BShip_AIConnection *ai_conn, BShip_Connection *conn, bool debug);

void BShip_AIConnection_Close(BShip_AIConnection *conn);


BShip_ErrorType BShip_AIConnection_StartProcess(BShip_AIConnection *ai_conn, const char *ai_path, const char *socket_path);

void BShip_AIConnection_WaitProcess(BShip_AIConnection *ai_conn);

void BShip_AIConnection_KillProcess(BShip_AIConnection *ai_conn);


BShip_ErrorType BShip_AIConnection_Send(BShip_AIConnection *ai_conn, uint8_t *message, uint32_t size);

BShip_ErrorType BShip_AI_Connection_Receive(BShip_AIConnection *ai_conn, uint8_t *message, uint32_t size);


#endif // BSHIP_PLATFORM_H
