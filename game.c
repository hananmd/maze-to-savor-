// game.c
#include "game.h"

void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    memset(maze, 0, sizeof(Cell) * NUM_FLOORS * FLOOR_WIDTH * FLOOR_LENGTH);

    // Floor 0: 10×25 grid (assignment specifies dimensions)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            // Starting area: 4 blocks width (6-9), 9 blocks length (8-16)
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0; // Players can't move through starting area
            } else {
                maze[0][w][l].is_valid = 1;
            }
            maze[0][w][l].has_wall = 0;
            maze[0][w][l].bawana_cell_type = -1;
            maze[0][w][l].consumable_value = 0;
            maze[0][w][l].bonus_value = 0;
            maze[0][w][l].multiplier = 1;
        }
    } 

    // Floor 1: Two rectangular areas (10×8) connected by bridge (4×9)
    // First area: [1,0,0] to [1,9,7]
    for (int w = 0; w < 10; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Bridge: [1,6,8] to [1,9,16] 
    for (int w = 6; w < 10; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Second area: [1,0,17] to [1,9,24]
    for (int w = 0; w < 10; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }

    // Floor 2: Single rectangular area (10×9) above standing area
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) { // Above the starting area
            maze[2][w][l].is_valid = 1;
        }
    }

    // Initialize all valid cells with default values
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid) {
                    maze[f][w][l].has_wall = 0;
                    maze[f][w][l].bawana_cell_type = -1;
                    maze[f][w][l].consumable_value = 0;
                    maze[f][w][l].bonus_value = 0;
                    maze[f][w][l].multiplier = 1;
                }
            }
        }
    }

    // Bawana area setup (Rule 7): [0,6,20] to [0,9,24] with walls
    for (int w = 6; w <= 9; w++) {
        for (int l = 21; l <= 24; l++) { // Interior cells only
            maze[0][w][l].is_valid = 1;
        }
    }

    // Bawana walls
    for (int w = 6; w <= 9; w++) {
        maze[0][w][20].has_wall = 1; // South wall
    }
    for (int l = 20; l <= 24; l++) {
        maze[0][6][l].has_wall = 1; // West wall
    }

    // Bawana cell effects: 2 of each type + 4 random MP cells
    int bawana_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };

    // Shuffle bawana cells for random assignment
    for (int i = 0; i < 12; i++) {
        int j = rand() % 12;
        int temp_w = bawana_cells[i][0];
        int temp_l = bawana_cells[i][1];
        bawana_cells[i][0] = bawana_cells[j][0];
        bawana_cells[i][1] = bawana_cells[j][1];
        bawana_cells[j][0] = temp_w;
        bawana_cells[j][1] = temp_l;
    }

    // Assign 2 of each type
    int cell_idx = 0;
    for (int type = 0; type < 4; type++) {
        for (int count = 0; count < 2; count++) {
            maze[0][bawana_cells[cell_idx][0]][bawana_cells[cell_idx][1]].bawana_cell_type = type;
            cell_idx++;
        }
    }
    // Assign remaining 4 as random MP cells
    for (int i = 0; i < 4; i++) {
        maze[0][bawana_cells[cell_idx][0]][bawana_cells[cell_idx][1]].bawana_cell_type = BA_RANDOM_MP;
        cell_idx++;
    }

    // Rule 10: Cell value distribution
    int total_cells = 0;
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid && maze[f][w][l].bawana_cell_type == -1) {
                    total_cells++;
                }
            }
        }
    }

    // Create array of all valid non-Bawana cells
    int valid_cells[total_cells][3];
    int idx = 0;
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid && maze[f][w][l].bawana_cell_type == -1) {
                    valid_cells[idx][0] = f;
                    valid_cells[idx][1] = w;
                    valid_cells[idx][2] = l;
                    idx++;
                }
            }
        }
    }

    // Shuffle cells for random assignment
    for (int i = 0; i < total_cells; i++) {
        int j = rand() % total_cells;
        int temp[3] = {valid_cells[i][0], valid_cells[i][1], valid_cells[i][2]};
        valid_cells[i][0] = valid_cells[j][0];
        valid_cells[i][1] = valid_cells[j][1];
        valid_cells[i][2] = valid_cells[j][2];
        valid_cells[j][0] = temp[0];
        valid_cells[j][1] = temp[1];
        valid_cells[j][2] = temp[2];
    }

    // Assign cell values according to Rule 10
    idx = 0;
    
    // 25% consumable value 0 (already initialized to 0)
    int n_zero = total_cells * 0.25;
    idx += n_zero;

    // 35% consumable value 1-4
    int n_consumable = total_cells * 0.35;
    for (int i = 0; i < n_consumable && idx < total_cells; i++, idx++) {
        int f = valid_cells[idx][0];
        int w = valid_cells[idx][1];
        int l = valid_cells[idx][2];
        maze[f][w][l].consumable_value = (rand() % 4) + 1;
    }

    // 25% bonus 1-2
    int n_bonus12 = total_cells * 0.25;
    for (int i = 0; i < n_bonus12 && idx < total_cells; i++, idx++) {
        int f = valid_cells[idx][0];
        int w = valid_cells[idx][1];
        int l = valid_cells[idx][2];
        maze[f][w][l].bonus_value = (rand() % 2) + 1;
    }

    // 10% bonus 3-5
    int n_bonus35 = total_cells * 0.10;
    for (int i = 0; i < n_bonus35 && idx < total_cells; i++, idx++) {
        int f = valid_cells[idx][0];
        int w = valid_cells[idx][1];
        int l = valid_cells[idx][2];
        maze[f][w][l].bonus_value = (rand() % 3) + 3;
    }

    // 5% multiplier 2-3
    int n_mult = total_cells * 0.05;
    for (int i = 0; i < n_mult && idx < total_cells; i++, idx++) {
        int f = valid_cells[idx][0];
        int w = valid_cells[idx][1];
        int l = valid_cells[idx][2];
        maze[f][w][l].multiplier = (rand() % 2) + 2;
    }
}

