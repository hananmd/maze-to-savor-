// game.c
#include "game.h"

void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    // Initialize all cells first
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                maze[f][w][l].is_valid = 0;
                maze[f][w][l].is_starting_area = 0;
                maze[f][w][l].has_wall = 0;
                maze[f][w][l].bawana_cell_type = -1;
            }
        }
    }

    // Floor 0: Initialize all valid blocks (856 sq ft = 214 blocks)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0; // Starting area is not part of playable maze
            } else {
                maze[0][w][l].is_valid = 1;
            }
        }
    }

    // Floor 1: Two rectangular areas (784 sq ft = 196 blocks)
    // First rectangle: 10x8 (80 blocks)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Bridge: 4x9 (36 blocks) - centered
    for (int w = 3; w <= 6; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    // Second rectangle: 10x8 (80 blocks)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }

    // Floor 2: Single rectangle 10x9 (360 sq ft = 90 blocks) - above starting area
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {
            maze[2][w][l].is_valid = 1;
        }
    }

    // Initialize Bawana area on Floor 0
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            maze[0][w][l].is_valid = 1;
        }
    }

    // Set Bawana walls
    // Wall from [6,20] to [9,20]
    for (int w = 6; w <= 9; w++) {
        maze[0][w][20].has_wall = 1;
    }
    // Wall from [6,20] to [6,24]
    for (int l = 20; l <= 24; l++) {
        maze[0][6][l].has_wall = 1;
    }

    // Bawana entrance at [9,19]
    maze[0][9][19].is_valid = 1;

    // Assign Bawana effects to interior cells (excluding walls and entrance)
    int bawana_interior_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };

    // Initialize all interior cells
    for (int i = 0; i < 12; i++) {
        int w = bawana_interior_cells[i][0];
        int l = bawana_interior_cells[i][1];
        maze[0][w][l].bawana_cell_type = -1;
    }

    // Assign 2 of each effect type (Food Poisoning, Disoriented, Triggered, Happy)
    int effect_types[] = {BA_FOOD_POISONING, BA_DISORIENTED, BA_TRIGGERED, BA_HAPPY};
    int assigned[12] = {0}; // Track which cells are assigned
    
    // Assign 2 of each effect type
    for (int effect = 0; effect < 4; effect++) {
        for (int count = 0; count < 2; count++) {
            int idx;
            do {
                idx = rand() % 12;
            } while (assigned[idx]);
            
            int w = bawana_interior_cells[idx][0];
            int l = bawana_interior_cells[idx][1];
            maze[0][w][l].bawana_cell_type = effect_types[effect];
            assigned[idx] = 1;
        }
    }
    
    // Assign random MP to remaining 4 cells
    for (int i = 0; i < 12; i++) {
        if (!assigned[i]) {
            int w = bawana_interior_cells[i][0];
            int l = bawana_interior_cells[i][1];
            maze[0][w][l].bawana_cell_type = BA_RANDOM_MP;
        }
    }
}

