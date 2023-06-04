/**
 * @file server.cpp
 * @authors Matthew Getgen
 * @brief Battleships Server C++ file
 * @date 2022-04-18
 * 
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/multi/unixsock.html)
 */

#include "server.h"


/* ─────────────────────────── *
 * SOCKET CONNECTION FUNCTIONS *
 * ─────────────────────────── */

ErrorNum bind_socket(Connection *connect, char *socket_name) {
    socklen_t socket_len;
    size_t name_len;
    timeval tv;

    // set timer
    tv.tv_sec   = 0;
    tv.tv_usec  = 500000;   // half a second, very generous.

    // define the socket
    connect->server_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( connect->server_desc == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_SOCK_CREATE;
    }

    // prepare defined socket for binding
    connect->server_sock.sun_family = AF_UNIX;
    memset(connect->server_sock.sun_path, 0, sizeof(connect->server_sock.sun_path));
    name_len = strlen(socket_name);
    if ( name_len > sizeof(connect->server_sock.sun_path) - 1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_SOCK_NAME;
    }
    memcpy(connect->server_sock.sun_path, socket_name, name_len+1);
    unlink(connect->server_sock.sun_path);  // destroy any socket of the same name if it already exists
    
    socket_len = sizeof(sockaddr_un);
    // bind the values to the socket
    if ( bind(connect->server_desc, (struct sockaddr*)&connect->server_sock, socket_len) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_SOCK_BIND;
    }

    // don't set a timer if there you want to debug.
    if ( !debug ) {
        // set timer for server socket
        if ( setsockopt(connect->server_desc, SOL_SOCKET, SO_RCVTIMEO, (timeval *)&tv, sizeof(tv)) == -1 ) {
            print_error(strerror(errno), __FILE__, __LINE__);
            return BAD_SOCK_OPT;
        }
    }
    
    return NO_ERR;
}

ErrorNum accept_connection(int server_desc, int *player_desc) {
    sockaddr_un player_sock;
    socklen_t len = sizeof(sockaddr_un);
    timeval tv;

    // set timer
    tv.tv_sec   = 0;
    tv.tv_usec  = 500000;   // half a second

    if ( listen(server_desc, 1) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_CONNECT;
    }

    *player_desc = accept(server_desc, (sockaddr *)&player_sock, &len);
    if ( *player_desc == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_CONNECT;
    }

    // don't set a timer if you want to debug.
    if ( !debug ) {
        // set timer for player socket
        if ( setsockopt(*player_desc, SOL_SOCKET, SO_RCVTIMEO, (timeval *)&tv, sizeof(tv)) == -1 ) {
            print_error(strerror(errno), __FILE__, __LINE__);
            return BAD_SOCK_OPT;
        }
    }
    
    return NO_ERR;
}


/* ──────────────────────── *
 * PLAYER PROCESS FUNCTIONS *
 * ──────────────────────── */

ErrorNum run_player(char *path, char **argv, pid_t *pid) {

    *pid = fork();   // create a child process

    switch (*pid) {
        case 0: // child process
            // set CTRL-C to kill the children.
            signal(SIGINT, SIG_DFL);
            if ( execv(path, argv) == -1 ) {
                print_error(strerror(errno), __FILE__, __LINE__);
                exit(-1); // just exit this process asap
            }
            break;
        case -1:    // error with , no process made
            print_error(strerror(errno), __FILE__, __LINE__);
            exit(-1);
        default:
            break;
    }
    return NO_ERR;
}

void kill_player(pid_t pid) {
    kill(pid, SIGKILL);
    return;
}


/* ────────────────────────────── *
 * MESSAGE TRANSMISSION FUNCTIONS *
 * ────────────────────────────── */

ErrorNum send_message(int player_desc, char *msg) {
    if ( send(player_desc, msg, MAX_MSG_SIZE, 0) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_SEND;
    }
    return NO_ERR;
}

ErrorNum recv_message(int player_desc, char *msg) {
    memset(msg, 0, MAX_MSG_SIZE);
    if ( recv(player_desc, msg, MAX_MSG_SIZE, 0) == -1 ) {
        print_error(strerror(errno), __FILE__, __LINE__);
        return BAD_RECV;
    }
    return NO_ERR;
}


/* ────────────────────── *
 * CLOSE SOCKET FUNCTIONS *
 * ────────────────────── */

void close_player_sockets(Connection *connect) {
    close(connect->player1_desc);
    close(connect->player2_desc);
    return;
}

void close_sockets(Connection *connect) {
    close_player_sockets(connect);
    close(connect->server_desc);
    unlink(connect->server_sock.sun_path);
    return;
}

