#define _DEFAULT_SOURCE 1
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <strings.h>
#include <string>
#include <vector>
// #include <time.h>
// #include <x86intrin.h>

#include "lib/battleshipslib.h"
#include "src/tui/options.cpp"
#include "src/tui/results.cpp"

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

vector<BShip_AIFileData> GetAIs(BShip_Arena *arena)
{
    vector<BShip_AIFileData> ais;

    char *cwd = NULL;
    {
        const size_t CWD_BUFFER_SIZE_DEFAULT = 64;
        size_t cwd_buffer_size = CWD_BUFFER_SIZE_DEFAULT;
        char *cwd_buffer = NULL;
        BShip_ArenaMark cwd_mark = BShip_ArenaMark_Get(arena);

        while (cwd == NULL)
        {
            cwd_buffer = (char *)BShip_Arena_Push(arena, cwd_buffer_size);
            if (cwd_buffer == NULL)
            {
                return ais;
            }

            cwd = getcwd(cwd_buffer, cwd_buffer_size);
            if (cwd != NULL)
            {
                break;
            }
            if (errno != ERANGE)
            {
                perror("getcwd");
                return ais;
            }

            cwd_buffer_size += CWD_BUFFER_SIZE_DEFAULT;
            BShip_Arena_Rollback(arena, cwd_mark);
        }
    }
    string ai_dir_string = cwd;
    ai_dir_string += "/ai";

    DIR *ai_dir_ref = opendir(ai_dir_string.c_str());
    if (ai_dir_ref == NULL)
    {
        perror("opendir");
        return ais;
    }
    struct dirent *ai_dir_entry = NULL;

    while ((ai_dir_entry = readdir(ai_dir_ref)) != NULL)
    {
        // Skip "." and ".."
        if (strncmp(ai_dir_entry->d_name, ".", 1) == 0 ||
            strncmp(ai_dir_entry->d_name, "..", 2) == 0)
        {
            continue;
        }
        string player_dir_string = ai_dir_string + "/" + ai_dir_entry->d_name;
        {
            struct stat st = {};
            if (stat(player_dir_string.c_str(), &st) == -1)
            {
                perror("perror");
                continue;
            }

            if (!S_ISDIR(st.st_mode))
            {
                continue;
            }
        }

        DIR *player_dir_ref = opendir(player_dir_string.c_str());
        if (player_dir_ref == NULL)
        {
            perror("opendir");
            continue;
        }
        struct dirent *player_dir_entry = NULL;

        while ((player_dir_entry = readdir(player_dir_ref)) != NULL)
        {
            // Skip "." and ".."
            if (strncmp(player_dir_entry->d_name, ".", 1) == 0 ||
                strncmp(player_dir_entry->d_name, "..", 2) == 0)
            {
                continue;
            }
            string player_exec_string = player_dir_string + "/" + player_dir_entry->d_name;
            {
                struct stat st = {};
                if (stat(player_exec_string.c_str(), &st) == -1)
                {
                    perror("perror");
                    continue;
                }
                if (!S_ISREG(st.st_mode) || !(st.st_mode & S_IXUSR))
                {
                    continue;
                }
            }
            BShip_AIFileData player = {};
            typedef struct {
                char **dest;
                const char *src;
                size_t size;
            } PlayerPathWriter;
            PlayerPathWriter paths[] = {
                {
                    .dest = &player.file_name,
                    .src = player_dir_entry->d_name,
                    .size = strlen(player_dir_entry->d_name) + 1,
                },
                {
                    .dest = &player.file_path,
                    .src = player_exec_string.c_str(),
                    .size = player_exec_string.size() + 1,
                },
                {
                    .dest = &player.runtime_directory,
                    .src = player_dir_string.c_str(),
                    .size = player_dir_string.size() + 1,
                },
            };
            for (size_t i = 0; i < TUI_ARRAY_LENGTH(paths); i++)
            {
                PlayerPathWriter path = paths[i];
                *path.dest = (char *)BShip_Arena_Push(arena, path.size);
                memset(*path.dest, 0, path.size);
                memcpy(*path.dest, path.src, path.size);
            }
            ais.push_back(player);
            break;
        }
        closedir(player_dir_ref);
    }
    closedir(ai_dir_ref);

    sort(ais.begin(), ais.end(),
        [](const BShip_AIFileData& a, const BShip_AIFileData& b) {
            return strcasecmp(a.file_name, b.file_name) < 0;
        });

    return ais;
}

int main(void)
{
    bool debug = true;
    BShip_Arena string_arena = {};
    BShip_Arena_Initialize(&string_arena, 0); // 0 creates default size.
    vector<BShip_AIFileData> ais = GetAIs(&string_arena);

    TUI_Options options = {};
    bool should_exit = TUI_Options_Get(&options, ais, debug);
    if (should_exit)
    {
        goto on_error;
    }

    if (options.runtime == RUNTIME_MATCH)
    {
        size_t match_memory_size = BShip_Match_CalculateMemorySize(options.board_size, options.games_per_match);
        BShip_Arena arena = {};
        BShip_Arena_Initialize(&arena, match_memory_size);

        BShip_MatchData match = BShip_Match_Run(&arena, (char *)"/tmp/battleships.sock",
            options.ai1, options.ai2, options.board_size, options.games_per_match, debug);

        TUI_Match_Display(match, debug);

        BShip_Arena_Destroy(&arena);
    }
    else
    {
        printf("Unsupported runtime type\n");
    }

    BShip_Arena_Destroy(&string_arena);
    return 0;
on_error:
    BShip_Arena_Destroy(&string_arena);
    return 1;
}
