#include "game.h"

// Initialize maze with proper floor layouts
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    // Initialize all cells as invalid first
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                maze[f][w][l].is_valid = 0;
                maze[f][w][l].is_starting_area = 0;
                maze[f][w][l].has_wall = 0;
            }
        }
    }

    // Floor 0: 10x25 (entire floor is valid)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            maze[0][w][l].is_valid = 1;
        }
    }
    
    // Starting area: 4 blocks width x 9 blocks length, starting at position 8 along length
    for (int w = 3; w < 7; w++) {  // 4 blocks width (positions 3-6)
        for (int l = 8; l < 17; l++) {  // 9 blocks length starting at position 8
            maze[0][w][l].is_starting_area = 1;
        }
    }

    // Floor 1: Two 10x8 rectangular areas + bridge = 784 square feet total
    // First rectangle: 10x8 = 80*4 = 320 sq ft
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Second rectangle: 10x8 = 80*4 = 320 sq ft (positions 17-24)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Bridge: 4x9 = 36*4 = 144 sq ft (connecting the rectangles)
    // Total: 320 + 320 + 144 = 784 square feet
    for (int w = 3; w < 7; w++) {  // 4 blocks width
        for (int l = 8; l < 17; l++) {  // 9 blocks length
            maze[1][w][l].is_valid = 1;
        }
    }

    // Floor 2: Single 10x9 rectangular area above starting area
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {  // 9 blocks length starting at position 8
            maze[2][w][l].is_valid = 1;
        }
    }
}

// Initialize players with correct starting positions
void initialize_players(Player players[3]) {
    // Player A: starts at (0, 6, 12), first maze cell (0, 5, 12), direction North
    players[0].pos[0] = 0; players[0].pos[1] = 6; players[0].pos[2] = 12;
    players[0].in_game = 0;
    players[0].direction = DIR_NORTH;
    players[0].movement_points = 100;
    players[0].roll_count = 0;
    players[0].entered_maze = 0;

    // Player B: starts at (0, 9, 8), first maze cell (0, 9, 7), direction West
    players[1].pos[0] = 0; players[1].pos[1] = 9; players[1].pos[2] = 8;
    players[1].in_game = 0;
    players[1].direction = DIR_WEST;
    players[1].movement_points = 100;
    players[1].roll_count = 0;
    players[1].entered_maze = 0;

    // Player C: starts at (0, 9, 16), first maze cell (0, 9, 17), direction East
    players[2].pos[0] = 0; players[2].pos[1] = 9; players[2].pos[2] = 16;
    players[2].in_game = 0;
    players[2].direction = DIR_EAST;
    players[2].movement_points = 100;
    players[2].roll_count = 0;
    players[2].entered_maze = 0;
}

// Sample stairs initialization
void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 3;
    // Example stairs
    stairs[0] = (Stair){0, 2, 5, 1, 3, 10};
    stairs[1] = (Stair){1, 4, 8, 2, 4, 8};
    stairs[2] = (Stair){0, 8, 20, 1, 8, 20};
}

// Sample poles initialization
void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 2;
    poles[0] = (Pole){2, 0, 5, 15};
    poles[1] = (Pole){1, 0, 7, 22};
}

// Sample walls initialization
void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 2;
    walls[0] = (Wall){0, 2, 5, 8, 5};
    walls[1] = (Wall){1, 1, 3, 6, 3};
}

void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls) {
    do {
        flag[0] = rand() % NUM_FLOORS;
        flag[1] = rand() % FLOOR_WIDTH;
        flag[2] = rand() % FLOOR_LENGTH;
    } while (!maze[flag[0]][flag[1]][flag[2]].is_valid || 
             maze[flag[0]][flag[1]][flag[2]].is_starting_area);
}

// Robust dice rolling functions
int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

int roll_direction_dice(void) {
    int roll = (rand() % 6) + 1;
    switch (roll) {
        case 1: return DIR_EMPTY;
        case 2: return DIR_NORTH;
        case 3: return DIR_EAST;
        case 4: return DIR_SOUTH;
        case 5: return DIR_WEST;
        case 6: return DIR_EMPTY;
        default: return DIR_EMPTY;
    }
}