void initialize_players(Player players[3]) {
    // Player A: starting area [0,6,12], first maze cell [0,5,12], direction North
    players[PLAYER_A].pos[0] = 0; 
    players[PLAYER_A].pos[1] = 6; 
    players[PLAYER_A].pos[2] = 12;
    players[PLAYER_A].in_game = 0;
    players[PLAYER_A].direction = DIR_NORTH;
    players[PLAYER_A].movement_points = 100;
    players[PLAYER_A].roll_count = 0;
    players[PLAYER_A].captured = 0;
    players[PLAYER_A].capture_start_pos[0] = 0; 
    players[PLAYER_A].capture_start_pos[1] = 6; 
    players[PLAYER_A].capture_start_pos[2] = 12;
    players[PLAYER_A].bawana_effect = 0;
    players[PLAYER_A].bawana_turns_left = 0;
    players[PLAYER_A].bawana_random_mp = 0;
    players[PLAYER_A].disoriented_turns = 0;
    players[PLAYER_A].food_poisoning_turns = 0;

    // Player B: starting area [0,9,8], first maze cell [0,9,7], direction West
    players[PLAYER_B].pos[0] = 0; 
    players[PLAYER_B].pos[1] = 9; 
    players[PLAYER_B].pos[2] = 8;
    players[PLAYER_B].in_game = 0;
    players[PLAYER_B].direction = DIR_WEST;
    players[PLAYER_B].movement_points = 100;
    players[PLAYER_B].roll_count = 0;
    players[PLAYER_B].captured = 0;
    players[PLAYER_B].capture_start_pos[0] = 0; 
    players[PLAYER_B].capture_start_pos[1] = 9; 
    players[PLAYER_B].capture_start_pos[2] = 8;
    players[PLAYER_B].bawana_effect = 0;
    players[PLAYER_B].bawana_turns_left = 0;
    players[PLAYER_B].bawana_random_mp = 0;
    players[PLAYER_B].disoriented_turns = 0;
    players[PLAYER_B].food_poisoning_turns = 0;

    // Player C: starting area [0,9,16], first maze cell [0,9,17], direction East
    players[PLAYER_C].pos[0] = 0; 
    players[PLAYER_C].pos[1] = 9; 
    players[PLAYER_C].pos[2] = 16;
    players[PLAYER_C].in_game = 0;
    players[PLAYER_C].direction = DIR_EAST;
    players[PLAYER_C].movement_points = 100;
    players[PLAYER_C].roll_count = 0;
    players[PLAYER_C].captured = 0;
    players[PLAYER_C].capture_start_pos[0] = 0; 
    players[PLAYER_C].capture_start_pos[1] = 9; 
    players[PLAYER_C].capture_start_pos[2] = 16;
    players[PLAYER_C].bawana_effect = 0;
    players[PLAYER_C].bawana_turns_left = 0;
    players[PLAYER_C].bawana_random_mp = 0;
    players[PLAYER_C].disoriented_turns = 0;
    players[PLAYER_C].food_poisoning_turns = 0;
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 0;
    
    // Example stair from ground floor to floor 1
    stairs[(*num_stairs)].start_floor = 0;
    stairs[(*num_stairs)].start_w = 5;
    stairs[(*num_stairs)].start_l = 10;
    stairs[(*num_stairs)].end_floor = 1;
    stairs[(*num_stairs)].end_w = 5;
    stairs[(*num_stairs)].end_l = 5;
    stairs[(*num_stairs)].direction_type = STAIR_BIDIRECTIONAL;
    (*num_stairs)++;

    // Example stair from floor 1 to floor 2
    stairs[(*num_stairs)].start_floor = 1;
    stairs[(*num_stairs)].start_w = 7;
    stairs[(*num_stairs)].start_l = 12;
    stairs[(*num_stairs)].end_floor = 2;
    stairs[(*num_stairs)].end_w = 7;
    stairs[(*num_stairs)].end_l = 12;
    stairs[(*num_stairs)].direction_type = STAIR_BIDIRECTIONAL;
    (*num_stairs)++;
}

