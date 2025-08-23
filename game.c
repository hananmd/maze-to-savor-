#include "game.h"

// Initialize maze cells
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    srand((unsigned int)time(NULL));
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                maze[f][w][l].is_valid = 1;
                maze[f][w][l].is_starting_area = (f == 0 &&
                    w >= 6 && w <= 9 &&
                    l >= 8 && l <= 16) ? 1 : 0;
                maze[f][w][l].has_wall = 0;
            }
        }
    }
}

// Initialize players: Rule 1 setup
void initialize_players(Player players[3]) {
    // Player A: starts at 6,12 → enters 5,12 → facing North
    players[0].pos[0] = 0; players[0].pos[1] = 6; players[0].pos[2] = 12;
    players[0].direction = DIR_NORTH;

    // Player B: starts at 9,8 → enters 9,7 → facing West
    players[1].pos[0] = 0; players[1].pos[1] = 9; players[1].pos[2] = 8;
    players[1].direction = DIR_WEST;

    // Player C: starts at 9,16 → enters 9,17 → facing East
    players[2].pos[0] = 0; players[2].pos[1] = 9; players[2].pos[2] = 16;
    players[2].direction = DIR_EAST;

    // Common setup
    for (int i = 0; i < 3; i++) {
        players[i].in_game = 0;
        players[i].movement_points = 100;
        players[i].roll_count = 0;
    }
}

// Initialize stairs
void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 2;
    stairs[0] = (Stair){0, 5, 5, 1, 5, 5};
    stairs[1] = (Stair){1, 7, 7, 2, 7, 7};
}

// Initialize poles
void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 2;
    poles[0] = (Pole){1, 0, 2, 10};
    poles[1] = (Pole){2, 1, 3, 15};
}

// Initialize walls
void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 0; // no walls yet
}

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

// Enter maze
void enter_maze(Player *player, int player_id) {
    player->in_game = 1;
    // First maze entry cell based on player
    if (player_id == 0) { player->pos[1] = 5; player->pos[2] = 12; }
    if (player_id == 1) { player->pos[1] = 9; player->pos[2] = 7; }
    if (player_id == 2) { player->pos[1] = 9; player->pos[2] = 17; }
}

// Find stair index
int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l) {
    for (int i = 0; i < num_stairs; i++) {
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l))
            return i;
    }
    return -1;
}

// Find pole index
int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].start_floor == floor && poles[i].w == w && poles[i].l == l)
            return i;
    }
    return -1;
}

// Move player: Rule 2 strict + Rule 4 teleport continuation
void move_player_with_teleport(Player *player,
                               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls,
                               int steps) {
    for (int s = 0; s < steps; s++) {
        // Move one step
        switch (player->direction) {
            case DIR_NORTH: player->pos[1]--; break;
            case DIR_SOUTH: player->pos[1]++; break;
            case DIR_EAST:  player->pos[2]++; break;
            case DIR_WEST:  player->pos[2]--; break;
        }

        // Stop if boundary or wall hit before finishing steps
        if (player->pos[1] < 0 || player->pos[1] >= FLOOR_WIDTH ||
            player->pos[2] < 0 || player->pos[2] >= FLOOR_LENGTH ||
            maze[player->pos[0]][player->pos[1]][player->pos[2]].has_wall) {
            printf("⛔ Blocked — can't move full %d steps, staying put.\n", steps);
            return; // cancel entire move if cannot complete
        }

        // Check stairs
        int stair_index = find_stair_at(stairs, num_stairs,
                                        player->pos[0], player->pos[1], player->pos[2]);
        if (stair_index != -1) {
            if (stairs[stair_index].start_floor == player->pos[0]) {
                player->pos[0] = stairs[stair_index].end_floor;
                player->pos[1] = stairs[stair_index].end_w;
                player->pos[2] = stairs[stair_index].end_l;
            } else {
                player->pos[0] = stairs[stair_index].start_floor;
                player->pos[1] = stairs[stair_index].start_w;
                player->pos[2] = stairs[stair_index].start_l;
            }
            printf("🚶 Stair used → now at [%d,%d,%d]\n",
                   player->pos[0], player->pos[1], player->pos[2]);
            continue;
        }

        // Check poles
        int pole_index = find_pole_at(poles, num_poles,
                                      player->pos[0], player->pos[1], player->pos[2]);
        if (pole_index != -1 && player->pos[0] > 0) {
            player->pos[0] = poles[pole_index].end_floor;
            printf("🪂 Pole used → dropped to [%d,%d,%d]\n",
                   player->pos[0], player->pos[1], player->pos[2]);
            continue;
        }
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
