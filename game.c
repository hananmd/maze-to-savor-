// game.c
#include "game.h"
#include <string.h>

void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    memset(maze, 0, sizeof(Cell) * NUM_FLOORS * FLOOR_WIDTH * FLOOR_LENGTH);

    // Floor 0: 214 usable blocks, starting area [6-9,8-16] is restricted
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0;
            } else {
                maze[0][w][l].is_valid = 1;
            }
            maze[0][w][l].has_wall = 0;
        }
    }

    // Floor 1: 784 sq ft = 196 blocks
    for (int w = 0; w < 10; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
        }
    }
    for (int w = 0; w < 10; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
        }
    }
    for (int w = 6; w < 10; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
        }
    }

    // Floor 2: 360 sq ft = 90 blocks
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {
            maze[2][w][l].is_valid = 1;
            maze[2][w][l].has_wall = 0;
        }
    }
}

void initialize_players(Player players[3]) {
    players[0].pos[0] = 0; players[0].pos[1] = 6; players[0].pos[2] = 12; players[0].in_game = 0;
    players[1].pos[0] = 0; players[1].pos[1] = 9; players[1].pos[2] = 8;  players[1].in_game = 0;
    players[2].pos[0] = 0; players[2].pos[1] = 9; players[2].pos[2] = 16; players[2].in_game = 0;
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 0;
    stairs[(*num_stairs)++] = (Stair){0, 5, 10, 1, 5, 10};
    stairs[(*num_stairs)++] = (Stair){1, 7, 12, 2, 7, 12};
}

void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 0;
    poles[(*num_poles)++] = (Pole){2, 0, 5, 24};
}

void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 0;

    // Temporary wall: horizontal wall on Floor 0 from [6,20] to [9,20] (Bawana boundary)
    walls[(*num_walls)++] = (Wall){0, 6, 20, 9, 20};

    // Temporary wall: vertical wall on Floor 0 from [6,20] to [6,24]
    walls[(*num_walls)++] = (Wall){0, 6, 20, 6, 24};

    // Example from doc: [1, 0, 2, 8, 2] → Floor 1, from [0,2] to [8,2]
    walls[(*num_walls)++] = (Wall){1, 0, 2, 8, 2};
}

void apply_stair_teleport(Player *player, Stair stairs[], int num_stairs) {
    int f = player->pos[0];
    int w = player->pos[1];
    int l = player->pos[2];

    for (int i = 0; i < num_stairs; i++) {
        Stair s = stairs[i];
        if (f == s.start_floor && w == s.start_w && l == s.start_l) {
            player->pos[0] = s.end_floor;
            player->pos[1] = s.end_w;
            player->pos[2] = s.end_l;
            return;
        }
        if (f == s.end_floor && w == s.end_w && l == s.end_l) {
            player->pos[0] = s.start_floor;
            player->pos[1] = s.start_w;
            player->pos[2] = s.start_l;
            return;
        }
    }
}

void apply_pole_teleport(Player *player, Pole poles[], int num_poles) {
    int f = player->pos[0];
    int w = player->pos[1];
    int l = player->pos[2];

    for (int i = 0; i < num_poles; i++) {
        Pole p = poles[i];
        if (w == p.w && l == p.l) {
            if (f <= p.start_floor && f >= p.end_floor) {
                player->pos[0] = p.end_floor;
                return;
            }
        }
    }
}

// Check if a wall blocks movement between two adjacent cells
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2) {
    // Only check if cells are adjacent
    if (!((w1 == w2 && abs(l1 - l2) == 1) || (l1 == l2 && abs(w1 - w2) == 1))) {
        return 0;
    }

    Wall walls[MAX_WALLS];
    int num_walls;
    initialize_walls(walls, &num_walls);

    for (int i = 0; i < num_walls; i++) {
        Wall wall = walls[i];
        if (wall.floor != floor) continue;

        // Horizontal wall (fixed l)
        if (wall.start_l == wall.end_l && wall.start_w != wall.end_w) {
            int l_wall = wall.start_l;
            int w_min = (wall.start_w < wall.end_w) ? wall.start_w : wall.end_w;
            int w_max = (wall.start_w > wall.end_w) ? wall.start_w : wall.end_w;
            if (l1 == l_wall && l2 == l_wall) {
                int w_min_c = (w1 < w2) ? w1 : w2;
                int w_max_c = (w1 > w2) ? w1 : w2;
                if (w_max_c == w_min_c + 1 && w_max_c <= w_max && w_min_c >= w_min) {
                    return 1;
                }
            }
        }

        // Vertical wall (fixed w)
        if (wall.start_w == wall.end_w && wall.start_l != wall.end_l) {
            int w_wall = wall.start_w;
            int l_min = (wall.start_l < wall.end_l) ? wall.start_l : wall.end_l;
            int l_max = (wall.start_l > wall.end_l) ? wall.start_l : wall.end_l;
            if (w1 == w_wall && w2 == w_wall) {
                int l_min_c = (l1 < l2) ? l1 : l2;
                int l_max_c = (l1 > l2) ? l1 : l2;
                if (l_max_c == l_min_c + 1 && l_max_c <= l_max && l_min_c >= l_min) {
                    return 1;
                }
            }
        }
    }
    
    int check_flag_capture(Player *player, int flag[3]) {
        if (player->pos[0] == flag[0] &&
            player->pos[1] == flag[1] &&
            player->pos[2] == flag[2]) {
            return 1;
        }
        return 0;
    }
    
    // Check if any player has captured the flag
    int is_game_over(Player players[3], int flag[3]) {
        for (int i = 0; i < 3; i++) {
            if (!players[i].captured && check_flag_capture(&players[i], flag)) {
                printf("🎉 Player %c has captured the flag and wins the game!\n", 'A' + i);
                return 1;
            }
        }
        return 0;
    }
}