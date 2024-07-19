# Battleships Contest Project Design & Requirements
#### _By Matthew Getgen and Joey Gorski_

## Project Goal 
---
To create a program that can interact with student's Battleships AIs and manage matches and contests between them.

## Project Requirements
---
- [x] Run a game of battleships between two AI
- [x] Run a match of multiple games between two AI
- [x] Run a contest between multiple AI to find a winner
- [x] Create a system that can run and manage AI processes separate from the controller
- [x] Create a system that can support different programming languages
- [x] Display actions taken by the AI onto the terminal
- [x] Log actions taken by each AI into a file for playback
- [x] Track player stats for games and matches to allow students to evaluate how their player performs

## Project Definitions
---
- **Game:** Set of ship placements and shots taken until there is a winner
- **Match:** Set number of games played between two AI
- **Contest:** Multiple matches played between multiple AI to determine the best AI

## Project Design
---
### AI Process Management
The Controller manages AI by starting separate processes and running the AI executable in that separate process. From there it will communicate through a shared socket.

Currently, the server only passes one argument to the player, which is the path to the socket being used by the server. When each AI is executed, it should read in this socket path and connect to it. In practice this would look like:
``` bash
$ ./player_example /path/to/battleships.socket
```
From there, the rest of the communication occurs over the socket and the process is left to communicate with the Controller until the Match is over. When the Match is over, it will kill the AI process to make sure it is no longer running on the system.

### Socket Communication
The Controller uses something called Unix Domain Sockets (UDS for short). This is a standard type of socket for inter-process communication. Meaning, you can communicate with a different process on the same CPU, without going through the network, making it a very fast communication standard, and is great for what this project needs.

To connect with the socket, simply get the path from the AI's argument list, and use the socket API for your language to connect with it. This is handled by default for the `Player.cpp` and `Player.py` classes. Use these class files to get an example for an AI in another language.

### Message Protocol
The message protocol for Battleships is defined [here](./protocol.md). There have been some changes to the protocol, and all of the latest data is contained there.

