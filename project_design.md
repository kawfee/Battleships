# Battleships Contest Project Design & Requirements
#### _By Matthew Getgen and Joey Gorski_


## Project Goal
To create a Battleships Contest Program that runs and handles a game and contest of Battleships played by a number of AIs.


## Project Requirements
- Run a game of battleships between two AIs.
- Run a match of multiple games between two AIs.
- Run a contest of matches between multiple AIs (some tournament method).
- Create a system that is compatible with multiple programming languages.
- Display actions taken by each AI on the screen.
- Store game and contest data.
    - Print the end result to the user and store all data to a log file.
- Display match or contest data effectively.


## Definitions
- `Turn`: 1 AI 1 action.
- `Round`: Each AI's turn.
- `Game`: Set of rounds between AI with an outcome. (Like a game of battleships with your friends).
- `Match`: Set number of games between two AI players.
- `Contest`: Set of matches between all available players.


## Message Requirements and Design
>_Main Idea:_ send everything as a JSON string for simpler language support.

### Messages Types and related Content
**Server messages to send:**
- Setup Match
    - board size
    - player number (1 or 2)
- Start Game
- Place Ship
    - length
- Take Shot
- Shot Return
    - player number (1 or 2)
    - row
    - col
    - shot value
        - hit       (X)
        - miss      (M)
        - duplicate (D)
- Ship Dead
    - player number (1 or 2)
    - row
    - col
    - direction
        - horizontal(H)
        - vertical  (V)
    - length
- Game Over
- Match Over

