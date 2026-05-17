# Battleships Message Protocol
#### _By Matthew Getgen_
***

The protocol has gone through two versions in the time of this project.
1. The original was designed to be simple.
2. The new was optimized to send fewer messages, and more information with the client/player (for the rest of this doc a Player will be referred to as a client, and the controller as the server).
    - This version saved nearly 48% of messages sent between the client/server over the course of the average match!

Because the protocol is handled at the `Player.cpp`/`Player.py` level, the effects of the re-write shouldn't impact older AI.

## Message Protocol
Protocol Restraints:
- The messages are sent over a [Unix Domain Socket](https://en.wikipedia.org/wiki/Unix_domain_socket) (`AF_UNIX`) using the `SOCK_STREAM` type.
- The messages are sent as C-style strings (null terminated).
- The messages are JSON with specific values depending on the type of message.

### Message Types:

Client-driven Messages:
- `Hello`
- `Ship Placed`
- `Shot Taken`

Server-driven Messages:
- `Setup Match`
- `Start Game`
- `Place Ship`
- `Take Shot`
- `Shot Return`
- `Game Over`
- `Match Over`

### Message Process:

The # of messages sent assumes 500 games per match and 20 shots per game.

| # | Client        |    | Server        | # Game/Match | Description                                                                                            |
| - | ------------- | -- | ------------- | ------------ | ------------------------------------------------------------------------------------------------------ |
| 1 | `Hello`       | -> |               |          0/1 | Client sends a `Hello` message letting the server know who it is                                       |
| 2 |               | <- | `Setup Match` |          0/1 | Server lets the client know match info, like board size and which player # they are                    |
| 3 |               | <- | `Start Game`  |        1/500 | Server lets the client know that a game has started                                                    |
| 4 |               | <- | `Place Ship`  |       6/3000 | Server lets the client know to choose a ship placement on their board. Must be of length provided      |
| 5 | `Ship Placed` | -> |               |       6/3000 | Client tells the server where they placed the ship                                                     |
| 6 |               | <- | `Take Shot`   |        1/500 | Server tells the client to take a shot. Happens once after ships placed, then handled by `Shot Return` |
| 7 | `Shot Taken`  | -> |               |     20/10000 | Client tells the server where they placed the shot                                                     |
| 8 |               | <- | `Shot Return` |     20/10000 | Server tells the client result of both player shots, if any ships were killed, and take next shot      |
| 9 |               | <- | `Game Over`   |        1/500 | Server lets the client know that the game is over, and stats from the game                             |
|10 |               | <- | `Match Over`  |          0/1 | Server lets the client know that the match is over                                                     |

### Message Data:

The protocol uses a JSON format to send the messages. The contents of the message are defined here:

#### Keys:

Keys are shortened to save on message length, and therefore saves on JSON serialization/deserialization time, memory use, message passing, etc.

Here are the messages and what they mean:
- "mt" = message type
- "ai" = player name
- "au" = author name(s)
- "bs" = board size
- "pn" = player number
    - The only valid values are either 1 or 2, depending on what player you are.
- "l" = ship length
- "r" = ship/shot row
- "c" = ship/shot column
- "d" = ship direction
    - The only valid values for this are either 'H' or 'V' characters in ASCII, so 72 or 86 respectively
- "p1" = player 1
    - this key refers to the player that was passed a 1 through the "pn" key.
- "p2" = player 2
    - this key refers to the player that was passed a 2 through the "pn" key.
- "st" = shot taken
- "v" = shot value
    - The only valid values for this are a part of the BoardValue enum, excluding the `WATER` value
- "sp" = ship killed
- "tns" = take next shot
- "gr" = game result
    - The only valid values for this are one of the three GameResult enum values
- "nb" = # shots made onto the opponent's board
- "nh" = # hits made onto the opponent's board
- "nm" = # misses made onto the opponent's board
- "nd" = # duplicates made onto the opponent's board
- "sk" = # opponent's ships killed

#### Definitions/Examples:

1. Hello:
```JSON
{
    "mt": 1,
    "ai": "Getgen Player",
    "au": "Matthew Getgen"
}
```

2. Setup Match:
```JSON
{
    "mt": 2,
    "bs": 10,
    "pn": 1
}
```

3. Start Game:
```JSON
{
    "mt": 3,
}
```

4. Place Ship:
```JSON
{
    "mt": 4,
    "l": 5,
}
```

5. Ship Placed:
```JSON
{
    "mt": 5,
    "l": 5,
    "r": 0,
    "c": 0,
    "d": 72
}
```

6. Take Shot:
```JSON
{
    "mt": 6,
}
```

7. Shot Taken:
```JSON
{
    "mt": 7,
    "r": 0,
    "c": 0
}
```

8. Shot Return:
```JSON
{
    "mt": 8,
    "p1": {
        "st": {
            "r": 0,
            "c": 0,
            "v": HIT,
        },
        "sp": {
            "r": 0,
            "c": 0,
            "l": 5,
            "d": HORIZONTAL,
        }
    },
    "p2": {
        "st": {
            "r": 0,
            "c": 0,
            "v": HIT,
        },
        "sp": {
            "r": 0,
            "c": 0,
            "l": 5,
            "d": HORIZONTAL,
        }
    },
    "tns": true
}
```

9. Game Over:
```JSON
{
    "mt": 8,
    "gr": WIN,
    "nb": 50,
    "nh": 24,
    "nm": 5,
    "nd": 0,
    "sk": 6
}
```

10. Match Over:
```JSON
{
    "mt": 9,
}
```