void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 0;
    
    // Example pole from floor 2 to ground floor
    poles[(*num_poles)].start_floor = 2;
    poles[(*num_poles)].end_floor = 0;
    poles[(*num_poles)].w = 5;
    poles[(*num_poles)].l = 15;
    (*num_poles)++;
}

void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 0;
    
    // Bawana walls
    walls[(*num_walls)].floor = 0;
    walls[(*num_walls)].start_w = 6;
    walls[(*num_walls)].start_l = 20;
    walls[(*num_walls)].end_w = 9;
    walls[(*num_walls)].end_l = 20;
    (*num_walls)++;

    walls[(*num_walls)].floor = 0;
    walls[(*num_walls)].start_w = 6;
    walls[(*num_walls)].start_l = 20;
    walls[(*num_walls)].end_w = 6;
    walls[(*num_walls)].end_l = 24;
    (*num_walls)++;

    // Example maze wall
    walls[(*num_walls)].floor = 1;
    walls[(*num_walls)].start_w = 0;
    walls[(*num_walls)].start_l = 2;
    walls[(*num_walls)].end_w = 8;
    walls[(*num_walls)].end_l = 2;
    (*num_walls)++;
}

int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

int roll_direction_dice(void) {
    return (rand() % 6) + 1; // 1,6 = empty, 2=N, 3=E, 4=S, 5=W
}

void enter_maze(Player *player, int player_id) {
    switch (player_id) {
        case PLAYER_A: 
            player->pos[1] = 5; 
            player->pos[2] = 12; 
            break;
        case PLAYER_B: 
            player->pos[1] = 9; 
            player->pos[2] = 7;  
            break;
        case PLAYER_C: 
            player->pos[1] = 9; 
            player->pos[2] = 17; 
            break;
    }
    player->in_game = 1;
}

