/**
 * @file platform.h
 * @author Matthew Getgen
 * @brief Platform-defined functions that need to be supported on all applications for the library to function.
 * @date 2026-05-12
 */

#ifndef BSHIP_PLATFORM_H
#define BSHIP_PLATFORM_H

#include "../battleshipslib.h"


void *BShip_Allocate(size_t size);

void BShip_Deallocate(void *ptr);

bool BShip_PathIsFile(char *path);

bool BShip_PathIsExecutable(char *path);

bool BShip_PathIsDirectory(char *path);

ssize_t BShip_File_GetSize(char *path);

bool BShip_File_Read(char *path, BShip_Buffer *buffer, size_t size);

bool BShip_File_Write(char *path, BShip_Buffer *buffer);

typedef struct BShip_Timer BShip_Timer;

size_t BShip_Timer_GetSize(void);

bool BShip_Timer_Now(BShip_Timer *timer);

int BShip_Timer_Compare(BShip_Timer *a, BShip_Timer *b);

void BShip_Timer_Add_Nanoseconds(BShip_Timer *timer, uint64_t nsec);

void BShip_Timer_Add_Milliseconds(BShip_Timer *timer, uint64_t msec);

bool BShip_Timer_In_Nanoseconds(BShip_Timer *timer, uint64_t nsec);

bool BShip_Timer_In_Milliseconds(BShip_Timer *timer, uint64_t msec);

typedef struct BShip_AIConnection BShip_AIConnection;

size_t BShip_Connection_GetSize(void);

size_t BShip_AIConnection_GetSize(void);

bool BShip_Connection_Open(BShip_Connection *conn, char *socket_path);

void BShip_Connection_Close(BShip_Connection *conn);

BShip_ErrorType BShip_AIConnection_StartProcess(BShip_AIConnection *ai_conn, char *socket_path,
    char *ai_path, char *ai_dir);

bool BShip_AIConnection_WaitProcess(BShip_AIConnection *ai_conn, bool debug);

void BShip_AIConnection_KillProcess(BShip_AIConnection *ai_conn);

BShip_ErrorType BShip_AIConnection_Accept(BShip_AIConnection *ai_conn, BShip_Connection *conn,
    BShip_Timer *deadline, bool debug);

BShip_ErrorType BShip_AIConnection_Send(BShip_AIConnection *ai_conn, BShip_Message message,
    BShip_Timer *deadline, bool debug);

BShip_ErrorType BShip_AIConnection_Receive(BShip_AIConnection *ai_conn, BShip_Message *message,
    BShip_Timer *deadline, bool debug);

void BShip_AIConnection_Close(BShip_AIConnection *conn);


#endif // BSHIP_PLATFORM_H
