#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* robust input: read a number 1..6 from stdin */
static int read_dice_input(const char *prompt) {
    char buf[64];
    long v;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof buf, stdin)) {
            // EOF -> default to 1 to avoid undefined state
            return 1;
        }
        char *end = NULL;
        v = strtol(buf, &end, 10);
        if (end != buf && v >= 1 && v <= 6) return (int)v;
        printf("Please enter a number between 1 and 6.\n");
    }
}

void play_turn(int player_id,
               Player players[3],
               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs,
               Pole poles[], int num_poles,
               Wall walls[], int num_walls,
               int flag[3]) {
    Player *p = &players[player_id];
    printf("\n=== Player %c's turn ===\n", 'A' + player_id);

    /* MOVEMENT DIE: user input */
    int move_roll = read_dice_input("Enter MOVEMENT die (1-6): ");
    printf("Movement die: %d\n", move_roll);

    /* entry logic: needs a 6 to enter */
    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1; // first counted turn after entering
            printf("Player %c enters the maze at [%d,%d,%d]\n",
                   'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter.\n", 'A' + player_id);
        }
        return;
    }

    /* DIRECTION DIE: only every 4th roll */
    if (p->roll_count % 4 == 0) {
        int dir_roll = read_dice_input(
            "Enter DIRECTION die (1/6=Empty, 2=N, 3=E, 4=S, 5=W): ");
        if (dir_roll == 2)      { p->direction = DIR_NORTH; printf("Direction -> North\n"); }
        else if (dir_roll == 3) { p->direction = DIR_EAST;  printf("Direction -> East\n");  }
        else if (dir_roll == 4) { p->direction = DIR_SOUTH; printf("Direction -> South\n"); }
        else if (dir_roll == 5) { p->direction = DIR_WEST;  printf("Direction -> West\n");  }
        else { printf("Direction unchanged\n"); }
    } else {
        printf("(Direction die not rolled this turn)\n");
    }

    /* move with Rule 2 (no partial moves) and Rule 4 (mid-move stair/pole) */
    move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles,
                              walls, num_walls, move_roll);

    printf("Player %c at [%d,%d,%d]\n",
           'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);

    if (check_flag_capture(p, flag)) {
        printf("🎉 Player %c captured the flag!\n", 'A' + player_id);
        printf("🏆 Player %c wins!\n", 'A' + player_id);
        exit(0);
    }

    p->roll_count++;
}

int main(void) {
    /* srand still fine (not used for dice now, but used for flag placement) */
    srand((unsigned int)time(NULL));

    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;
    int flag[3];

    initialize_maze(maze);
    initialize_players(players);
    initialize_stairs(stairs, &num_stairs);
    initialize_poles(poles, &num_poles);
    initialize_walls(walls, &num_walls);
    place_random_flag(flag, maze, walls, num_walls);

    printf("Maze Game (manual dice mode)\n");
    printf("Flag is at [%d,%d,%d]\n\n", flag[0], flag[1], flag[2]);

    while (1) {
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs,
                      poles, num_poles, walls, num_walls, flag);
        }
    }
    return 0;
}
