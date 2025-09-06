// game.c
#include "game.h"
#include <stdlib.h>

const char* get_direction_name(int direction) {
    switch(direction) {
        case DIR_NORTH: return "North";
        case DIR_EAST: return "East";
        case DIR_SOUTH: return "South";
        case DIR_WEST: return "West";
        default: return "Unknown";
    }
}

const char* format_position(int floor, int w, int l) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "[%d,%d,%d]", floor, w, l);
    return buffer;
}

const char* get_blockage_reason_description(int blocking_reason) {
    switch(blocking_reason) {
        case BLOCK_WALL: return "wall";
        case BLOCK_INVALID_CELL: return "invalid cell";
        case BLOCK_BAWANA_ENTRANCE: return "Bawana entrance restriction";
        default: return "unknown obstacle";
    }
}

void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                maze[f][w][l].is_valid = 0;
                maze[f][w][l].is_starting_area = 0;
                maze[f][w][l].has_wall = 0;
                maze[f][w][l].bawana_cell_type = -1;
                maze[f][w][l].is_blocked_by_stair = 0;
                maze[f][w][l].is_bawana_entrance = 0;
            }
        }
    }

    // Floor 0
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0;
            } else {
                maze[0][w][l].is_valid = 1;
            }
        }
    }

    // Floor 1
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    for (int w = 3; w <= 6; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }

    // Floor 2
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {
            maze[2][w][l].is_valid = 1;
        }
    }

    // Bawana area
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            maze[0][w][l].is_valid = 1;
        }
    }

    // Mark Bawana entrance cell
    maze[0][9][19].is_valid = 1;
    maze[0][9][19].is_bawana_entrance = 1;

    // Bawana walls
    for (int w = 6; w <= 9; w++) {
        maze[0][w][20].has_wall = 1;
    }
    for (int l = 20; l <= 24; l++) {
        maze[0][6][l].has_wall = 1;
    }

    // Assign Bawana effects
    int bawana_interior_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };

    int effect_types[] = {BA_FOOD_POISONING, BA_DISORIENTED, BA_TRIGGERED, BA_HAPPY};
    int assigned[12] = {0};

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

    for (int i = 0; i < 12; i++) {
        if (!assigned[i]) {
            int w = bawana_interior_cells[i][0];
            int l = bawana_interior_cells[i][1];
            maze[0][w][l].bawana_cell_type = BA_RANDOM_MP;
        }
    }
}

void initialize_players(Player players[3]) {
    players[PLAYER_A].pos[0] = 0; players[PLAYER_A].pos[1] = 6; players[PLAYER_A].pos[2] = 12;
    players[PLAYER_A].in_game = 0;
    players[PLAYER_A].direction = DIR_NORTH;
    players[PLAYER_A].movement_points = 100;
    players[PLAYER_A].roll_count = 0;
    players[PLAYER_A].captured = 0;
    players[PLAYER_A].bawana_effect = EFFECT_NONE;
    players[PLAYER_A].bawana_turns_left = 0;
    players[PLAYER_A].bawana_random_mp = 0;
    players[PLAYER_A].just_entered = 0;

    players[PLAYER_B].pos[0] = 0; players[PLAYER_B].pos[1] = 9; players[PLAYER_B].pos[2] = 8;
    players[PLAYER_B].in_game = 0;
    players[PLAYER_B].direction = DIR_WEST;
    players[PLAYER_B].movement_points = 100;
    players[PLAYER_B].roll_count = 0;
    players[PLAYER_B].captured = 0;
    players[PLAYER_B].bawana_effect = EFFECT_NONE;
    players[PLAYER_B].bawana_turns_left = 0;
    players[PLAYER_B].bawana_random_mp = 0;
    players[PLAYER_B].just_entered = 0;

    players[PLAYER_C].pos[0] = 0; players[PLAYER_C].pos[1] = 9; players[PLAYER_C].pos[2] = 16;
    players[PLAYER_C].in_game = 0;
    players[PLAYER_C].direction = DIR_EAST;
    players[PLAYER_C].movement_points = 100;
    players[PLAYER_C].roll_count = 0;
    players[PLAYER_C].captured = 0;
    players[PLAYER_C].bawana_effect = EFFECT_NONE;
    players[PLAYER_C].bawana_turns_left = 0;
    players[PLAYER_C].bawana_random_mp = 0;
    players[PLAYER_C].just_entered = 0;
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 2;
    stairs[0].start_floor = 0; stairs[0].start_w = 5; stairs[0].start_l = 10;
    stairs[0].end_floor = 1; stairs[0].end_w = 5; stairs[0].end_l = 10;
    stairs[0].direction_type = STAIR_BIDIRECTIONAL;

    stairs[1].start_floor = 1; stairs[1].start_w = 4; stairs[1].start_l = 12;
    stairs[1].end_floor = 2; stairs[1].end_w = 4; stairs[1].end_l = 12;
    stairs[1].direction_type = STAIR_BIDIRECTIONAL;
}

