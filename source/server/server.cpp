/**
 * @file server.cpp
 * @authors Matthew Getgen
 * @brief Battleships Server C++ file
 * @date 2023-08-27
 * 
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/multi/unixsock.html)
 */

#include "server.h"
#include <cstdlib>


/* ─────────────────────────── *
 * SOCKET CONNECTION FUNCTIONS *
 * ─────────────────────────── */

Connection create_socket(const char *socket_name) {
    Connection connect;
    memset(&connect, 0, sizeof(connect));
    if ( bind_socket(connect, socket_name) != 0 ) {
        close_sockets(connect);
        cout << "\nExiting.\n" << flush;
        exit(EXIT_FAILURE);
    }
    return connect;
}

int bind_socket(Connection &connect, const char *socket_name) {
    socklen_t socket_len;
    size_t name_len;
    timeval tv;

    // set timer for server socket.
    tv.tv_sec   = SECONDS;
    tv.tv_usec  = MICROSECONDS;   // half a second, very generous.

    // define the socket
    connect.server_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( connect.server_desc == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    // prepare defined socket for binding
    connect.server_sock.sun_family = AF_UNIX;
    memset(connect.server_sock.sun_path, 0, sizeof(connect.server_sock.sun_path));
    name_len = strlen(socket_name);
    if ( name_len > sizeof(connect.server_sock.sun_path) - 1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    memcpy(connect.server_sock.sun_path, socket_name, name_len+1);
    unlink(connect.server_sock.sun_path);  // destroy any socket of the same name if it already exists
    
    socket_len = sizeof(sockaddr_un);
    // bind the values to the socket
    if ( bind(connect.server_desc, (struct sockaddr*)&connect.server_sock, socket_len) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    // don't set a timer if there you want to debug.
    if ( !debug ) {
        // set timer for server socket
        if ( setsockopt(connect.server_desc, SOL_SOCKET, SO_RCVTIMEO, (timeval *)&tv, sizeof(tv)) == -1 ) {
            print_error(strerror(errno), __FILE__, __LINE__);
            return -1;
        }
    }
    
    return 0;
}

ErrorType accept_connection(ConnectionPlayer &connect, int server_desc) {
    sockaddr_un player_sock;
    socklen_t len = sizeof(sockaddr_un);
    timeval tv;

    // set timer for client sockets.
    tv.tv_sec   = SECONDS;
    tv.tv_usec  = MICROSECONDS;   // half a second

    if ( listen(server_desc, 1) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return ErrConnect;
    }

    connect.desc = accept(server_desc, (sockaddr *)&player_sock, &len);
    if ( connect.desc == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return ErrConnect;
    }

    // don't set a timer if you want to debug.
    if ( !debug ) {
        // set timer for player socket
        if ( setsockopt(connect.desc, SOL_SOCKET, SO_RCVTIMEO, (timeval *)&tv, sizeof(tv)) == -1 ) {
            print_error(strerror(errno), __FILE__, __LINE__);
            return ErrConnect;
        }
    }
    
    return OK;
}


/* ──────────────────────── *
 * PLAYER PROCESS FUNCTIONS *
 * ──────────────────────── */

ErrorType start_player(ConnectionPlayer &connect, int server_desc, const char *path, const char *socket_name) {
    ErrorType err;

    char *argv[] = { (char *)path, (char *)socket_name, NULL};
    err = run_player(path, argv, connect.pid);
    if ( err != OK ) {
        if ( connect.pid != -1) kill_player(connect.pid);
        return err;
    }
    err =  accept_connection(connect, server_desc);
    if ( err != OK ) {
        kill_player(connect.pid);
    }
    return err;
}

ErrorType run_player(const char *path, char **argv, pid_t &pid) {
    pid = fork();   // create a child process

    switch (pid) {
        case 0: // child process
            // set CTRL-C to kill the children.
            signal(SIGINT, SIG_DFL);
            if ( execv(path, argv) == -1 ) {
                print_error(strerror(errno), __FILE__, __LINE__);
                exit(-1); // just exit this process asap
            }
            break;
        case -1:    // error, exit out of this process.
            print_error(strerror(errno), __FILE__, __LINE__);
            exit(-1);
        default:
            break;
    }
    return OK;
}

void wait_player(ConnectionPlayer &connect) {
    // status can't initially be null.
    int status = 1;

    timeval start, end, difference;
    gettimeofday(&start, NULL);
    end.tv_sec = start.tv_sec + SECONDS;
    end.tv_usec = start.tv_usec + MICROSECONDS;
    
    bool exited = false;

    do {
        // wait for player for the default time length
        waitpid(connect.pid, &status, WNOHANG);
        if ( WIFEXITED(status) ) {
            int exit_status = WEXITSTATUS(status);
            if ( exit_status != 0 ) {
                fprintf(stderr, "Player exit status: %d\n", exit_status);
            }
            exited = true;
            break;
        }
        gettimeofday(&start, NULL);
        difference.tv_sec = end.tv_sec - start.tv_sec;
        difference.tv_usec = end.tv_usec - start.tv_usec;
    } while (difference.tv_sec > 0 || (difference.tv_sec == 0 && difference.tv_usec >= 0));

    if ( !exited ) {
        kill_player(connect.pid);
    }

    return;
}

void kill_player(pid_t &pid) {
    int status = 1;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    return;
}


/* ────────────────────────────── *
 * MESSAGE TRANSMISSION FUNCTIONS *
 * ────────────────────────────── */

ErrorType send_msg(int player_desc, char *msg) {
    if ( send(player_desc, msg, MAX_MSG_SIZE, 0) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return ErrSend;
    }
    return OK;
}

ErrorType recv_msg(int player_desc, char *msg) {
    memset(msg, 0, MAX_MSG_SIZE);
    if ( recv(player_desc, msg, MAX_MSG_SIZE, 0) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return ErrReceive;
    }
    // received an empty message, usually indication of an early exited AI...
    if ( strnlen(msg, MAX_MSG_SIZE) == 0 ) {
        print_error(RECV_MESSAGE_ERR, __FILE__, __LINE__);
        return ErrReceive;
    }
    return OK;
}


/* ────────────────────── *
 * CLOSE SOCKET FUNCTIONS *
 * ────────────────────── */

void close_player_sockets(Connection &connect) {
    if (connect.player1.desc != 0) {
        close(connect.player1.desc);
    }
    if (connect.player2.desc != 0) {
        close(connect.player2.desc);
    }
    return;
}

void close_sockets(Connection &connect) {
    close_player_sockets(connect);
    if (connect.server_desc != 0) {
        close(connect.server_desc);
    }
    unlink(connect.server_sock.sun_path);
    return;
}