int is_valid_position(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w, int l) {
    if (floor < 0 || floor >= NUM_FLOORS) return 0;
    if (w < 0 || w >= FLOOR_WIDTH) return 0;
    if (l < 0 || l >= FLOOR_LENGTH) return 0;
    return maze[floor][w][l].is_valid;
}

int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2) {
    // Check if movement from (w1,l1) to (w2,l2) is blocked by a wall
    if (!((w1 == w2 && abs(l1 - l2) == 1) || (l1 == l2 && abs(w1 - w2) == 1))) {
        return 0; // Not adjacent cells
    }

    // Check if destination has a wall
    if (!is_valid_position(maze, floor, w2, l2)) return 1;
    if (maze[floor][w2][l2].has_wall) return 1;

    return 0;
}

int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l) {
    for (int i = 0; i < num_stairs; i++) {
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l)) {
            return i;
        }
    }
    return -1;
}

int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].w == w && poles[i].l == l && 
            (floor == poles[i].start_floor || 
             (floor >= poles[i].end_floor && floor <= poles[i].start_floor))) {
            return i;
        }
    }
    return -1;
}

void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    int cell_type = maze[player->pos[0]][player->pos[1]][player->pos[2]].bawana_cell_type;
    
    switch (cell_type) {
        case BA_FOOD_POISONING:
            printf(" Food Poisoning! Player misses next 3 turns.\n");
            player->food_poisoning_turns = 3;
            player->bawana_effect = 1;
            break;
            
        case BA_DISORIENTED:
            printf(" Disoriented! +50 MP, moved to entrance, next 4 moves random.\n");
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19; // Entrance
            player->direction = DIR_NORTH;
            player->disoriented_turns = 4;
            player->bawana_effect = 2;
            break;
            
        case BA_TRIGGERED:
            printf(" Triggered! +50 MP, moved to entrance, moves at double speed.\n");
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19; // Entrance
            player->direction = DIR_NORTH;
            player->bawana_effect = 3;
            break;
            
        case BA_HAPPY:
            printf(" Happy! +200 MP, moved to entrance.\n");
            player->movement_points += 200;
            player->pos[1] = 9; player->pos[2] = 19; // Entrance
            player->direction = DIR_NORTH;
            player->bawana_effect = 4;
            break;
            
        case BA_RANDOM_MP: {
            int bonus = (rand() % 91) + 10; // 10-100 MP
            printf(" Random bonus! +%d MP.\n", bonus);
            player->movement_points += bonus;
            break;
        }
    }
}

void handle_bawana_turns(Player *player) {
    if (player->food_poisoning_turns > 0) {
        player->food_poisoning_turns--;
        if (player->food_poisoning_turns == 0) {
            printf(" Food poisoning effect ended.\n");
            player->bawana_effect = 0;
            // Randomly place in Bawana again
            int bawana_cells[12][2] = {
                {6,21}, {6,22}, {6,23}, {6,24},
                {7,21}, {7,22}, {7,23}, {7,24},
                {8,21}, {8,22}, {8,23}, {8,24}
            };
            int idx = rand() % 12;
            player->pos[1] = bawana_cells[idx][0];
            player->pos[2] = bawana_cells[idx][1];
        }
        return; // Skip turn
    }
    
    if (player->disoriented_turns > 0) {
        player->disoriented_turns--;
        if (player->disoriented_turns == 0) {
            printf(" Disorientation effect ended.\n");
            player->bawana_effect = 0;
        }
    }
}