void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 1;
    poles[0].start_floor = 2;
    poles[0].end_floor = 0;
    poles[0].w = 5;
    poles[0].l = 24;
}

void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 3;
    walls[0].floor = 0; walls[0].start_w = 6; walls[0].start_l = 20; walls[0].end_w = 9; walls[0].end_l = 20;
    walls[1].floor = 0; walls[1].start_w = 6; walls[1].start_l = 20; walls[1].end_w = 6; walls[1].end_l = 24;
    walls[2].floor = 1; walls[2].start_w = 0; walls[2].start_l = 2; walls[2].end_w = 8; walls[2].end_l = 2;
}

int read_seed_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Could not open %s. Using default seed.\n", filename);
        return time(NULL);
    }
    int seed;
    if (fscanf(file, "%d", &seed) != 1) {
        printf("Warning: Could not read seed from %s. Using default seed.\n", filename);
        fclose(file);
        return time(NULL);
    }
    fclose(file);
    return seed;
}

int read_stairs_from_file(const char *filename, Stair stairs[], int *num_stairs) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    *num_stairs = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && *num_stairs < MAX_STAIRS) {
        if (sscanf(line, "[%d, %d, %d, %d, %d, %d]",
                   &stairs[*num_stairs].start_floor,
                   &stairs[*num_stairs].start_w,
                   &stairs[*num_stairs].start_l,
                   &stairs[*num_stairs].end_floor,
                   &stairs[*num_stairs].end_w,
                   &stairs[*num_stairs].end_l) == 6) {
            stairs[*num_stairs].direction_type = STAIR_BIDIRECTIONAL;
            (*num_stairs)++;
        }
    }
    fclose(file);
    printf("Loaded %d stairs from %s\n", *num_stairs, filename);
    return 1;
}

int read_poles_from_file(const char *filename, Pole poles[], int *num_poles) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    *num_poles = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && *num_poles < MAX_POLES) {
        if (sscanf(line, "[%d, %d, %d, %d]",
                   &poles[*num_poles].start_floor,
                   &poles[*num_poles].end_floor,
                   &poles[*num_poles].w,
                   &poles[*num_poles].l) == 4) {
            (*num_poles)++;
        }
    }
    fclose(file);
    printf("Loaded %d poles from %s\n", *num_poles, filename);
    return 1;
}

int read_walls_from_file(const char *filename, Wall walls[], int *num_walls) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    *num_walls = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && *num_walls < MAX_WALLS) {
        if (sscanf(line, "[%d, %d, %d, %d, %d]",
                   &walls[*num_walls].floor,
                   &walls[*num_walls].start_w,
                   &walls[*num_walls].start_l,
                   &walls[*num_walls].end_w,
                   &walls[*num_walls].end_l) == 5) {
            (*num_walls)++;
        }
    }
    fclose(file);
    printf("Loaded %d walls from %s\n", *num_walls, filename);
    return 1;
}

int read_flag_from_file(const char *filename, int flag[3]) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "[%d, %d, %d]", &flag[0], &flag[1], &flag[2]) == 3) {
            fclose(file);
            printf("Loaded flag position [%d,%d,%d] from %s\n", flag[0], flag[1], flag[2], filename);
            return 1;
        }
    }
    fclose(file);
    return 0;
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
    player->just_entered = 1;
}

