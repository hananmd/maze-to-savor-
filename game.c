#include "game.h"

// Initialize maze cells
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    srand((unsigned int)time(NULL));
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                maze[f][w][l].is_valid = 1;
                maze[f][w][l].is_starting_area = (f == 0 &&
                    w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                    l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) ? 1 : 0;
                maze[f][w][l].has_wall = 0;
            }
        }
    }
}

// Initialize players
void initialize_players(Player players[3]) {
    for (int i = 0; i < 3; i++) {
        players[i].pos[0] = 0;
        players[i].pos[1] = START_AREA_W_MIN + i;
        players[i].pos[2] = START_AREA_L_MIN;
        players[i].in_game = 0;
        players[i].direction = DIR_EAST;
        players[i].movement_points = 100;
        players[i].roll_count = 0;
    }
}

// Initialize stairs, poles, and walls
void initialize_stairs(Stair stairs[], int *num_stairs) { *num_stairs = 0; }
void initialize_poles(Pole poles[], int *num_poles) { *num_poles = 0; }
void initialize_walls(Wall walls[], int *num_walls) { *num_walls = 0; }

// Roll movement dice
int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

// Roll direction dice
int roll_direction_dice(void) {
    return (rand() % 6) + 1;
}

// Check if a wall blocks movement
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                     int floor, int w1, int l1, int w2, int l2) {
    if (w2 < 0 || w2 >= FLOOR_WIDTH || l2 < 0 || l2 >= FLOOR_LENGTH)
        return 1;
    return maze[floor][w2][l2].has_wall;
}

// Enter maze logic
void enter_maze(Player *player, int player_id) {
    player->in_game = 1;
    player->pos[0] = 0;
    player->pos[1] = START_AREA_W_MIN + player_id;
    player->pos[2] = START_AREA_L_MIN;
}

// Move player step-by-step with teleportation
void move_player_with_teleport(Player *player,
                               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls,
                               int steps) {
    int floor = player->pos[0];
    int w = player->pos[1];
    int l = player->pos[2];

    for (int s = 0; s < steps; s++) {
        switch (player->direction) {
            case DIR_NORTH: w--; break;
            case DIR_SOUTH: w++; break;
            case DIR_EAST:  l++; break;
            case DIR_WEST:  l--; break;
        }

        if (w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) break;
        if (maze[floor][w][l].has_wall) break;

        player->pos[1] = w;
        player->pos[2] = l;
    }
}

// Place random flag
void place_random_flag(int flag[3],
                       Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                       Wall walls[], int num_walls) {
    do {
        flag[0] = rand() % NUM_FLOORS;
        flag[1] = rand() % FLOOR_WIDTH;
        flag[2] = rand() % FLOOR_LENGTH;
    } while (maze[flag[0]][flag[1]][flag[2]].has_wall);
}

// Check if player captured flag
int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] &&
            player->pos[1] == flag[1] &&
            player->pos[2] == flag[2]);
}
