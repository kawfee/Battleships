/**
 * @file unix.c
 * @authors Matthew Getgen
 * @brief Battleships Platform-specific Linux/macOS Code
 * @date 2026-05-12
 *
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/split/unixsock.html)
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"

#define BSHIP_TIMEOUT_SECONDS 0
#define BSHIP_TIMEOUT_MILLISECONDS 500

struct BShip_Connection {
    struct sockaddr_un socket_address;
    int32_t socket_desc;
};

struct BShip_AIConnection {
    int32_t socket_desc;
    int32_t exit_status;
    pid_t process_id;
};

void *BShip_Allocate(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        PRINT_ERROR(strerror(errno));
    }
    return ptr;
}

void BShip_Deallocate(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}

size_t BShip_Connection_GetSize(void)
{
    return (size_t)sizeof(BShip_Connection);
}

size_t BShip_AIConnection_GetSize(void)
{
    return (size_t)sizeof(BShip_AIConnection);
}

bool BShip_Connection_Create(BShip_Connection *conn, const char *socket_path)
{
    assert(conn != NULL);
    assert(socket_path != NULL);

    socklen_t socket_address_length = sizeof(conn->socket_address.sun_path);

    conn->socket_address.sun_family = AF_UNIX;
    memset(&conn->socket_address.sun_path, 0, socket_address_length);

    conn->socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (conn->socket_desc == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    {
        uint32_t socket_path_length = strlen(socket_path);
        if (socket_path_length > socket_address_length - 1)
        {
            PRINT_ERROR("socket_path is too large!");
            goto on_error;
        }
    }
    strncpy(conn->socket_address.sun_path, socket_path, socket_address_length-1);
    unlink(conn->socket_address.sun_path); // NOTE(mattg): destroy any socket of the same name if it already exists

    if (bind(conn->socket_desc, (struct sockaddr *)&conn->socket_address, socket_address_length) == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
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
    assert(ai_path != NULL);
    assert (socket_path != NULL);

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

bool BShip_AIConnection_WaitProcess(BShip_AIConnection *ai_conn, bool debug)
{
    if (ai_conn == NULL)
    {
        return false;
    }
    if (ai_conn->process_id == -1) {
        ai_conn->process_id = 0;
        return false;
    }
    int status = 0;

    if (debug)
    {
        waitpid(ai_conn->process_id, &status, 0);
        return true;
    }
    struct timespec sleep_time = {
        .tv_sec = 0,
        .tv_nsec = 5 * 1000 * 1000, // 5 ms
    };

    for (int i = 0; i < 100; i++)
    {
        pid_t result = waitpid(ai_conn->process_id, &status, WNOHANG);
        switch (result)
        {
        case -1:
            PRINT_ERROR(strerror(errno));
            return false;
            break;
        default:
            if (result == ai_conn->process_id)
            {
                return true;
            }
        }

        nanosleep(&sleep_time, NULL);
    }

    return false;
}

void BShip_AIConnection_KillProcess(BShip_AIConnection *ai_conn)
{
    if (ai_conn == NULL)
    {
        return;
    }
    PRINT_ERROR("AI has not exited, killing...");
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
        return ERROR_CONNECTION_FAILED;
    }
    
    if (!debug)
    {
        struct pollfd pfd = {
            .fd = conn->socket_desc,
            .events = POLLIN,
        };
        int rc = poll(&pfd, 1, BSHIP_TIMEOUT_MILLISECONDS);
        switch (rc)
        {
        case -1:
            PRINT_ERROR(strerror(errno));
            return ERROR_CONNECTION_FAILED;
            break;
        case 0:
            PRINT_ERROR("Waiting for an AI to connect timed out!");
            return ERROR_CONNECTION_TIMEOUT;
            break;
        default:
            break;
        }
    }
    struct sockaddr_un socket_address = {
        .sun_family = AF_UNIX,
    };
    socklen_t socket_address_length = sizeof(socket_address);
    ai_conn->socket_desc = accept(conn->socket_desc, (struct sockaddr *)&socket_address, &socket_address_length);
    if (ai_conn->socket_desc == -1) {
        PRINT_ERROR(strerror(errno));
        return ERROR_CONNECTION_FAILED;
    }

    return ERROR_SUCCESS;
}

BShip_ErrorType BShip_AIConnection_Send(BShip_AIConnection *ai_conn, BShip_Message message, bool debug)
{
    assert(ai_conn != NULL);
    assert(message.buffer != NULL);
    
    if (!debug)
    {
        struct pollfd pfd = {
            .fd = ai_conn->socket_desc,
            .events = POLLOUT,
        };
        int rc = poll(&pfd, 1, BSHIP_TIMEOUT_MILLISECONDS);
        switch (rc)
        {
        case -1:
            PRINT_ERROR(strerror(errno));
            return ERROR_SEND_FAILED;
            break;
        case 0:
            PRINT_ERROR("Waiting to send a message to the AI timed out!");
            return ERROR_SEND_TIMEOUT;
            break;
        default:
            break;
        }
    }
    if (send(ai_conn->socket_desc, message.buffer, BSHIP_MESSAGE_SIZE, 0) == -1)
    {
        PRINT_ERROR(strerror(errno));
        return ERROR_SEND_FAILED;
    }
    return ERROR_SUCCESS;
}

BShip_ErrorType BShip_AIConnection_Receive(BShip_AIConnection *ai_conn, BShip_Message *message, bool debug)
{
    assert(ai_conn != NULL);
    assert(message != NULL);
    assert(message->buffer != NULL);

    if (!debug)
    {
        struct pollfd pfd = {
            .fd = ai_conn->socket_desc,
            .events = POLLIN,
        };
        int rc = poll(&pfd, 1, BSHIP_TIMEOUT_MILLISECONDS);
        switch (rc)
        {
        case -1:
            PRINT_ERROR(strerror(errno));
            return ERROR_RECEIVE_FAILED;
            break;
        case 0:
            PRINT_ERROR("Waiting on a message from the AI timed out!");
            return ERROR_RECEIVE_TIMEOUT;
            break;
        default:
            break;
        }
    }

    int bytes_received = recv(ai_conn->socket_desc, message->buffer, BSHIP_MESSAGE_SIZE, 0);
    switch (bytes_received)
    {
    case -1:
        PRINT_ERROR(strerror(errno));
        return ERROR_RECEIVE_FAILED;
        break;
    case 0:
        // received an empty message, usually indicated an early exited AI...
        PRINT_ERROR("Empty message received from the AI!");
        return ERROR_RECEIVE_EMPTY_MESSAGE;
        break;
    default:
        break;
    }
    message->length = strnlen(message->buffer, BSHIP_MESSAGE_SIZE);
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

