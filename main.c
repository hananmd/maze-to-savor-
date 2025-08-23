#include "game.h"

void play_turn(int player_id,
               Player players[3],
               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs,
               Pole poles[], int num_poles,
               Wall walls[], int num_walls,
               int flag[3]) {
    Player *p = &players[player_id];
    printf("Player %c's turn\n", 'A' + player_id);

    int move_roll = roll_movement_dice();
    printf("Rolled %d\n", move_roll);

    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf("Player %c enters the maze at [%d,%d,%d]\n",
                   'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter. Rolled %d.\n",
                   'A' + player_id, move_roll);
        }
        return;
    }

    // Direction dice every 4th roll
    if (p->roll_count % 4 == 0) {
        int dir_roll = roll_direction_dice();
        if (dir_roll == 2) { p->direction = DIR_NORTH; printf("Direction changed to North\n"); }
        else if (dir_roll == 3) { p->direction = DIR_EAST; printf("Direction changed to East\n"); }
        else if (dir_roll == 4) { p->direction = DIR_SOUTH; printf("Direction changed to South\n"); }
        else if (dir_roll == 5) { p->direction = DIR_WEST; printf("Direction changed to West\n"); }
        else { printf("Direction unchanged\n"); }
    }

    // Move player with teleport support
    move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, move_roll);

    printf("Player %c moves to [%d,%d,%d]\n",
           'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);

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

    printf("Maze of UCSC\n");
    printf("Flag is at [%d,%d,%d]\n\n", flag[0], flag[1], flag[2]);

    while (1) {
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, flag);
        }
        printf("\n");
    }

    return 0;
}