**Player messages to send:**
- Hello (lets server know who you are and that you're alive)
    - ai name
    - author name(s)
- Ship Placed
    - row
    - col
    - direction
        - horizontal(H)
        - vertical  (V)
    - length
- Shot Taken
    - row
    - col


### Message Communication Protocol
Packet Communication Method:
| Message Num | Message Name  | Player1 |     | Server |     | Player2 |
| ----------- | ------------- | ------- | --- | ------ | --- | ------- |
|           1 | `hello`       | player1 |  -> | server | <-  | player2 |
|           2 | `setup_match` | player1 | <-  | server |  -> | player2 |
|           3 | `start_game`  | player1 | <-  | server |  -> | player2 |
|           4 | `place_ship`  | player1 | <-  | server |  -> | player2 |
|           5 | `ship_placed` | player1 |  -> | server | <-  | player2 |
|           6 | `take_shot`   | player1 | <-  | server |  -> | player2 |
|           7 | `shot_taken`  | player1 |  -> | server | <-  | player2 |
|           8 | `shot_return` | player1 | <-  | server |  -> | player2 |
|           9 | `ship_dead`   | player1 | <-  | server |  -> | player2 |
|          10 | `game_over`   | player1 | <-  | server |  -> | player2 |
|          11 | `match_over`  | player1 | <-  | server |  -> | player2 |

1. `hello` is sent from both players to the server after it connects.
2. `setup_match` is sent by the server at the start of each match. Tells players about match-specific data.
3. `start_game` is sent by the server at the start of each game.
4. `place_ship` is sent by the server at the start of each game to tell players to place ships.
5. `ship_placed` is sent by the player to tell the server where the ship was placed.
6. `take_shot` is sent by the server to tell the client to take a shot.
7. `shot_taken` is sent by the players to tell the server where it shot.
8. `shot_return` is sent by the server to tell both players the value of the shot.
9. `ship_dead` is sent by the server to tell both players that a ship has been killed.
10. `game_over` is sent by the server when a game is over.
11. `match_over` is sent to the players when the match is over, or there was a match-level error.

### Message Abbreviations and Examples
> JSON key words are abbreviated for smaller message sizes, message types are numbers for faster comparisons.

Abbreviation definitions can be found in the `source/defines.h` file, or assumed by context. 
The 
Look to the [message protocol](#message-communication-protocol) section for the message type numbers.

#### Examples
**Server:**
- `{ "mt": 2, "bs": 10, "pn": 1 }`
- `{ "mt": 3 }`
- `{ "mt": 4, "l": 5 }`
- `{ "mt": 6 }`
- `{ "mt": 8, "pn": 1, "r": 1, "c": 1, "v": HIT }`
- `{ "mt": 9, "pn": 1, "r": 1, "c": 1, "l": 5, "d": HORIZONTAL }`
- `{ "mt": 10 }`
- `{ "mt": 11 }`

**Player:**
- `{ "mt": 1, "ai": "example_ai_c++", "au": "Mamthew Gemchin and Goey Jorski" }`
- `{ "mt": 5, "r": 1, "c": 1, "l": 5, "d": HORIZONTAL }`
- `{ "mt": 7, "r": 1, "c": 1 }`


## Runtime Option Breakdown

- Ask the user what they want to run:
    - Test AI
    - Run Contest
    - Replay log
- If Test AI:
    - Ask for board size (3-10)
    - Ask for number of games (500 is standard)
    - Ask for how to log the games:
        - First 5, last 5
        - 5 wins, 5 losses, 5 ties, all errors
        - all (bad, but it's an option)
    - Ask how many games to display:
        - none
        - last
        - 1 win, 1 loss, 1 tie, 1 error (if present)
        - all (bad, but it's an option)
    - If num games to display is not none:
        - Ask for time between rounds to display (0.3, 0.5, 1)
    - Ask for AI 1
    - Ask for AI 2
- If Run Contest:
    - Ask for Contest Type:
        - Brandle Special
        - Round-Robin
        - Bracket-Style
    - For any of these contest types:
        - Ask for board size (3-10)
        - Ask for number of games (500 is standard)
        - (don't display any games, just results)
    - If Bracket-Style:
        - present bracket in a beautified way
- If Replay Log:
    - Give list of log files (latest contest or latest match)
        - Ask for which one to display.
    - If contest log:
        - Display all matches in the contest log
            - Ask which one to play
        - Ask how many games to display:
            - none
            - last
            - 1 win, 1 loss, 1 tie, 1 error (if present)
            - all (bad, but it's an option)
        - If num games to display is not none:
            - Ask for time between rounds to display (0.3, 0.5, 1)
    - If match log:
        - Ask how many games to display:
            - none
            - last
            - 1 win, 1 loss, 1 tie, 1 error (if present)
            - all (bad, but it's an option)
        - If num games to display is not none:
            - Ask for time between rounds to display (0.3, 0.5, 1)

## JSON Game Log Standard
```JSON
{
    "player1": {
        "ships": [
            {
                "row": 0,
                "col": 0,
                "len": 5,
                "dir": HORIZONTAL,
            },
        ],
        "shots": [
            {
                "row": 0,
                "col": 0,
                "value": HIT,
                "index_of_ship_sunk": 0, // OPTIONAL value (if a ship was sunk)
            },
        ],
        "outcome": WIN,
    },
    "player2": {
        "ships": [
            {
                "row": 0,
                "col": 0,
                "len": 5,
                "dir": HORIZONTAL,
            },
        ],
        "shots": [
            {
                "row": 0,
                "col": 0,
                "value": HIT,
            },
        ],
        "outcome": LOSS,
    },
}
```

## JSON Match Log Standard
```JSON
{
    "elapsed_time": 2,
    "board_size": 10,
    "player1": {
        "ai_name": "player_example_c++",
        "author_names": "Matthew Getgen",
        "wins": 0,
        "losses": 0,
        "ties": 0,
        "error": NO_ERR,
    },
    "player2": {
        "ai_name": "player_example_python",
        "author_names": "Matthew Getgen",
        "wins": 0,
        "losses": 0,
        "ties": 0,
        "error": NO_ERR,
    },
    "games": [],
}
```

## JSON Contest Log Standard
```JSON
{
    "board_size": 10,
    "players": [
        {
            "player_idx": 0,
            "ai_name": "",
            "author_names": "",
            "total_wins": 0,
            "total_losses": 0,
            "total_ties": 0,
        },
    ],
    "matches": [
        [
            {
                "elapsed_time": 0,
                "player1": {
                    "player_idx": 1,
                    "lives": 3,
                    "wins": 0,
                    "losses": 0,
                    "ties": 0,
                    "error": NO_ERR,
                },
                "player2": {
                    "player_idx": 0,
                    "lives": 3,
                    "wins": 0,
                    "losses": 0,
                    "ties": 0,
                    "error": NO_ERR,
                } ,
                "last_game": {},
            },
        ],
    ],
}
```

## JSON Step-Through Log Standard
```JSON
{
    "games": [
        {
            "player1": [
                {
                    "row": 0,
                    "col": 0,
                    "len": 5,
                    "dir": HORIZONTAL,
                },
                {
                    "row": 0,
                    "col": 0,
                    "value": HIT,
                    "index_of_ship_sunk": 0, // OPTIONAL value (if a ship was sunk)
                },
            ],
            "player2": [
                {
                    "row": 0,
                    "col": 0,
                    "len": 5,
                    "dir": HORIZONTAL,
                },
                {
                    "row": 0,
                    "col": 0,
                    "value": HIT,
                },
            ],
        },
    ]
}
```

## Display a Game for a Match

### See Playthrough
```
<Author Name 1>     (TOP_ROW-10)
──── VS ────        (TOP_ROW-9)
<Author Name 2>     (TOP_ROW-8)

Game #<num>         (TOP_ROW-6)

<AI Name>'s Board:  (TOP_ROW-4)

 │0123456789        (TOP_ROW-2)
─┼──────────        (TOP_ROW-1)
0│~~~~~~~~~~        (TOP_ROW) 
1│~~~~~~~~~~
2│~~~~~~~~~~
3│~~~~~~~~~~
4│~~~~~~~~~~
5│~~~~~~~~~~
6│~~~~~~~~~~
7│~~~~~~~~~~
8│~~~~~~~~~~
9│~~~~~~~~~~

<shot value>        (TOP_ROW+board_size+1)
```

### See Final Result
```
<Author Name 1>
--- VS ---
<Author Name 2>

Game #<num>

Final Status of <AI Name>'s Board:

 │0123456789
─┼──────────
0│~~~~~~~~~~
1│~~~~~~~~~~
2│~~~~~~~~~~
3│~~~~~~~~~~
4│~~~~~~~~~~
5│~~~~~~~~~~
6│~~~~~~~~~~
7│~~~~~~~~~~
8│~~~~~~~~~~
9│~~~~~~~~~~

<GameResult or Error>


    AI Name │   Wins │ Losses │   Ties │    (TOP_ROW+board_size+4)
────────────┼────────┼────────┼────────┤
 <AI Name>  │      0 │      0 │      0 │
 <AI Name>  │      0 │      0 │      0 │

 Time elapsed: <time> seconds               (TOP_ROW+board_size+9)
```

### See Final Result Without Full Display
```
Match Results

    AI Name │   Wins │ Losses │   Ties │    (-TOP_ROW-1)
────────────┼────────┼────────┼────────┤
 <AI Name>  │      0 │      0 │      0 │
 <AI Name>  │      0 │      0 │      0 │

 Time elapsed: <time> seconds               (-TOP_ROW-1+5)
```

## Display a Game for a Contest

### See Playthrough
```
<Author Name 1>
--- VS ---
<Author Name 2>

Last Game

<AI Name>'s Board:

 │0123456789
─┼──────────
0│~~~~~~~~~~
1│~~~~~~~~~~
2│~~~~~~~~~~
3│~~~~~~~~~~
4│~~~~~~~~~~
5│~~~~~~~~~~
6│~~~~~~~~~~
7│~~~~~~~~~~
8│~~~~~~~~~~
9│~~~~~~~~~~

<shot value>
```

### See Final Result
```
<Author Name 1>
--- VS ---
<Author Name 2>

Last Game

Final Status of <AI Name>'s Board:

 │0123456789
─┼──────────
0│~~~~~~~~~~
1│~~~~~~~~~~
2│~~~~~~~~~~
3│~~~~~~~~~~
4│~~~~~~~~~~
5│~~~~~~~~~~
6│~~~~~~~~~~
7│~~~~~~~~~~
8│~~~~~~~~~~
9│~~~~~~~~~~

<GameResult or Error>


    AI Name │   Wins │ Losses │   Ties │
────────────┼────────┼────────┼────────┤
 <AI Name>  │      0 │      0 │      0 │
 <AI Name>  │      0 │      0 │      0 │

 Time elapsed: <time> seconds
```

### See Leaderboard
```
Top 10 Leaderboard (or Final Leaderboard)         (TOP_ROW-10)

 Rank │ Author(s) │ AI Name │   Wins │ Losses │   Ties │   (TOP_ROW-8)
──────┼───────────┼─────────┼────────┼────────┼────────┤
    1 │ <Authors> │<AI Name>│      0 │      0 │      0 │
```

## Display Step-through of a Game
```
<Author Name 1>
--- VS ---
<Author Name 2>

Last Game

<AI Name>'s Board:

 │0123456789
─┼──────────
0│~~~~~~~~~~
1│~~~~~~~~~~
2│~~~~~~~~~~
3│~~~~~~~~~~
4│~~~~~~~~~~
5│~~~~~~~~~~
6│~~~~~~~~~~
7│~~~~~~~~~~
8│~~~~~~~~~~
9│~~~~~~~~~~

<ship placement or shot value or game result>


<Please hit ENTER:  or Please use ->, <-, or ENTER to end: >    (TOP_ROW+board_size+4)
```

## TODOS
1. make a diagram of how the whole thing runs/works

