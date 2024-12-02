# Battleships Contest
#### _By Matthew Getgen, Joey Gorski, Matthew Bouch, and Stefan Brandle_
***

This project's goal is to handle a Battleships Contest between AIs through JSON messages and Unix Domain Sockets.


## Repository Information
---

Welcome to the Battleships Contest! This repository has all of the code
to make a new AI and compete against other Battleship AI.

### Definitions:
- `Game`: Starts by placing ships, then taking shots at the opponent's board. Ends when all of an AI's ships are killed (Like a game of battleships with your friends).
- `Match`: Set number of games between two AI players (Usually 500).
- `Contest`: Set of matches between all available players with a winner.
- `Round`: Set of matches inside a round that can determine if a player continues in the contest or not.

### Project Directories:
- `source/` -- Where game logic, message, display, and logging code is stored.
- `logs/` -- Where the log files are stored.
- `ai_files/` -- Where you will store your AI code.
- `ai_files/protected/` -- Where the protected AIs and protected AI player classes are stored.


## Install
---

To install the battleships project:
- Download this repo to a Linux computer (matt, lab machine, or personal).
- `cd` into the main project directory.
- Run `make build` to compile the controller code and all players.
    - You should only need to do this once, or after a `make clean`.


## Usage
---

To use the battleships project:
- After making changes to your AI, run `make` to compile your code and run the controller. You can also do `make run` or `./controller`.
- The questions are pretty straightforward, and the defaults are great! You can hit enter on questions with defaults.
- If you want, you could use the `options.json` file to store the options you use the most. Just run `make options` to create the JSON file, and then modify the values for the runtime. Anything you want to choose manually, set the value to `""` or `NULL`.

### Runtime Questions:
- `Test AI` -- Plays a match between two AI (you must choose the AI to play).
    - Use this in the development of your AI.
- `Run Contest` -- Plays a contest between all available AI.
    - More than likely, you won't have to use this unless you are Dr. Brandle.
- `Replay Logs` -- Replays a log file for you.
    - Replays log files from either the last match or contest played.


## How to Create a new C++ or Python AI:
---

- Copy or rename the `player_example.cpp` and `player_example.h` for C++, or `player_example.py` for Python.
- Replace the default `AI_NAME` variable with your AI's name.
- Replace the default `AUTHOR_NAMES` variable with you and/or your Teammates' names.
- In C++:
    - Look at `Player.cpp` and `Player.h` files to get a better understanding of your base class.
    - Rename the `PlayerExample` class name to your player's class.
    - Add variables and functions to the private class's header file.
        - If you want to add any boards, make sure to add them to the `create_boards()`, `clear_boards()`, and `delete_boards()` functions (or any other board functions you create).
- In Python:
    - Make sure that your python file is an executable (run `chmod 755 your_file.py`).
    - Look at the `Player.py` file to get a better understanding of your base class.
    - Rename the `PlayerExample` class name in your player file.
    - Add values you want to track throughout the match under the `__init__()` function of the player class.
        - If you want to add any boards, make sure to add them to the `create_boards()`, `clear_boards()`, and `delete_boards()` functions (or any other board functions you create).

- Modify the ship placement, shot placement, and other game-logic functions to make a better AI!

### Defined Values
- Ship Directions:
    - Either `HORIZONTAL` or `VERTICAL`.
- Board Values:
    - `WATER` - Only used to clear a board, or check if a spot has been shot at.
    - `SHIP` - Only used to store the location of a ship onto a board.
    - `MISS` - Returned by `shot_return` when `WATER` was shot at. Store it to not shoot there again.
    - `HIT` - Returned by `shot_return` when a `SHIP` was shot at. Store it to shoot by it but not there again.
    - `KILL` - Only used to store where a `SHIP` was killed.
    - `DUPLICATE_HIT`, `DUPLICATE_MISS`, and `DUPLICATE_KILL` - Returned by `shot_return` when a shot was made at the same location two or more times. You should stop your AI from doing this.


## Error Codes and potential issues:
---

| Code(s) | Potential Reasons                                       | Potential Fixes                                                                  |
| ------- | ------------------------------------------------------- | -------------------------------------------------------------------------------- |
|       0 | No error                                                | Nothing to do                                                                    |
|     1-2 | Player process failed to run                            | Make sure your AI has execute permissions                                        |
|     3-4 | Player process exited early                             | Make sure your AI didn't segfault, get caught in an infinite loop, etc.          |
|     5-7 | A message sent by the player didn't follow the protocol | Usually this shouldn't happen. Make sure your player follows the protocol        |
|       8 | Ship length returned by player isn't the right length   | The player placed a ship of the wrong length. Don't return the wrong ship length |
|       9 | The ship that was placed is off the board               | Make sure your AI respects the board boundary when placing a ship                |
|      10 | The ship that was placed intersects with another ship   | Make sure your AI doesn't place ships on top of each other                       |
|      11 | The shot that was taken is off the board                | Make sure your AI respects the board boundary when placing a shot                |

> If you are still having an issue, contact me on the CSE slack @ mattgetgen


## Debugging an AI:
---

There is a debug mode for the controller that disables message timeouts. This can be accessed by adding a `-d` or `--debug` to the controller's arguments.
> **Example:** `./controller --debug`

### Debugging with **gdb**:
- Run `gdb -q controller` on the command line.
- In the gdb command line, type these commands:
    - `break main` or `b main`
        - This breakpoint will hit both the main function in controller, and the main function of the player you are debugging.
    - `set follow-fork-mode child`
        - `fork()` is used to create a new process.
        - The controller uses forks to run an AI in a separate process.
        - This will allow gdb to step into the forked process.
    - `set follow-exec-mode new`
        - `execve()` is used to execute a new program.
        - The new process runs this to execute the AI into the process.
        - This will allow gdb to attach to the AI.
    - `run --debug` or `r -d`
        - Runs the controller with the debug mode set.
    - `continue` or `c`
        - Continues execution until prompted for runtime questions.
        > **NOTE:** -- The first AI you choose at runtime will be the AI you debug.
- After you choose the AI to play a match, it should stop at the AI's main function where you can:
    - set breakpoints, step through the program, etc.
- This does not work for Python AIs.


## How to Create a new AI in a language of your choice
---

### For a language to be compatible with this project, it must:
- Support JSON and Unix Domain Sockets for the protocol.
- Be either compiled to an executable, or interpreted with a command line "executable-like" workaround.
    - Python usually requires `python3 player_example.py` at the command line to run, but if you chmod the file to give it executable permissions, and add an `#!/usr/bin/python3` or similar shebang at the top of the file, it can be executed at the command line without having to put `python3` before the filename. A similar approach is available for node/js.

### For an AI to be compatible with this project, it must:
- Accept the socket file path as the first program argument.
    - How the player is run by the controller:
```shell
./player_example /path/to/battleships.socket
```
- Connect to the socket using:
    - `AF_UNIX` as the TYPE.
    - `SOCK_STREAM` as the PROTOCOL.
- Send and receive JSON (converted into c-string) messages over the socket.
- Handle different message types:
    - Create messages to send to the server:
        - `hello`
        - `ship_placed`
        - `shot_taken`
    - Receive and process messages sent from the server:
        - `setup_match`
        - `start_game`
        - `place_ship`
        - `take_shot`
        - `shot_return`
        - `game_over`
        - `match_over`
    - For more details about the protocol, go to the [Protocol](docs/protocol.md) page to read more.
    - If you are really interested in making a new AI, make sure to implement everything in the base `Player` class files. Everything there is required.