void initialize_players(Player players[3]) {
    // Player A - starts at [0,6,12], enters at [0,5,12], faces North
    players[PLAYER_A].pos[0] = 0; players[PLAYER_A].pos[1] = 6; players[PLAYER_A].pos[2] = 12;
    players[PLAYER_A].in_game = 0;
    players[PLAYER_A].direction = DIR_NORTH;
    players[PLAYER_A].movement_points = 100;
    players[PLAYER_A].roll_count = 0;
    players[PLAYER_A].captured = 0;
    players[PLAYER_A].bawana_effect = 0;
    players[PLAYER_A].bawana_turns_left = 0;
    players[PLAYER_A].bawana_random_mp = 0;

    // Player B - starts at [0,9,8], enters at [0,9,7], faces West
    players[PLAYER_B].pos[0] = 0; players[PLAYER_B].pos[1] = 9; players[PLAYER_B].pos[2] = 8;
    players[PLAYER_B].in_game = 0;
    players[PLAYER_B].direction = DIR_WEST;
    players[PLAYER_B].movement_points = 100;
    players[PLAYER_B].roll_count = 0;
    players[PLAYER_B].captured = 0;
    players[PLAYER_B].bawana_effect = 0;
    players[PLAYER_B].bawana_turns_left = 0;
    players[PLAYER_B].bawana_random_mp = 0;

    // Player C - starts at [0,9,16], enters at [0,9,17], faces East
    players[PLAYER_C].pos[0] = 0; players[PLAYER_C].pos[1] = 9; players[PLAYER_C].pos[2] = 16;
    players[PLAYER_C].in_game = 0;
    players[PLAYER_C].direction = DIR_EAST;
    players[PLAYER_C].movement_points = 100;
    players[PLAYER_C].roll_count = 0;
    players[PLAYER_C].captured = 0;
    players[PLAYER_C].bawana_effect = 0;
    players[PLAYER_C].bawana_turns_left = 0;
    players[PLAYER_C].bawana_random_mp = 0;
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 2;
    
    // Stair 1: From Floor 0 to Floor 1
    stairs[0].start_floor = 0; stairs[0].start_w = 5; stairs[0].start_l = 10;
    stairs[0].end_floor = 1; stairs[0].end_w = 5; stairs[0].end_l = 10;
    stairs[0].direction_type = STAIR_BIDIRECTIONAL;
    
    // Stair 2: From Floor 1 to Floor 2
    stairs[1].start_floor = 1; stairs[1].start_w = 4; stairs[1].start_l = 12;
    stairs[1].end_floor = 2; stairs[1].end_w = 4; stairs[1].end_l = 12;
    stairs[1].direction_type = STAIR_BIDIRECTIONAL;
}

void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 1;
    
    // Pole from Floor 2 to Floor 0
    poles[0].start_floor = 2;
    poles[0].end_floor = 0;
    poles[0].w = 5;
    poles[0].l = 24;
}

void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 3;
    
    // Bawana wall 1: horizontal from [6,20] to [9,20]
    walls[0].floor = 0;
    walls[0].start_w = 6; walls[0].start_l = 20;
    walls[0].end_w = 9; walls[0].end_l = 20;
    
    // Bawana wall 2: vertical from [6,20] to [6,24]
    walls[1].floor = 0;
    walls[1].start_w = 6; walls[1].start_l = 20;
    walls[1].end_w = 6; walls[1].end_l = 24;
    
    // Additional wall on Floor 1
    walls[2].floor = 1;
    walls[2].start_w = 0; walls[2].start_l = 2;
    walls[2].end_w = 8; walls[2].end_l = 2;
}

int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

int roll_direction_dice(void) {
    return (rand() % 6) + 1;
}

void enter_maze(Player *player, int player_id) {
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
}

int is_valid_position(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w, int l) {
    if (floor < 0 || floor >= NUM_FLOORS || 
        w < 0 || w >= FLOOR_WIDTH || 
        l < 0 || l >= FLOOR_LENGTH) {
        return 0;
    }
    return maze[floor][w][l].is_valid;
}

