/**
 * @file linux.c
 * @authors Matthew Getgen
 * @brief Battleships Platform-specific Linux Code
 * @date 2026-05-12
 *
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/split/unixsock.html)
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "platform.h"

#define BSHIP_TIMEOUT_SECONDS 0
#define BSHIP_TIMEOUT_MICROSECONDS 500000 // 0.5 seconds

struct BShip_Connection {
    struct sockaddr_un socket_address;
    int32_t socket_desc;
};

struct BShip_AIConnection {
    int32_t socket_desc;
    int32_t exit_status;
    pid_t process_id;
};

// static inline size_t BShip_Arena_AlignForward(size_t ptr, size_t alignment)
// {
//     size_t modulo = ptr & (alignment - 1);
//
//     if (modulo != 0)
//     {
//         ptr += alignment - modulo;
//     }
//     return p;
// }

static inline BShip_ArenaBlock *BShip_ArenaBlock_Allocate(size_t size)
{
    BShip_ArenaBlock *block = malloc(size);
    if (block == NULL)
    {
        PRINT_ERROR(strerror(errno));
        return block;
    }
    block->previous = NULL;
    block->capacity = size - sizeof(BShip_ArenaBlock);
    block->offset = 0;
    return block;
}

void BShip_Arena_Initialize(BShip_Arena *arena, size_t capacity)
{
    if (arena == NULL)
    {
        return;
    }
    capacity += sizeof(BShip_ArenaBlock);
    size_t size = BSHIP_ARENA_BLOCK_SIZE_DEFAULT;
    while (capacity > size)
    {
        size += size;
    }
    BShip_ArenaBlock *block = BShip_ArenaBlock_Allocate(size);
    arena->first = block;
    arena->current = block;
}

void BShip_Arena_Destroy(BShip_Arena *arena)
{
    if (arena == NULL)
    {
        return;
    }
    BShip_ArenaBlock *block = arena->current;
    while (block != NULL)
    {
        BShip_ArenaBlock *next = block->previous;
        free(block);
        block = next;
    }
    arena->first = NULL;
    arena->current = NULL;
}

void *BShip_Arena_Push(BShip_Arena *arena, size_t size)
{

    if (arena == NULL || arena->first == NULL || arena->current == NULL)
    {
        return NULL;
    }

    size_t space = arena->current->capacity - arena->current->offset;
    if (space >= size)
    {
        void *memory = &arena->current->memory[arena->current->offset];
        arena->current->offset += size;
        return memory;
    }
    size_t new_block_capacity = arena->current->capacity * 2;
    while ((size + sizeof(BShip_ArenaBlock)) > new_block_capacity)
    {
        new_block_capacity += new_block_capacity;
    }
    BShip_ArenaBlock *block = BShip_ArenaBlock_Allocate(new_block_capacity);
    if (block == NULL)
    {
        return block;
    }
    block->previous = arena->current;
    block->offset += size;
    arena->current = block;
    return block->memory;
}

void BShip_Arena_Reset(BShip_Arena *arena)
{
    assert(arena != NULL);
    BShip_ArenaBlock *block = arena->current;
    while (block != NULL && arena->first != block)
    {
        BShip_ArenaBlock *next = block->previous;
        free(block);
        block = next;
    }
    arena->current = arena->first;
    arena->first->offset = 0;
}

BShip_ArenaMark BShip_ArenaMark_Get(BShip_Arena *arena)
{
    assert(arena != NULL);
    BShip_ArenaMark mark = {
        .block = arena->current,
        .offset = arena->current->offset,
    };
    return mark;
}

void BShip_Arena_Rollback(BShip_Arena *arena, BShip_ArenaMark mark)
{
    assert(arena != NULL);
    assert(mark.block != NULL);

    BShip_ArenaBlock *block = arena->current;
    while (block != mark.block)
    {
        BShip_ArenaBlock *previous = block->previous;
        free(block);
        block = previous;
    }
    block->offset = mark.offset;
    arena->current = block;
}

BShip_Connection *BShip_Connection_Allocate(BShip_Arena *arena)
{
    return BShip_Arena_Push(arena, sizeof(BShip_Connection));
}

BShip_AIConnection *BShip_AIConnection_Allocate(BShip_Arena *arena)
{
    return BShip_Arena_Push(arena, sizeof(BShip_AIConnection));
}

bool BShip_Connection_Create(BShip_Connection *conn, const char *socket_path, bool debug)
{
    assert(conn != NULL);
    assert(socket_path != NULL);
    int32_t sun_path_size = sizeof(conn->socket_address.sun_path);
    conn->socket_address.sun_family = AF_UNIX;
    memset(&conn->socket_address.sun_path, 0, sun_path_size);

    conn->socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (conn->socket_desc == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    int32_t socket_path_length = strlen(socket_path);
    if (socket_path_length > sun_path_size - 1)
    {
        PRINT_ERROR("socket_path is too large!");
        goto on_error;
    }
    memcpy(conn->socket_address.sun_path, socket_path, socket_path_length-1);
    unlink(conn->socket_address.sun_path); // NOTE(mattg): destroy any socket of the same name if it already exists
    
    socklen_t socket_address_length = sizeof(conn->socket_address);

    if (bind(conn->socket_desc, (struct sockaddr *)&conn->socket_address, socket_address_length) == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    struct timeval tv = {
        .tv_sec = BSHIP_TIMEOUT_SECONDS,
        .tv_usec = BSHIP_TIMEOUT_MICROSECONDS,
    };

    // NOTE(mattg): Don't set a timer if we want to debug the code.
    if (!debug)
    {
        if (setsockopt(conn->socket_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }
    }

    return true;
on_error:
    BShip_Connection_Close(conn);
    return false;
}

void BShip_Connection_Close(BShip_Connection *conn)
{
    if (conn == NULL)
    {
        return;
    }
    if (conn->socket_desc > 2) // NOTE(mattg): -1 - socket() error, 0 - stdin, 1 - stdout, 2 - stderr
    {
        close(conn->socket_desc);
    }
    unlink(conn->socket_address.sun_path);
    memset(conn, 0, sizeof(BShip_Connection));
}

BShip_ErrorType BShip_AIConnection_StartProcess(BShip_AIConnection *ai_conn, const char *ai_path, const char *socket_path)
{
    assert(ai_conn != NULL);
    struct stat filestat = {0};
    switch (stat(ai_path, &filestat))
    {
    case -1:
        PRINT_ERROR(strerror(errno));
        return ERROR_AI_PATH_ISSUE;
        break;
    case 0:
    default:
        // NOTE(mattg): check if it's a regular file, and if the user has execute permissions
        if (!S_ISREG(filestat.st_mode) || !(filestat.st_mode & S_IXUSR))
        {
            PRINT_ERROR_F("AI file %s is not an executable!", ai_path);
            return ERROR_AI_PATH_ISSUE;
        }
        break;
    }

    char *argv[] = {(char *)ai_path, (char *)socket_path, NULL};
    ai_conn->process_id = fork();

    switch (ai_conn->process_id)
    {
    case 0: // child process
        // allow CTRL-C to kill the child process.
        signal(SIGINT, SIG_DFL);

        if (execv(ai_path, argv) == -1)
        {
            PRINT_ERROR(strerror(errno));
            exit(1); // just exit the child process.
        }
        break;
    case -1: // error
        PRINT_ERROR(strerror(errno));
        BShip_AIConnection_KillProcess(ai_conn);
        return ERROR_PROCESS_FAILED;
        break;
    default:
        break;
    }

    return ERROR_SUCCESS;
}

bool BShip_AIConnection_WaitProcess(BShip_AIConnection *ai_conn)
{
    if (ai_conn == NULL)
    {
        return false;
    }
    if (ai_conn->process_id == -1) {
        ai_conn->process_id = 0;
        return false;
    }
    struct timeval now, end = {0};
    gettimeofday(&now, NULL);
    end.tv_sec = now.tv_sec + BSHIP_TIMEOUT_SECONDS;
    end.tv_usec = now.tv_usec + BSHIP_TIMEOUT_MICROSECONDS;

    int status = 0;
    do
    {
        waitpid(ai_conn->process_id, &status, WNOHANG);
        if (WIFEXITED(status))
        {
            ai_conn->exit_status = WEXITSTATUS(status);
            if (ai_conn->exit_status != 0)
            {
                PRINT_ERROR_F("AI exited with non-zero status code: %d", ai_conn->exit_status);
            }
            return true;
            break;
        }
        gettimeofday(&now, NULL);
    } while (now.tv_sec <= end.tv_sec && now.tv_usec <= end.tv_usec);

    return false;
}

void BShip_AIConnection_KillProcess(BShip_AIConnection *ai_conn)
{
    if (ai_conn == NULL)
    {
        return;
    }
    if (ai_conn->process_id != -1 && ai_conn->process_id != 0)
    {
        kill(ai_conn->process_id, SIGKILL);
        waitpid(ai_conn->process_id, NULL, 0);
    }
    ai_conn->process_id = 0;
}

BShip_ErrorType BShip_AIConnection_Accept(BShip_AIConnection *ai_conn, BShip_Connection *conn, bool debug)
{
    assert(conn != NULL);
    assert(ai_conn != NULL);
    if (listen(conn->socket_desc, 1) == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    struct sockaddr_un socket_address = {
        .sun_family = AF_UNIX,
    };
    socklen_t socket_address_length = sizeof(socket_address);
    ai_conn->socket_desc = accept(conn->socket_desc, (struct sockaddr *)&socket_address, &socket_address_length);
    if (ai_conn->socket_desc == -1) {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    struct timeval tv = {
        .tv_sec = BSHIP_TIMEOUT_SECONDS,
        .tv_usec = BSHIP_TIMEOUT_MICROSECONDS,
    };
    // NOTE(mattg): Don't set a timer if we want to debug the code.
    if (!debug)
    {
        if (setsockopt(ai_conn->socket_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }
    }

    return ERROR_SUCCESS;
on_error:
    BShip_AIConnection_Close(ai_conn);
    return ERROR_CONNECTION_FAILED;
}

BShip_ErrorType BShip_AIConnection_Send(BShip_AIConnection *ai_conn, BShip_Message message)
{
    assert(ai_conn != NULL);
    assert(message.json != NULL);
    if (send(ai_conn->socket_desc, message.json, message.length, 0) == -1) // TODO(mattg): test with MSG_DONTWAIT for non-blocking I/O
    {
        PRINT_ERROR(strerror(errno));
        return ERROR_SEND_FAILED;
    }
    return ERROR_SUCCESS;
}

BShip_ErrorType BShip_AIConnection_Receive(BShip_AIConnection *ai_conn, BShip_Message *message)
{
    assert(ai_conn != NULL);
    assert(message != NULL);
    assert(message->json != NULL);
    memset(message->json, 0, message->capacity);
    int bytes_received = recv(ai_conn->socket_desc, message->json, message->capacity, 0);
    switch (bytes_received)
    {
    case -1:
        PRINT_ERROR(strerror(errno));
        return ERROR_RECEIVE_FAILED;
        break;
    case 0:
        // received an empty message, usually indicated an early exited AI...
        PRINT_ERROR("Failed to receive a message from the AI!");
        return ERROR_RECEIVE_FAILED;
        break;
    default:
        message->length = bytes_received;
        break;
    }
    return ERROR_SUCCESS;
}

void BShip_AIConnection_Close(BShip_AIConnection *ai_conn)
{
    if (ai_conn->socket_desc > 2) // NOTE(mattg): -1 == socket() error, 0 == stdin, 1 == stdout, 2 == stderr
    {
        close(ai_conn->socket_desc);
    }
    ai_conn->socket_desc = 0;
}

