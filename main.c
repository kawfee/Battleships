#define _DEFAULT_SOURCE 1
#include <stdio.h>
// #include <time.h>
// #include <x86intrin.h>

#include "lib/battleshipslib.h"

// typedef struct {
//     long cycles;
//     struct timespec time;
// } Timer;
//
// static inline void get_time(Timer *timer)
// {
//     timer->cycles = __rdtsc();
//     if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer->time) != 0)
//     {
//         exit(1);
//     }
// }
//
// void print_time(Timer start, Timer end)
// {
//     long cycles = end.cycles - start.cycles;
//     long nanoseconds = ((end.time.tv_sec - start.time.tv_sec) * (long)1e9) + (
//         end.time.tv_nsec - start.time.tv_nsec
//     );
//     printf("%ld cycles, %ld ns\n", cycles, nanoseconds);
// }

int main(void)
{
    uint8_t board_size = 10;
    uint32_t games_per_match = 500;
    size_t match_memory_size = BShip_Match_CalculateMemorySize(board_size, games_per_match);

    BShip_Arena arena = {0};
    BShip_Arena_Initialize(&arena, match_memory_size);

    char example_player_1[] = "/home/mgetgen/repos/battleshipssource/ai/example_player/example_player";
    char example_player_2[] = "/home/mgetgen/repos/battleshipssource/ai/example_player_v2/example_player_v2";
    char bad_player[] = "/home/mgetgen/repos/battleshipssource/ai/00_bad_player/00_bad_player";
    char clean_player[] = "/home/mgetgen/repos/battleshipssource/ai/01_clean_player/01_clean_player";
    char gambler_player[] = "/home/mgetgen/repos/battleshipssource/ai/02_gambler_player/02_gambler_player";
    char easy_learning_gambler[] = "/home/mgetgen/repos/battleshipssource/ai/03_easy_learning_gambler/03_easy_learning_gambler";
    char learning_player[] = "/home/mgetgen/repos/battleshipssource/ai/04_learning_player/04_learning_player";
    char mean_player[] = "/home/mgetgen/repos/battleshipssource/ai/05_mean_player/05_mean_player";
    char *ai1_path = example_player_1;
    char *ai2_path = mean_player;

    BShip_Match_Run(&arena, "/tmp/battleships.sock", ai1_path, ai2_path, board_size, games_per_match, false);
    BShip_Arena_Destroy(&arena);
    return 0;
}
