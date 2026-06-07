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
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
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

bool BShip_PathIsExecutable(char *path)
{
    struct stat statbuf = {0};
    if (stat(path, &statbuf) == -1)
    {
        PRINT_ERROR(strerror(errno));
        return false;
    }
    return (S_ISREG(statbuf.st_mode) && (statbuf.st_mode & S_IXUSR));
}

bool BShip_PathIsDirectory(char *path)
{
    struct stat statbuf = {0};
    if (stat(path, &statbuf) == -1)
    {
        PRINT_ERROR(strerror(errno));
        return false;
    }
    return (S_ISDIR(statbuf.st_mode));
}

size_t BShip_Connection_GetSize(void)
{
    return (size_t)sizeof(BShip_Connection);
}

size_t BShip_AIConnection_GetSize(void)
{
    return (size_t)sizeof(BShip_AIConnection);
}

bool BShip_Connection_Create(BShip_Connection *conn, char *socket_path)
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

    int flags = fcntl(conn->socket_desc, F_GETFD);
    if (flags == -1)
    {
        PRINT_ERROR(strerror(errno));
        goto on_error;
    }

    if (fcntl(conn->socket_desc, F_SETFD, flags | FD_CLOEXEC) == -1)
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

bool set_resource_limit(int resource, rlim_t soft_limit, rlim_t hard_limit)
{
    struct rlimit limit = {
        .rlim_cur = soft_limit,
        .rlim_max = hard_limit,
    };
    if (setrlimit(resource, &limit) == -1)
    {
        PRINT_ERROR(strerror(errno));
        return false;
    }
    return true;
}

BShip_ErrorType BShip_AIConnection_StartProcess(BShip_AIConnection *ai_conn, char *socket_path,
    char *ai_path, char *ai_dir)
{
    assert(ai_conn != NULL);
    assert(ai_path != NULL);
    assert (socket_path != NULL);

    if (!BShip_PathIsExecutable(ai_path))
    {
        PRINT_ERROR_F("AI file %s is not an executable!", ai_path);
        return ERROR_AI_PATH_ISSUE;
    }

    if (!BShip_PathIsDirectory(ai_dir))
    {
        PRINT_ERROR_F("AI directory %s is not a directory!", ai_dir);
        return ERROR_AI_PATH_ISSUE;
    }

    ai_conn->process_id = fork();

    if (ai_conn->process_id == 0)
    {
        // child process
 
        // allow CTRL-C to kill the child process.
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }

        // PROCESS RESTRICTION - security surrounding child processes in UNIX/POSIX systems.
        // 1. Manage process user and group permissions.
        if (setpgid(0, 0) == -1)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }

        // 2. Set system resource limits.
        if (!set_resource_limit(RLIMIT_NPROC, 20, 20))
        {
            goto on_error;
        }
        if (!set_resource_limit(RLIMIT_NOFILE, 64, 64))
        {
            goto on_error;
        }
        rlim_t file_size_limit = 10 * 1024 * 1024;
        if (!set_resource_limit(RLIMIT_FSIZE, file_size_limit, file_size_limit))
        {
            goto on_error;
        }

        // 3. Run the AIs in separate directories, so that AIs don't accidentially edit other files.
        char home_env_start[] = "HOME=";
        size_t home_env_start_length = strlen(home_env_start);
        size_t ai_dir_length = strlen(ai_dir);
        size_t home_env_size = home_env_start_length + ai_dir_length + 1;

        char *home_env = BShip_Allocate(home_env_size);
        memset(home_env, 0, home_env_size);

        memcpy(home_env, home_env_start, home_env_start_length);
        memcpy(&home_env[home_env_start_length], ai_dir, ai_dir_length);
        if (chdir(ai_dir) == -1)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }
        // 4. Close unwanted file descriptors.
        // NOTE(mattg): This one is handled elsewhere, look for FD_CLOEXEC
        // 5. Restrict ENV variables to just the basics.
        char *argv[] = {
            (char *)ai_path,
            (char *)socket_path,
            NULL
        };
        char *envp[] = {
            "PATH=/usr/bin:/bin",
            home_env, // created from the ai_dir calculation
            "TMPDIR=/tmp",
            NULL
        };
        
        if (execve(ai_path, argv, envp) == -1)
        {
            PRINT_ERROR(strerror(errno));
            goto on_error;
        }
on_error:
        _exit(1); // just exit the child process.
    }
    else if (ai_conn->process_id == -1)
    {
        // fork error
        PRINT_ERROR(strerror(errno));
        BShip_AIConnection_KillProcess(ai_conn);
        return ERROR_PROCESS_FAILED;
    }
    // server, return
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
        if (kill(-(ai_conn->process_id), SIGKILL) == -1)
        {
            PRINT_ERROR(strerror(errno));
        }
        if (waitpid(ai_conn->process_id, NULL, 0) == -1)
        {
            PRINT_ERROR(strerror(errno));
        }
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

    int flags = fcntl(ai_conn->socket_desc, F_GETFD);
    if (flags == -1)
    {
        PRINT_ERROR(strerror(errno));
        return ERROR_CONNECTION_FAILED;
    }

    if (fcntl(ai_conn->socket_desc, F_SETFD, flags | FD_CLOEXEC) == -1)
    {
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