void enter_maze_like_player_a(Player *player) {
    // When captured or reset, all players enter like Player A
    player->pos[0] = 0; 
    player->pos[1] = 5; 
    player->pos[2] = 12;
    player->direction = DIR_NORTH; // Same direction as Player A
    player->in_game = 1;
    player->just_entered = 1;
}

int is_valid_position(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w, int l) {
    if (floor < 0 || floor >= NUM_FLOORS || w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) {
        return 0;
    }
    if (maze[floor][w][l].is_blocked_by_stair) return 0;
    return maze[floor][w][l].is_valid;
}

int can_enter_bawana_entrance(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int new_w, int new_l) {
    if (maze[0][new_w][new_l].is_bawana_entrance) {
        if (player->movement_points > 0) {
            return 0;
        }
    }
    return 1;
}

int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2, Wall walls[], int num_walls) {
    for (int i = 0; i < num_walls; i++) {
        if (walls[i].floor != floor) continue;
        if (walls[i].start_w == walls[i].end_w) {
            int wall_w = walls[i].start_w;
            int wall_l_min = (walls[i].start_l < walls[i].end_l) ? walls[i].start_l : walls[i].end_l;
            int wall_l_max = (walls[i].start_l > walls[i].end_l) ? walls[i].start_l : walls[i].end_l;
            if (((w1 == wall_w && w2 == wall_w + 1) || (w1 == wall_w + 1 && w2 == wall_w)) &&
                l1 == l2 && l1 >= wall_l_min && l1 <= wall_l_max) {
                return 1;
            }
        }
        if (walls[i].start_l == walls[i].end_l) {
            int wall_l = walls[i].start_l;
            int wall_w_min = (walls[i].start_w < walls[i].end_w) ? walls[i].start_w : walls[i].end_w;
            int wall_w_max = (walls[i].start_w > walls[i].end_w) ? walls[i].start_w : walls[i].end_w;
            if (((l1 == wall_l && l2 == wall_l + 1) || (l1 == wall_l + 1 && l2 == wall_l)) &&
                w1 == w2 && w1 >= wall_w_min && w1 <= wall_w_max) {
                return 1;
            }
        }
    }
    return 0;
}

int find_all_stairs_at(Stair stairs[], int num_stairs, int floor, int w, int l, int result_indices[]) {
    int count = 0;
    for (int i = 0; i < num_stairs; i++) {
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l)) {
            result_indices[count++] = i;
        }
    }
    return count;
}

int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].w == w && poles[i].l == l) {
            if ((poles[i].start_floor >= floor && poles[i].end_floor <= floor) ||
                (poles[i].start_floor <= floor && poles[i].end_floor >= floor)) {
                return i;
            }
        }
    }
    return -1;
}

int manhattan_distance(int f1, int w1, int l1, int f2, int w2, int l2) {
    return abs(f1 - f2) + abs(w1 - w2) + abs(l1 - l2);
}

// NEW: Check path validity and return detailed blocking reason
int check_path_validity(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                        Wall walls[], int num_walls, int steps, int player_id, int *blocking_step, int *blocking_reason) {
    int current_w = player->pos[1];
    int current_l = player->pos[2];
    int current_floor = player->pos[0];
    
    *blocking_step = -1;
    *blocking_reason = BLOCK_NONE;
    
    for (int s = 0; s < steps; s++) {
        int new_w = current_w;
        int new_l = current_l;
        
        switch (player->direction) {
            case DIR_NORTH: new_l--; break;
            case DIR_EAST:  new_w++; break;
            case DIR_SOUTH: new_l++; break;
            case DIR_WEST:  new_w--; break;
        }
        
        // Check if wall blocks movement first (highest priority)
        if (is_wall_blocking(maze, current_floor, current_w, current_l, new_w, new_l, walls, num_walls)) {
            *blocking_step = s;
            *blocking_reason = BLOCK_WALL;
            return 0;
        }
        
        // Check if next position is valid
        if (!is_valid_position(maze, current_floor, new_w, new_l)) {
            *blocking_step = s;
            *blocking_reason = BLOCK_INVALID_CELL;
            return 0;
        }
        
        // Check Bawana entrance restriction
        if (!can_enter_bawana_entrance(player, maze, new_w, new_l)) {
            *blocking_step = s;
            *blocking_reason = BLOCK_BAWANA_ENTRANCE;
            return 0;
        }
        
        // Update position for next iteration
        current_w = new_w;
        current_l = new_l;
    }
    
    return 1; // Path is valid
}

