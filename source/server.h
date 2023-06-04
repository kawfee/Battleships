/**
 * @file server.h
 * @author Matthew Getgen
 * @brief Battleships Server Definitions
 * @date 2022-04-25
 * 
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/multi/unixsock.html)
 * 
 */

#ifndef SERVER_H
#define SERVER_H

#include <cerrno>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#include "defines.h"

/**
 * @brief A Structure used to store important connection data.
 */
struct Connection {
    sockaddr_un server_sock;
    int server_desc;
    int player1_desc;
    int player2_desc;
    pid_t pid1;
    pid_t pid2;
};


/* ─────────────────────────── *
 * SOCKET CONNECTION FUNCTIONS *
 * ─────────────────────────── */

/**
 * @brief Creates one sockets for the entire contest or match. This is called once.
 * @param connect to store connection data.
 * @param socket_name socket name to create.
 * @return NO_ERR on success, BAD_SOCK_CREATE, BAD_SOCK_NAME, BAD_SOCK_BIND, or BAD_SOCK_OPT on failure.
 */
ErrorNum bind_socket(Connection *connect, char *socket_name);

/**
 * @brief Accepts a connection from an executed player.
 * @param server_desc server socket descriptor to listen for a connection.
 * @param player_desc pointer to player socket descriptor for the new connection.
 * @return NO_ERR on success, BAD_CONNECT or BAD_SOCK_OPT on failure.
 */
ErrorNum accept_connection(int server_desc, int *player_desc);


/* ──────────────────────── *
 * PLAYER PROCESS FUNCTIONS *
 * ──────────────────────── */

/**
 * @brief Runs an executable in a separate process ID.
 * Requires special handling for the arguments passed to it.
 * @param path executable path for player.
 * @param argv player arguments.
 * @param pid pointer to process ID value.
 * @return NO_ERR on success, BAD_FORK on failure.
 */
ErrorNum run_player(char *path, char **argv, pid_t *pid);

/**
 * @brief Kill a player process. Do this when the match is over or there is an error.
 * @param pid player pid to kill.
 */
void kill_player(pid_t pid);


/* ────────────────────────────── *
 * MESSAGE TRANSMISSION FUNCTIONS *
 * ────────────────────────────── */

/**
 * @brief Send a message to a player.
 * @param player_desc socket descriptor to send to.
 * @param msg message buffer to send.
 * @return NO_ERR on success, BAD_SEND on failure.
 */
ErrorNum send_message(int player_desc, char *msg);

/**
 * @brief Receive a message from a player.
 * @param player_desc socket descriptor to receive from.
 * @param msg message buffer to receive on.
 * @return NO_ERR on success, BAD_RECV on failure.
 */
ErrorNum recv_message(int player_desc, char *msg);


/* ────────────────────── *
 * CLOSE SOCKET FUNCTIONS *
 * ────────────────────── */

/**
 * @brief Close the player-specific socket descriptors.
 * Do this when the match is over, or there is an error.
 * @param connect socket data to close.
 */
void close_player_sockets(Connection *connect);

/**
 * @brief Close the socket and socket descriptors for a connection.
 * Do this when the contest or match is over, or if there is an error.
 * @param connect socket data to close.
 */
void close_sockets(Connection *connect);

#endif