void move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps) {
    
    // Handle Bawana effects first
    if (player->bawana_effect == 3) { // Triggered - double movement
        steps *= 2;
    }
    
    for (int s = 0; s < steps; s++) {
        int f = player->pos[0];
        int w = player->pos[1];
        int l = player->pos[2];
        int new_w = w, new_l = l;

        // Determine movement direction
        int move_dir = player->direction;
        if (player->disoriented_turns > 0) {
            move_dir = rand() % 4; // Random direction when disoriented
        }

        // Calculate new position
        switch (move_dir) {
            case DIR_NORTH: new_l = l - 1; break;
            case DIR_EAST:  new_w = w + 1; break;
            case DIR_SOUTH: new_l = l + 1; break;
            case DIR_WEST:  new_w = w - 1; break;
        }

        // Check boundaries and validity
        if (!is_valid_position(maze, f, new_w, new_l)) {
            printf(" Movement blocked - invalid position.\n");
            return; // Stop movement entirely
        }

        // Check wall blocking
        if (is_wall_blocking(maze, f, w, l, new_w, new_l)) {
            printf(" Movement blocked by wall.\n");
            player->movement_points -= 2; // Penalty for blocked movement
            return; // Stop movement entirely
        }

        // Move to new position
        player->pos[1] = new_w;
        player->pos[2] = new_l;

        // Apply consumable cost (Rule 8)
        int consumable = maze[f][new_w][new_l].consumable_value;
        player->movement_points -= consumable;
        if (consumable > 0) {
            printf(" Consumed %d movement points. Remaining: %d\n", consumable, player->movement_points);
        }

        // Check if movement points exhausted (Rule 11)
        if (player->movement_points <= 0) {
            printf(" Movement points exhausted! Transported to Bawana.\n");
            reset_to_bawana(player);
            return;
        }

        // Rule 4: Check for stairs/poles mid-movement and handle teleportation
        int stair_index = find_stair_at(stairs, num_stairs, f, new_w, new_l);
        if (stair_index != -1) {
            Stair stair = stairs[stair_index];
            int teleported = 0;
            
            if (stair.direction_type == STAIR_UP_ONLY && f == stair.start_floor) {
                player->pos[0] = stair.end_floor;
                player->pos[1] = stair.end_w;
                player->pos[2] = stair.end_l;
                teleported = 1;
                printf(" Took stairs up to floor %d [%d,%d]\n", stair.end_floor, stair.end_w, stair.end_l);
            } else if (stair.direction_type == STAIR_DOWN_ONLY && f == stair.end_floor) {
                player->pos[0] = stair.start_floor;
                player->pos[1] = stair.start_w;
                player->pos[2] = stair.start_l;
                teleported = 1;
                printf(" Took stairs down to floor %d [%d,%d]\n", stair.start_floor, stair.start_w, stair.start_l);
            } else if (stair.direction_type == STAIR_BIDIRECTIONAL) {
                if (f == stair.start_floor) {
                    player->pos[0] = stair.end_floor;
                    player->pos[1] = stair.end_w;
                    player->pos[2] = stair.end_l;
                    printf(" Took stairs up to floor %d [%d,%d]\n", stair.end_floor, stair.end_w, stair.end_l);
                } else {
                    player->pos[0] = stair.start_floor;
                    player->pos[1] = stair.start_w;
                    player->pos[2] = stair.start_l;
                    printf(" Took stairs down to floor %d [%d,%d]\n", stair.start_floor, stair.start_w, stair.start_l);
                }
                teleported = 1;
            }
            
            if (teleported) {
                // Continue remaining movement from new position
                f = player->pos[0];
                continue;
            }
        }

        // Check for pole
        int pole_index = find_pole_at(poles, num_poles, f, new_w, new_l);
        if (pole_index != -1) {
            Pole pole = poles[pole_index];
            if (f >= pole.end_floor && f <= pole.start_floor) {
                player->pos[0] = pole.end_floor;
                player->pos[1] = pole.w;
                player->pos[2] = pole.l;
                printf(" Slid down pole to floor %d [%d,%d]\n", pole.end_floor, pole.w, pole.l);
                f = player->pos[0];
                continue;
            }
        }

        // Check if landed in Bawana area
        if (f == 0 && new_w >= 6 && new_w <= 9 && new_l >= 21 && new_l <= 24) {
            if (maze[f][new_w][new_l].bawana_cell_type != -1) {
                printf(" Landed in Bawana!\n");
                apply_bawana_effect(player, maze);
                return;
            }
        }
    }
}