void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id) {
    int f = player->pos[0];
    int w = player->pos[1];
    int l = player->pos[2];
    
    if (f != 0 || w < 6 || w > 9 || l < 20 || l > 24) return;
    if (player->bawana_effect != EFFECT_NONE) return;

    int cell_type = maze[f][w][l].bawana_cell_type;
    char player_name = 'A' + player_id;
    
    // Second required message: Player is placed on a cell type and effects take place
    const char* cell_type_names[] = {"food poisoning", "disoriented", "triggered", "happy", "random MP"};
    if (cell_type >= 0 && cell_type < 5) {
        printf("%c is placed on a %s cell and effects take place.\n", player_name, cell_type_names[cell_type]);
    } else {
        printf("%c is placed on a random cell and effects take place.\n", player_name);
    }

    switch (cell_type) {
        case BA_FOOD_POISONING:
            player->bawana_effect = EFFECT_FOOD_POISONING;
            player->bawana_turns_left = 3;
            printf("%c eats from Bawana and has a bad case of food poisoning. Will need three rounds to recover.\n", player_name);
            break;
        case BA_DISORIENTED:
            player->bawana_effect = EFFECT_DISORIENTED;
            player->bawana_turns_left = 4;
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", player_name);
            break;
        case BA_TRIGGERED:
            player->bawana_effect = EFFECT_TRIGGERED;
            player->bawana_turns_left = 4; // Fixed to 4 turns
            player->movement_points += 50;
            player->pos[1] = 9; player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is triggered due to bad quality of food. %c is placed at the entrance of Bawana with 50 movement points.\n", player_name, player_name);
            break;
        case BA_HAPPY:
            player->bawana_effect = EFFECT_HAPPY;
            player->bawana_turns_left = 1; // Set to 1 to indicate active but no countdown
            player->movement_points += 200;
            player->pos[1] = 9; player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is happy. %c is placed at the entrance of Bawana with 200 movement points.\n", player_name, player_name);
            break;
        case BA_RANDOM_MP:
            player->bawana_effect = EFFECT_RANDOM_MP;
            player->bawana_turns_left = 4;
            player->bawana_random_mp = (rand() % 91) + 10;
            player->movement_points += player->bawana_random_mp;
            player->pos[1] = 9; player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and earns %d movement points and is placed at the %s.\n", 
                   player_name, player->bawana_random_mp, format_position(player->pos[0], player->pos[1], player->pos[2]));
            break;
    }
}

int is_in_starting_area(int floor, int w, int l) {
    return (floor == 0 &&
            w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
            l >= START_AREA_L_MIN && l <= START_AREA_L_MAX);
}

void reset_to_starting_area(Player *player, int player_id) {
    char player_name = 'A' + player_id;
    printf("Player %c trapped in Infinite Loop - resetting to Player A's starting area. Movement points preserved.\n", player_name);
    // All players go to Player A's starting position
    player->pos[0] = 0;
    player->pos[1] = 6;
    player->pos[2] = 12;
    player->direction = DIR_NORTH; // Same direction as Player A
    player->in_game = 0;
    player->captured = 0;
}