int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2) {
    Wall walls[MAX_WALLS];
    int num_walls;
    initialize_walls(walls, &num_walls);

    for (int i = 0; i < num_walls; i++) {
        if (walls[i].floor != floor) continue;
        
        // Check if movement crosses a wall
        if (walls[i].start_w == walls[i].end_w) { // Vertical wall
            int wall_w = walls[i].start_w;
            int wall_l_min = (walls[i].start_l < walls[i].end_l) ? walls[i].start_l : walls[i].end_l;
            int wall_l_max = (walls[i].start_l > walls[i].end_l) ? walls[i].start_l : walls[i].end_l;
            
            // Check if movement crosses this vertical wall
            if (((w1 == wall_w && w2 == wall_w + 1) || (w1 == wall_w + 1 && w2 == wall_w)) &&
                l1 == l2 && l1 >= wall_l_min && l1 <= wall_l_max) {
                return 1;
            }
        }
        
        if (walls[i].start_l == walls[i].end_l) { // Horizontal wall
            int wall_l = walls[i].start_l;
            int wall_w_min = (walls[i].start_w < walls[i].end_w) ? walls[i].start_w : walls[i].end_w;
            int wall_w_max = (walls[i].start_w > walls[i].end_w) ? walls[i].start_w : walls[i].end_w;
            
            // Check if movement crosses this horizontal wall
            if (((l1 == wall_l && l2 == wall_l + 1) || (l1 == wall_l + 1 && l2 == wall_l)) &&
                w1 == w2 && w1 >= wall_w_min && w1 <= wall_w_max) {
                return 1;
            }
        }
    }
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
        if ((poles[i].start_floor == floor || poles[i].start_floor > floor) &&
            poles[i].end_floor <= floor && poles[i].w == w && poles[i].l == l) {
            return i;
        }
    }
    return -1;
}

void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id) {
    int f = player->pos[0];
    int w = player->pos[1];
    int l = player->pos[2];
    
    if (f != 0 || w < 6 || w > 9 || l < 20 || l > 24) return;
    if (player->bawana_effect != 0) return; // Already has an effect
    
    int cell_type = maze[f][w][l].bawana_cell_type;
    char player_name = 'A' + player_id;
    
    switch (cell_type) {
        case BA_FOOD_POISONING:
            player->bawana_effect = 1;
            player->bawana_turns_left = 3;
            printf("%c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", player_name);
            break;
            
        case BA_DISORIENTED:
            player->bawana_effect = 2;
            player->bawana_turns_left = 4;
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19; // Move to entrance
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", player_name);
            break;
            
        case BA_TRIGGERED:
            player->bawana_effect = 3;
            player->bawana_turns_left = 4;
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19; // Move to entrance
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is triggered due to bad quality of food. %c is placed at the entrance of Bawana with 50 movement points.\n", player_name, player_name);
            break;
            
        case BA_HAPPY:
            player->bawana_effect = 4;
            player->bawana_turns_left = 4;
            player->movement_points += 200;
            player->pos[1] = 9; player->pos[2] = 19; // Move to entrance
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is happy. %c is placed at the entrance of Bawana with 200 movement points.\n", player_name, player_name);
            break;
            
        case BA_RANDOM_MP:
            player->bawana_effect = 5;
            player->bawana_turns_left = 4;
            player->bawana_random_mp = (rand() % 91) + 10; // 10-100
            player->movement_points += player->bawana_random_mp;
            player->pos[1] = 9; player->pos[2] = 19; // Move to entrance
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and earns %d movement points and is placed at the entrance.\n", player_name, player->bawana_random_mp);
            break;
    }
}