// Check if a cell is valid for the given floor
int is_valid_cell(int floor, int w, int l) {
    if (floor < 0 || floor >= NUM_FLOORS || w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) {
        return 0;
    }
    return 1;
}

// Check if player can move to a specific cell
int can_move_to(int floor, int w, int l, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    if (!is_valid_cell(floor, w, l)) return 0;
    return maze[floor][w][l].is_valid && !maze[floor][w][l].has_wall;
}

// Enter maze when player rolls 6
void enter_maze(Player *player, int player_id) {
    if (player->entered_maze) return;
    
    switch (player_id) {
        case PLAYER_A:
            player->pos[0] = 0; player->pos[1] = 5; player->pos[2] = 12;
            break;
        case PLAYER_B:
            player->pos[0] = 0; player->pos[1] = 9; player->pos[2] = 7;
            break;
        case PLAYER_C:
            player->pos[0] = 0; player->pos[1] = 9; player->pos[2] = 17;
            break;
    }
    player->in_game = 1;
    player->entered_maze = 1;
}

// Find stair at specific position
int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l) {
    for (int i = 0; i < num_stairs; i++) {
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l)) {
            return i;
        }
    }
    return -1;
}

// Find pole at specific position
int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].w == w && poles[i].l == l && 
            floor >= poles[i].end_floor && floor <= poles[i].start_floor) {
            return i;
        }
    }
    return -1;
}

// Check if wall blocks movement between two points
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                     int start_floor, int start_w, int start_l,
                     int end_floor, int end_w, int end_l) {
    // Simple implementation - check if destination has wall
    if (!can_move_to(end_floor, end_w, end_l, maze)) {
        return 1;
    }
    return 0;
}

// Move player with teleportation (stairs/poles)
void move_player_with_teleport(Player *player,
                               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls,
                               int steps) {
    if (!player->in_game || steps <= 0) return;
    
    int dx[] = {0, 1, 0, -1};  // North, East, South, West
    int dy[] = {-1, 0, 1, 0};
    
    for (int step = 0; step < steps; step++) {
        int new_w = player->pos[1] + dx[player->direction];
        int new_l = player->pos[2] + dy[player->direction];
        int new_floor = player->pos[0];
        
        // Check boundaries and validity
        if (!can_move_to(new_floor, new_w, new_l, maze)) {
            // Can't move further, stop here
            player->movement_points -= 2;
            printf("Movement blocked.\n");
            return;
        }
        
        // Move to new position
        player->pos[1] = new_w;
        player->pos[2] = new_l;
        
        // Check for stair/pole mid-movement
        int stair_idx = find_stair_at(stairs, num_stairs, new_floor, new_w, new_l);
        if (stair_idx != -1) {
            printf("Found stair! ");
            // Take the stair
            if (stairs[stair_idx].start_floor == new_floor && 
                stairs[stair_idx].start_w == new_w && stairs[stair_idx].start_l == new_l) {
                // At start of stair, go to end
                player->pos[0] = stairs[stair_idx].end_floor;
                player->pos[1] = stairs[stair_idx].end_w;
                player->pos[2] = stairs[stair_idx].end_l;
            } else {
                // At end of stair, go to start
                player->pos[0] = stairs[stair_idx].start_floor;
                player->pos[1] = stairs[stair_idx].start_w;
                player->pos[2] = stairs[stair_idx].start_l;
            }
            printf("Teleported to [%d,%d,%d]\n", player->pos[0], player->pos[1], player->pos[2]);
            continue; // Continue remaining movement from new position
        }
        
        int pole_idx = find_pole_at(poles, num_poles, new_floor, new_w, new_l);
        if (pole_idx != -1) {
            printf("Found pole! ");
            // Take the pole (always goes down)
            player->pos[0] = poles[pole_idx].end_floor;
            printf("Teleported to [%d,%d,%d]\n", player->pos[0], player->pos[1], player->pos[2]);
            // w, l stay same for poles
            continue; // Continue remaining movement from new position
        }
    }
}

// Check if player captured the flag
int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] && 
            player->pos[1] == flag[1] && 
            player->pos[2] == flag[2]);
}

// Note: Bawana implementation would have 12 cells (not 16)
// Located at boundary 6,20 to 9,20 with entrance at 9,19
// 3 cells each of: Food Poisoning, Disoriented, Triggered, Happy
// Total: 3*4 = 12 cells