int move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps, int player_id, const int flag[3],
                               int *movement_cost, int *actual_steps, int *blocking_reason) {
    
    int blocking_step;
    int local_blocking_reason;
    
    // Initialize output parameters
    if (movement_cost) *movement_cost = 0;
    if (actual_steps) *actual_steps = 0;
    if (blocking_reason) *blocking_reason = BLOCK_NONE;
    
    // First, check if entire path is valid
    int path_valid = check_path_validity(player, maze, walls, num_walls, steps, player_id, &blocking_step, &local_blocking_reason);
    
    if (blocking_reason) {
        *blocking_reason = local_blocking_reason;
    }
    
    if (!path_valid) {
        if (movement_cost) *movement_cost = 2; // Cost for being blocked
        return 0; // Path blocked, no movement
    }
    
    // Path is valid, proceed with movement
    int visited[MAX_LOOP_HISTORY][3];
    int visit_count = 0;
    char player_name = 'A' + player_id;

    for (int s = 0; s < steps; s++) {
        int old_f = player->pos[0];
        int old_w = player->pos[1];
        int old_l = player->pos[2];

        // Record current position before any movement
        visited[visit_count][0] = old_f;
        visited[visit_count][1] = old_w;
        visited[visit_count][2] = old_l;
        visit_count++;

        if (visit_count >= MAX_LOOP_HISTORY) {
            visit_count = 0;
        }

        int new_w = old_w;
        int new_l = old_l;

        switch (player->direction) {
            case DIR_NORTH: new_l--; break;
            case DIR_EAST:  new_w++; break;
            case DIR_SOUTH: new_l++; break;
            case DIR_WEST:  new_w--; break;
        }

        player->pos[1] = new_w;
        player->pos[2] = new_l;

        // Check for infinite loop after basic movement
        for (int i = 0; i < visit_count - 1; i++) {
            if (visited[i][0] == player->pos[0] &&
                visited[i][1] == player->pos[1] &&
                visited[i][2] == player->pos[2]) {
                printf("Infinite loop detected at [%d,%d,%d]!\n", player->pos[0], player->pos[1], player->pos[2]);
                reset_to_starting_area(player, player_id);
                return 1; // Successful completion (loop detected and handled)
            }
        }

        // Handle stairs
        int stair_indices[MAX_STAIRS];
        int num_found = find_all_stairs_at(stairs, num_stairs, old_f, new_w, new_l, stair_indices);

        if (num_found > 0) {
            printf("%c lands on %s which is a stair cell.\n", player_name, format_position(old_f, new_w, new_l));

            int chosen_stair = -1;

            if (num_found == 1) {
                chosen_stair = stair_indices[0];
            } else {
                int best_distance = 999999;
                int best_index = -1;
                int candidates[MAX_STAIRS];
                int candidate_count = 0;

                for (int i = 0; i < num_found; i++) {
                    int idx = stair_indices[i];
                    Stair *st = &stairs[idx];

                    int dest_f, dest_w, dest_l;
                    if (old_f == st->start_floor) {
                        dest_f = st->end_floor; dest_w = st->end_w; dest_l = st->end_l;
                    } else {
                        dest_f = st->start_floor; dest_w = st->start_w; dest_l = st->start_l;
                    }

                    int dist = manhattan_distance(dest_f, dest_w, dest_l, flag[0], flag[1], flag[2]);
                    if (dist < best_distance) {
                        best_distance = dist;
                        best_index = idx;
                        candidate_count = 0;
                        candidates[candidate_count++] = idx;
                    } else if (dist == best_distance) {
                        candidates[candidate_count++] = idx;
                    }
                }

                if (candidate_count > 1) {
                    chosen_stair = candidates[rand() % candidate_count];
                    printf("Multiple stairs at same distance - randomly chose one.\n");
                } else {
                    chosen_stair = best_index;
                }
            }

            Stair *st = &stairs[chosen_stair];
            if (st->direction_type == STAIR_BIDIRECTIONAL ||
                (st->direction_type == STAIR_UP_ONLY && old_f == st->start_floor) ||
                (st->direction_type == STAIR_DOWN_ONLY && old_f == st->end_floor)) {

                int dest_f, dest_w, dest_l;
                if (old_f == st->start_floor) {
                    dest_f = st->end_floor; dest_w = st->end_w; dest_l = st->end_l;
                } else {
                    dest_f = st->start_floor; dest_w = st->start_w; dest_l = st->start_l;
                }

                player->pos[0] = dest_f;
                player->pos[1] = dest_w;
                player->pos[2] = dest_l;

                printf("%c takes the stairs and now placed at %s in floor %d.\n",
                       player_name, format_position(player->pos[0], player->pos[1], player->pos[2]), player->pos[0]);

                // Check for infinite loop after stair teleportation
                for (int i = 0; i < visit_count; i++) {
                    if (visited[i][0] == player->pos[0] &&
                        visited[i][1] == player->pos[1] &&
                        visited[i][2] == player->pos[2]) {
                        printf("Infinite loop detected after stair teleportation at [%d,%d,%d]!\n", 
                               player->pos[0], player->pos[1], player->pos[2]);
                        reset_to_starting_area(player, player_id);
                        return 1; // Successful completion (loop detected and handled)
                    }
                }

                if (is_in_starting_area(player->pos[0], player->pos[1], player->pos[2])) {
                    printf("%c fell into starting area via stair - must roll 6 to re-enter.\n", player_name);
                    player->in_game = 0;
                }

                continue;
            }
        }

        // Handle poles
        int pole_index = find_pole_at(poles, num_poles, old_f, new_w, new_l);
        if (pole_index != -1) {
            Pole *pole = &poles[pole_index];
            printf("%c lands on %s which is a pole cell.\n", player_name, format_position(old_f, new_w, new_l));

            player->pos[0] = pole->end_floor;
            player->pos[1] = pole->w;
            player->pos[2] = pole->l;

            printf("%c slides down and now placed at %s in floor %d.\n",
                   player_name, format_position(player->pos[0], player->pos[1], player->pos[2]), player->pos[0]);

            // Check for infinite loop after pole teleportation
            for (int i = 0; i < visit_count; i++) {
                if (visited[i][0] == player->pos[0] &&
                    visited[i][1] == player->pos[1] &&
                    visited[i][2] == player->pos[2]) {
                    printf("Infinite loop detected after pole teleportation at [%d,%d,%d]!\n", 
                           player->pos[0], player->pos[1], player->pos[2]);
                    reset_to_starting_area(player, player_id);
                    return 1; // Successful completion (loop detected and handled)
                }
            }

            if (is_in_starting_area(player->pos[0], player->pos[1], player->pos[2])) {
                printf("%c fell into starting area via pole - must roll 6 to re-enter.\n", player_name);
                player->in_game = 0;
            }

            continue;
        }

        // Apply Bawana effects if in Bawana area
        if (old_f == 0 && new_w >= 6 && new_w <= 9 && new_l >= 20 && new_l <= 24) {
            apply_bawana_effect(player, maze, player_id);
        }
    }
    
    // Set output parameters for successful movement
    if (movement_cost) *movement_cost = steps; // 1 MP per step
    if (actual_steps) *actual_steps = steps;
    
    return 1; // Movement successful
}

