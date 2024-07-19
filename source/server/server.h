/**
 * @file server.h
 * @author Matthew Getgen
 * @brief Battleships Server Definitions, Revised
 * @date 2023-08-27
 * 
 * Unix Domain Socket Programming from [Beej's Guide](https://beej.us/guide/bgipc/html/multi/unixsock.html)
 * 
 */


#ifndef SERVER_H
#define SERVER_H

#include <cerrno>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../defines.h"

const long SECONDS = 0;
const long MICROSECONDS = 500000; // 0.5 seconds

/// @brief Struct that contains important connection data per Player.
struct ConnectionPlayer {
    int desc;
    pid_t pid;
    char msg[MAX_MSG_SIZE];
};

/// @brief Struct that contains important connection data.
struct Connection {
    sockaddr_un server_sock;
    int server_desc;
    ConnectionPlayer player1;
    ConnectionPlayer player2;
};


/* ─────────────────────────── *
 * SOCKET CONNECTION FUNCTIONS *
 * ─────────────────────────── */

/// @brief Creates a usable socket struct for the server. If creation fails, it exists the program.
/// @param socket_name name of the socket to create.
/// @return Connection data structure, or exit on error.
Connection create_socket(const char *socket_name);

/// @brief Creates a socket for a contest or match.
/// @param connect Struct to store connection.
/// @param socket_name name of socket to create
/// @return 0 on success, -1 or other error on failure.
int bind_socket(Connection &connect, const char *socket_name);

/// @brief Accepts a connection from an executed player.
/// @param connect Struct to store connection.
/// @param server_desc Socket to connect with.
/// @return OK on success, BAD_CONNECT on failure.
ErrorType accept_connection(ConnectionPlayer &connect, int server_desc);


/* ──────────────────────── *
 * PLAYER PROCESS FUNCTIONS *
 * ──────────────────────── */

/// @brief Runs the player, and then connects with them. Will kill the player if the player fails.
/// @param connect Player connection data.
/// @param server_desc Server socket desc.
/// @param path executable path for player.
/// @param socket_name name of the socket to create.
/// @return OK on success, BAD_FORK or BAD_CONNECT on failure.
ErrorType start_player(ConnectionPlayer &connect, int server_desc, const char *path, const char *socket_name);

/// @brief Runs an executable in a separate process (a little fork and exec action).
/// @param path executable path for player.
/// @param argv player arguments.
/// @param pid pointer to process ID value.
/// @return OK on success, BAD_FORK on failure.
ErrorType run_player(const char *path, char **argv, pid_t &pid);

/// @brief Waits and collects the player return value. If the player is still running, kill them.
/// @param connect Connection Player data.
void wait_player(ConnectionPlayer &connect);

/// @brief Kills a player process. Do this when the match is over, or there is an error.
/// @param pid player pid to kill.
void kill_player(pid_t &pid);


/* ────────────────────────────── *
 * MESSAGE TRANSMISSION FUNCTIONS *
 * ────────────────────────────── */

/// @brief Send a message to a player.
/// @param player_desc socket to send to.
/// @param msg message buffer to send.
/// @return OK on success, BAD_SEND on failure.
ErrorType send_msg(int player_desc, char *msg);

/// @brief Receive a message from a player.
/// @param player_desc socket descriptor to receive from.
/// @param msg message buffer to receive from.
/// @return OK on success, BAD_RECV on failure.
ErrorType recv_msg(int player_desc, char *msg);


/* ────────────────────── *
 * CLOSE SOCKET FUNCTIONS *
 * ────────────────────── */

/// @brief Close the player-specific socket descriptors. Do this when the match is over, or there's an error.
/// @param connect socket data to close.
void close_player_sockets(Connection &connect);

/// @brief Close the socket and socket descriptors for a connection. do this when contest or match is over, or if there's an error.
/// @param connect socket data to close.
void close_sockets(Connection &connect);

#endif