void place_random_flag(int flag[3], 
    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
    Stair stairs[], int num_stairs,
    Pole poles[], int num_poles,
    Wall walls[], int num_walls) {
int valid_positions[1000][3];
int count = 0;

for (int f = 0; f < NUM_FLOORS; f++) {
for (int w = 0; w < FLOOR_WIDTH; w++) {
for (int l = 0; l < FLOOR_LENGTH; l++) {
if (!maze[f][w][l].is_valid ||
 maze[f][w][l].is_starting_area ||
 maze[f][w][l].has_wall ||
 maze[f][w][l].bawana_cell_type != -1) {
 continue;
}

if (find_stair_at(stairs, num_stairs, f, w, l) != -1) continue;
if (find_pole_at(poles, num_poles, f, w, l) != -1) continue;

valid_positions[count][0] = f;
valid_positions[count][1] = w;
valid_positions[count][2] = l;
count++;
        }
    }
}


    if (count == 0) {
        printf("Error: No valid cells for flag placement!\n");
        exit(1);
    }

    int choice = rand() % count;
    flag[0] = valid_positions[choice][0];
    flag[1] = valid_positions[choice][1];
    flag[2] = valid_positions[choice][2];
}

int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] &&
            player->pos[1] == flag[1] &&
            player->pos[2] == flag[2]);
}

void check_player_capture(Player players[3], int current_player) {
    Player *current = &players[current_player];
    
    for (int i = 0; i < 3; i++) {
        if (i == current_player || !players[i].in_game) continue;
        
        if (current->pos[0] == players[i].pos[0] &&
            current->pos[1] == players[i].pos[1] &&
            current->pos[2] == players[i].pos[2]) {
            
            printf(" Player %c captures Player %c!\n", 'A' + current_player, 'A' + i);
            
            // Send captured player back to starting area
            players[i].in_game = 0;
            players[i].captured = 1;
            players[i].pos[0] = players[i].capture_start_pos[0];
            players[i].pos[1] = players[i].capture_start_pos[1];
            players[i].pos[2] = players[i].capture_start_pos[2];
            players[i].roll_count = 0;
            
            // Reset Bawana effects
            players[i].bawana_effect = 0;
            players[i].bawana_turns_left = 0;
            players[i].disoriented_turns = 0;
            players[i].food_poisoning_turns = 0;
            
            printf(" Player %c sent back to starting area [%d,%d,%d]\n", 
                   'A' + i, players[i].pos[0], players[i].pos[1], players[i].pos[2]);
            return;
        }
    }
}
//Block intermediate stair cells
void mark_vertical_stair_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
    Stair stairs[], int num_stairs) {
for (int i = 0; i < num_stairs; i++) {
int sf = stairs[i].start_floor, ef = stairs[i].end_floor;
int sw = stairs[i].start_w, sl = stairs[i].start_l;
int ew = stairs[i].end_w, el = stairs[i].end_l;

if (sw != ew || sl != el) continue;

int floor_min = (sf < ef) ? sf : ef;
int floor_max = (sf > ef) ? sf : ef;

for (int f = floor_min + 1; f < floor_max; f++) {
if (f >= 0 && f < NUM_FLOORS) {
maze[f][sw][sl].has_wall = 1;}
                                
         }
    }
 }
void update_stair_directions(Stair stairs[], int num_stairs) {
    static int last_update_round = 0;
    static int current_round = 0;
    current_round++;
    
    if (current_round - last_update_round >= 5) {
        for (int i = 0; i < num_stairs; i++) {
            stairs[i].direction_type = rand() % 3;
        }
        printf(" Stair configurations have been updated.\n");
        last_update_round = current_round;
    }
}

void reset_to_bawana(Player *player) {
    // Transport to Bawana entrance
    player->pos[0] = 0;
    player->pos[1] = 9;
    player->pos[2] = 19;
    player->direction = DIR_NORTH;
    player->in_game = 1;
    
    // Reset movement points to minimum 1
    if (player->movement_points <= 0) {
        player->movement_points = 1;
    }
    
    printf(" Player transported to Bawana entrance [0,9,19]\n");
}