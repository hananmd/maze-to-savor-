// main.c
#include "game.h"
#include <stdio.h>

static void wait_for_enter(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    fgets(buffer, sizeof(buffer), stdin);
}

void play_turn(int player_id, Player players[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs, Pole poles[], int num_poles,
               Wall walls[], int num_walls, int flag[3]) {
    Player *p = &players[player_id];
    printf("\n=== Player %c's Turn ===\n", 'A' + player_id);

    wait_for_enter("Press Enter to roll movement die: ");
    int move_roll = roll_movement_dice();
    printf("Movement die: %d\n", move_roll);

    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf("Player %c enters the maze at [%d,%d,%d]\n", 'A'+player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter. Stay in starting area.\n", 'A'+player_id);
            p->movement_points -= 2;
            printf("Lost 2 movement points. Remaining: %d\n", p->movement_points);
            return;
        }
    }

    if (p->roll_count % 4 == 0) {
        int dir_roll = roll_direction_dice();
        if (dir_roll == 2) { p->direction = DIR_NORTH; printf("Direction changed to North\n"); }
        else if (dir_roll == 3) { p->direction = DIR_EAST;  printf("Direction changed to East\n"); }
        else if (dir_roll == 4) { p->direction = DIR_SOUTH; printf("Direction changed to South\n"); }
        else if (dir_roll == 5) { p->direction = DIR_WEST;  printf("Direction changed to West\n"); }
        else { printf("Direction unchanged\n"); }
    }

    if (p->bawana_effect == 2) {
        p->direction = rand() % 4;
        printf("🌀 Direction randomized due to disorientation.\n");
    } else if (p->bawana_effect == 3) {
        move_roll *= 2;
        printf("💥 Triggered! Movement doubled: %d steps\n", move_roll);
    }

    move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, move_roll);
    printf("Player %c moves to [%d,%d,%d]\n", 'A'+player_id, p->pos[0], p->pos[1], p->pos[2]);

    if (p->in_game) {
        Cell *cell = &maze[p->pos[0]][p->pos[1]][p->pos[2]];
        if (cell->bonus_value > 0) {
            p->movement_points += cell->bonus_value;
            printf("🎁 Bonus! +%d movement points. Total: %d\n", cell->bonus_value, p->movement_points);
        }
        if (cell->multiplier > 1) {
            printf("✨ Multiplier x%d applied!\n", cell->multiplier);
            p->movement_points *= cell->multiplier;
            printf("New movement points: %d\n", p->movement_points);
        }
    }

    if (p->movement_points <= 0) {
        reset_to_bawana(p);
        return;
    }

    check_player_capture(players, player_id);

    if (check_flag_capture(p, flag)) {
        printf("🎉 Player %c has captured the flag!\n", 'A' + player_id);
        printf("🏆 Player %c wins the game!\n", 'A' + player_id);
        exit(0);
    }

    p->roll_count++;
}

int main(void) {
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

    printf("=== Maze of UCSC ===\n");
    printf("Flag is at [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    printf("Players start with 100 movement points.\n\n");

    int round = 1;
    while (1) {
        printf("=== Round %d ===\n", round);
        update_stair_directions(stairs, num_stairs);

        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, flag);
        }
        printf("\n");
        round++;
    }

    return 0;
}