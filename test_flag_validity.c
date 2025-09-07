#include "game.h"
#include <stdio.h>

static int is_excluded(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int f, int w, int l) {
    if (!maze[f][w][l].is_valid) return 1;
    if (maze[f][w][l].is_starting_area) return 1;
    if (maze[f][w][l].has_wall) return 1;
    if (maze[f][w][l].is_blocked_by_stair) return 1;
    if (maze[f][w][l].is_bawana_entrance) return 1;
    if (f == 0 && w >= 6 && w <= 9 && l >= 20 && l <= 24) return 1; // Bawana interior
    return 0;
}

int main(void) {
    int failures = 0;
    for (int t = 0; t < 200; t++) {
        int seed = 1000 + t;
        srand((unsigned int)seed);
        
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
        
        place_random_flag(flag, maze);
        int f = flag[0], w = flag[1], l = flag[2];
        
        if (is_excluded(maze, f, w, l)) {
            printf("✗ Invalid flag at [%d,%d,%d] on iteration %d\n", f, w, l, t);
            failures++;
            break;
        }
    }
    if (failures == 0) {
        printf("✓ Flag placement test passed across 200 iterations.\n");
    }
    return failures ? 1 : 0;
}