void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    int valid_positions[1000][3];
    int count = 0;
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
    if (count == 0) { exit(1); }
    int choice = rand() % count;
    flag[0] = valid_positions[choice][0];
    flag[1] = valid_positions[choice][1];
    flag[2] = valid_positions[choice][2];
}

int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] && player->pos[1] == flag[1] && player->pos[2] == flag[2]);
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
            // All captured players go to Player A's starting position
            players[i].pos[0] = 0; 
            players[i].pos[1] = 6; 
            players[i].pos[2] = 12;
            players[i].direction = DIR_NORTH; // Same direction as Player A
            printf("Player %c sent back to Player A's starting area - must roll 6 to re-enter like Player A\n", 'A' + i);
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

void reset_to_bawana(Player *player, int player_id) {
    char player_name = 'A' + player_id;
    printf("%c movement points are depleted and requires replenishment. Transporting to Bawana.\n", player_name);
    
    // Place player on a random Bawana cell to get effects
    int bawana_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };
    int idx = rand() % 12;
    player->pos[0] = 0;
    player->pos[1] = bawana_cells[idx][0];
    player->pos[2] = bawana_cells[idx][1];
    player->direction = DIR_NORTH;
    player->in_game = 1;
    player->bawana_effect = EFFECT_NONE;
    player->bawana_turns_left = 0;
    player->bawana_random_mp = 0;
    
    // Note: The second message and effects will be handled by the calling function
    // after this function returns, when apply_bawana_effect is called
}