void move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps) {
    for (int s = 0; s < steps; s++) {
        int old_f = player->pos[0];
        int old_w = player->pos[1];
        int old_l = player->pos[2];
        
        int new_w = old_w;
        int new_l = old_l;
        
        // Calculate next position based on direction
        switch (player->direction) {
            case DIR_NORTH: new_l--; break;
            case DIR_EAST:  new_w++; break;
            case DIR_SOUTH: new_l++; break;
            case DIR_WEST:  new_w--; break;
        }
        
        // Check bounds and validity
        if (!is_valid_position(maze, old_f, new_w, new_l)) {
            return; // Can't move further
        }
        
        // Check for wall blocking
        if (is_wall_blocking(maze, old_f, old_w, old_l, new_w, new_l)) {
            return; // Wall blocks movement
        }
        
        // Update position
        player->pos[1] = new_w;
        player->pos[2] = new_l;
        
        // Check for stair
        int stair_index = find_stair_at(stairs, num_stairs, old_f, new_w, new_l);
        if (stair_index != -1) {
            Stair stair = stairs[stair_index];
            printf("Player lands on [%d,%d,%d] which is a stair cell.\n", old_f, new_w, new_l);
            
            if (stair.direction_type == STAIR_BIDIRECTIONAL ||
                (stair.direction_type == STAIR_UP_ONLY && old_f == stair.start_floor) ||
                (stair.direction_type == STAIR_DOWN_ONLY && old_f == stair.end_floor)) {
                
                if (old_f == stair.start_floor) {
                    player->pos[0] = stair.end_floor;
                    player->pos[1] = stair.end_w;
                    player->pos[2] = stair.end_l;
                } else {
                    player->pos[0] = stair.start_floor;
                    player->pos[1] = stair.start_w;
                    player->pos[2] = stair.start_l;
                }
                printf("Player takes the stairs and now placed at [%d,%d,%d] in floor %d.\n",
                       player->pos[0], player->pos[1], player->pos[2], player->pos[0]);
            }
            continue;
        }
        
        // Check for pole
        int pole_index = find_pole_at(poles, num_poles, old_f, new_w, new_l);
        if (pole_index != -1) {
            Pole pole = poles[pole_index];
            printf("Player lands on [%d,%d,%d] which is a pole cell.\n", old_f, new_w, new_l);
            player->pos[0] = pole.end_floor;
            player->pos[1] = pole.w;
            player->pos[2] = pole.l;
            printf("Player slides down and now placed at [%d,%d,%d] in floor %d.\n",
                   player->pos[0], player->pos[1], player->pos[2], player->pos[0]);
            continue;
        }
        
        // Check for Bawana effect
        if (old_f == 0 && new_w >= 6 && new_w <= 9 && new_l >= 20 && new_l <= 24) {
            apply_bawana_effect(player, maze, 0); // Assuming we pass player index somehow
        }
    }
}

void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    int valid_positions[1000][3];
    int count = 0;
    
    // Collect all valid positions
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid && !maze[f][w][l].is_starting_area && !maze[f][w][l].has_wall) {
                    valid_positions[count][0] = f;
                    valid_positions[count][1] = w;
                    valid_positions[count][2] = l;
                    count++;
                }
            }
        }
    }
    
    if (count == 0) {
        printf("Error: No valid positions for flag!\n");
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
    for (int i = 0; i < 3; i++) {
        if (i == current_player || !players[i].in_game) continue;
        
        if (players[current_player].pos[0] == players[i].pos[0] &&
            players[current_player].pos[1] == players[i].pos[1] &&
            players[current_player].pos[2] == players[i].pos[2]) {
            
            printf("Player %c captures Player %c!\n", 'A' + current_player, 'A' + i);
            players[i].in_game = 0;
            players[i].captured = 1;
            
            // Reset to starting position
            switch (i) {
                case PLAYER_A:
                    players[i].pos[0] = 0; players[i].pos[1] = 6; players[i].pos[2] = 12;
                    break;
                case PLAYER_B:
                    players[i].pos[0] = 0; players[i].pos[1] = 9; players[i].pos[2] = 8;
                    break;
                case PLAYER_C:
                    players[i].pos[0] = 0; players[i].pos[1] = 9; players[i].pos[2] = 16;
                    break;
            }
            printf("Player %c sent back to starting area\n", 'A' + i);
            return;
        }
    }
}

void update_stair_directions(Stair stairs[], int num_stairs) {
    static int round_count = 0;
    round_count++;
    
    if (round_count % 5 == 0) {
        for (int i = 0; i < num_stairs; i++) {
            stairs[i].direction_type = rand() % 3;
        }
        printf("Stair directions updated after 5 rounds.\n");
    }
}

void reset_to_bawana(Player *player) {
    printf("Player movement points are depleted and requires replenishment. Transporting to Bawana.\n");
    player->pos[0] = 0;
    player->pos[1] = 9;
    player->pos[2] = 19;
    player->direction = DIR_NORTH;
    player->in_game = 1;
    player->bawana_effect = 0;
    player->bawana_turns_left = 0;
    player->bawana_random_mp = 0;
}