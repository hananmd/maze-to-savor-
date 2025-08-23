#include "game.h"

// Global variables for Rule 6
int stair_change_counter = 0;
int rounds_completed = 0;

// Wait function to press Enter, then roll dice automatically
static void wait_for_enter(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    fgets(buffer, sizeof buffer, stdin);
}

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
    
    // Floor 0 (Ground): 10x25
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            maze[0][w][l].is_valid = 1;
        }
    }
    
    // Starting area: 4 blocks width, 9 blocks length, starting at position 8 along length
    for (int w = 0; w < 4; w++) {
        for (int l = 8; l < 17; l++) { // 8 to 16 (9 blocks)
            maze[0][w][l].is_starting_area = 1;
        }
    }
    
    // Floor 1: Two rectangular areas connected by bridge
    // First rectangular area (10x8)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    
    // Bridge (4 blocks width, 9 blocks length) - starting from l=8
    for (int w = 3; w < 7; w++) { // 4 blocks width centered
        for (int l = 8; l < 17; l++) { // 9 blocks length
            maze[1][w][l].is_valid = 1;
        }
    }
    
    // Second rectangular area (10x8)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 17; l < FLOOR_LENGTH; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    
    // Floor 2: Single rectangular area (10x9)
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) { // 9 blocks length
            maze[2][w][l].is_valid = 1;
        }
    }
}

void initialize_players(Player players[3]) {
    // Player A
    players[0].pos[0] = 0; players[0].pos[1] = 6; players[0].pos[2] = 12;
    players[0].in_game = 0;
    players[0].direction = DIR_NORTH;
    players[0].movement_points = 100;
    players[0].roll_count = 0;
    players[0].entered_maze = 0;
    players[0].captured = 0;
    players[0].capture_start_pos[0] = 0; players[0].capture_start_pos[1] = 6; players[0].capture_start_pos[2] = 12;
    
    // Player B
    players[1].pos[0] = 0; players[1].pos[1] = 9; players[1].pos[2] = 8;
    players[1].in_game = 0;
    players[1].direction = DIR_WEST;
    players[1].movement_points = 100;
    players[1].roll_count = 0;
    players[1].entered_maze = 0;
    players[1].captured = 0;
    players[1].capture_start_pos[0] = 0; players[1].capture_start_pos[1] = 9; players[1].capture_start_pos[2] = 8;
    
    // Player C
    players[2].pos[0] = 0; players[2].pos[1] = 9; players[2].pos[2] = 16;
    players[2].in_game = 0;
    players[2].direction = DIR_EAST;
    players[2].movement_points = 100;
    players[2].roll_count = 0;
    players[2].entered_maze = 0;
    players[2].captured = 0;
    players[2].capture_start_pos[0] = 0; players[2].capture_start_pos[1] = 9; players[2].capture_start_pos[2] = 16;
}

// Rule 6: Read stairs from PDF/text file
int read_stairs_from_pdf(const char* filename, Stair stairs[], int* num_stairs) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Could not open stairs file %s. Using default stairs.\n", filename);
        return 0;
    }
    
    char line[256];
    *num_stairs = 0;
    
    while (fgets(line, sizeof(line), file) && *num_stairs < MAX_STAIRS) {
        if (strncmp(line, "STAIR:", 6) == 0) {
            int start_floor, start_w, start_l, end_floor, end_w, end_l;
            if (sscanf(line, "STAIR: [%d, %d, %d, %d, %d, %d]",
                      &start_floor, &start_w, &start_l, &end_floor, &end_w, &end_l) == 6) {
                stairs[*num_stairs].start_floor = start_floor;
                stairs[*num_stairs].start_w = start_w;
                stairs[*num_stairs].start_l = start_l;
                stairs[*num_stairs].end_floor = end_floor;
                stairs[*num_stairs].end_w = end_w;
                stairs[*num_stairs].end_l = end_l;
                stairs[*num_stairs].direction_type = rand() % 3; // Random direction type
                (*num_stairs)++;
            }
        }
    }
    
    fclose(file);
    printf("Loaded %d stairs from %s\n", *num_stairs, filename);
    return 1;
}

// Rule 6: Read poles from PDF/text file
int read_poles_from_pdf(const char* filename, Pole poles[], int* num_poles) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Could not open poles file %s. Using default poles.\n", filename);
        return 0;
    }
    
    char line[256];
    *num_poles = 0;
    
    while (fgets(line, sizeof(line), file) && *num_poles < MAX_POLES) {
        if (strncmp(line, "POLE:", 5) == 0) {
            int start_floor, end_floor, w, l;
            if (sscanf(line, "POLE: [%d, %d, %d, %d]", &start_floor, &end_floor, &w, &l) == 4) {
                poles[*num_poles].start_floor = start_floor;
                poles[*num_poles].end_floor = end_floor;
                poles[*num_poles].w = w;
                poles[*num_poles].l = l;
                (*num_poles)++;
            }
        }
    }
    
    fclose(file);
    printf("Loaded %d poles from %s\n", *num_poles, filename);
    return 1;
}

// Rule 6: Read walls from PDF/text file
int read_walls_from_pdf(const char* filename, Wall walls[], int* num_walls) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Could not open walls file %s. Using default walls.\n", filename);
        return 0;
    }
    
    char line[256];
    *num_walls = 0;
    
    while (fgets(line, sizeof(line), file) && *num_walls < MAX_WALLS) {
        if (strncmp(line, "WALL:", 5) == 0) {
            int floor, start_w, start_l, end_w, end_l;
            if (sscanf(line, "WALL: [%d, %d, %d, %d, %d]", &floor, &start_w, &start_l, &end_w, &end_l) == 5) {
                walls[*num_walls].floor = floor;
                walls[*num_walls].start_w = start_w;
                walls[*num_walls].start_l = start_l;
                walls[*num_walls].end_w = end_w;
                walls[*num_walls].end_l = end_l;
                (*num_walls)++;
            }
        }
    }
    
    fclose(file);
    printf("Loaded %d walls from %s\n", *num_walls, filename);
    return 1;
}

// Rule 6: Randomize stair directions
void randomize_stair_directions(Stair stairs[], int num_stairs) {
    for (int i = 0; i < num_stairs; i++) {
        stairs[i].direction_type = rand() % 3;
        const char* dir_names[] = {"UP_ONLY", "DOWN_ONLY", "BIDIRECTIONAL"};
        printf("Stair %d direction changed to: %s\n", i, dir_names[stairs[i].direction_type]);
    }
}

// Rule 6: Update stair directions every 5 rounds
void update_stair_directions(Stair stairs[], int num_stairs) {
    stair_change_counter++;
    if (stair_change_counter >= 5) {
        printf("\n=== STAIR DIRECTIONS CHANGING! ===\n");
        randomize_stair_directions(stairs, num_stairs);
        stair_change_counter = 0;
        printf("=================================\n\n");
    }
}

// Rule 6: Check if a round is completed (all 3 players have played)
int check_round_completion(void) {
    static int turn_counter = 0;
    turn_counter++;
    
    if (turn_counter >= 3) {
        turn_counter = 0;
        rounds_completed++;
        return 1; // Round completed
    }
    return 0; // Round not completed
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    // Try to read from file first, if fails use default
    if (!read_stairs_from_pdf("stairs.txt", stairs, num_stairs)) {
        // Default stairs if file reading fails
        *num_stairs = 2;
        
        stairs[0].start_floor = 0; stairs[0].start_w = 4; stairs[0].start_l = 5;
        stairs[0].end_floor = 2; stairs[0].end_w = 0; stairs[0].end_l = 10;
        stairs[0].direction_type = rand() % 3;
        
        stairs[1].start_floor = 1; stairs[1].start_w = 6; stairs[1].start_l = 3;
        stairs[1].end_floor = 2; stairs[1].end_w = 8; stairs[1].end_l = 12;
        stairs[1].direction_type = rand() % 3;
        
        printf("Using default stairs configuration.\n");
    }
    
    // Randomize initial directions
    randomize_stair_directions(stairs, *num_stairs);
}

void initialize_poles(Pole poles[], int *num_poles) {
    // Try to read from file first, if fails use default
    if (!read_poles_from_pdf("poles.txt", poles, num_poles)) {
        // Default poles if file reading fails
        *num_poles = 1;
        poles[0].start_floor = 2; poles[0].end_floor = 0; 
        poles[0].w = 5; poles[0].l = 24;
        printf("Using default poles configuration.\n");
    }
}

void initialize_walls(Wall walls[], int *num_walls) {
    // Try to read from file first, if fails use default
    if (!read_walls_from_pdf("walls.txt", walls, num_walls)) {
        // Default walls if file reading fails
        *num_walls = 1;
        walls[0].floor = 1; walls[0].start_w = 0; walls[0].start_l = 2;
        walls[0].end_w = 8; walls[0].end_l = 2;
        printf("Using default walls configuration.\n");
    }
}

void place_random_flag(int flag[3]) {
    flag[0] = rand() % NUM_FLOORS;
    flag[1] = rand() % FLOOR_WIDTH;
    flag[2] = rand() % FLOOR_LENGTH;
}

int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

int roll_direction_dice(void) {
    int roll = rand() % 6;
    switch (roll) {
        case 0: case 5: return DIR_EMPTY; // Empty faces
        case 1: return DIR_NORTH;
        case 2: return DIR_EAST;
        case 3: return DIR_SOUTH;
        case 4: return DIR_WEST;
        default: return DIR_EMPTY;
    }
}

void enter_maze(Player *player, int player_id) {
    if (player_id == PLAYER_A) {
        player->pos[0] = 0; player->pos[1] = 5; player->pos[2] = 12;
        player->direction = DIR_NORTH;
        printf("Player A enters the maze at [0,5,12]\n");
    } else if (player_id == PLAYER_B) {
        player->pos[0] = 0; player->pos[1] = 9; player->pos[2] = 7;
        player->direction = DIR_WEST;
        printf("Player B enters the maze at [0,9,7]\n");
    } else if (player_id == PLAYER_C) {
        player->pos[0] = 0; player->pos[1] = 9; player->pos[2] = 17;
        player->direction = DIR_EAST;
        printf("Player C enters the maze at [0,9,17]\n");
    }
    player->in_game = 1;
    player->entered_maze = 1;
}

// Rule 5: Check for player collision at new position
int check_player_collision(Player players[3], int current_player, int new_floor, int new_w, int new_l) {
    for (int i = 0; i < 3; i++) {
        if (i != current_player && players[i].in_game && !players[i].captured) {
            if (players[i].pos[0] == new_floor && 
                players[i].pos[1] == new_w && 
                players[i].pos[2] == new_l) {
                return i; // Return the ID of the player to be captured
            }
        }
    }
    return -1; // No collision
}

// Rule 5: Capture a player and move them to starting area
void capture_player(Player *captured_player, int captured_id) {
    captured_player->captured = 1;
    captured_player->in_game = 0;
    captured_player->entered_maze = 0;
    captured_player->roll_count = 0;
    
    // Reset to starting position
    captured_player->pos[0] = captured_player->capture_start_pos[0];
    captured_player->pos[1] = captured_player->capture_start_pos[1];
    captured_player->pos[2] = captured_player->capture_start_pos[2];
    
    // Reset direction
    if (captured_id == PLAYER_A) {
        captured_player->direction = DIR_NORTH;
    } else if (captured_id == PLAYER_B) {
        captured_player->direction = DIR_WEST;
    } else if (captured_id == PLAYER_C) {
        captured_player->direction = DIR_EAST;
    }
    
    printf("Player %c has been captured and sent back to starting area!\n", 'A' + captured_id);
}

// Rule 5: Reset captured player status when they roll 6 to enter again
void reset_captured_player(Player *player, int player_id) {
    player->captured = 0;
    printf("Player %c is no longer captured and ready to enter maze again.\n", 'A' + player_id);
}

int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l) {
    for (int i = 0; i < num_stairs; i++) {
        // Rule 6: Check stair direction type for movement validity
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l)) {
            
            // Check if movement is allowed based on direction type
            if (stairs[i].direction_type == STAIR_BIDIRECTIONAL) {
                return i; // Always allowed
            } else if (stairs[i].direction_type == STAIR_UP_ONLY) {
                // Only allow if we're at start (going up)
                if (stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) {
                    return i;
                }
            } else if (stairs[i].direction_type == STAIR_DOWN_ONLY) {
                // Only allow if we're at end (going down)
                if (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l) {
                    return i;
                }
            }
        }
    }
    return -1;
}

int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].w == w && poles[i].l == l) {
            if (floor >= poles[i].end_floor && floor <= poles[i].start_floor) {
                return i;
            }
        }
    }
    return -1;
}

int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] && player->pos[1] == flag[1] && player->pos[2] == flag[2]);
}

int is_valid_position(int floor, int w, int l) {
    return (floor >= 0 && floor < NUM_FLOORS && w >= 0 && w < FLOOR_WIDTH && l >= 0 && l < FLOOR_LENGTH);
}

int can_move_to(int floor, int w, int l, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    if (!is_valid_position(floor, w, l)) return 0;
    return maze[floor][w][l].is_valid && !maze[floor][w][l].has_wall;
}

void move_player_with_teleport(Player *player,
                               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls,
                               int steps) {
    int dx[] = {0, 1, 0, -1}; // North, East, South, West (w direction)
    int dy[] = {-1, 0, 1, 0}; // North, East, South, West (l direction)
    
    for (int step = 0; step < steps; step++) {
        int new_w = player->pos[1] + dx[player->direction];
        int new_l = player->pos[2] + dy[player->direction];
        int new_floor = player->pos[0];
        
        // Check if new position is valid
        if (!can_move_to(new_floor, new_w, new_l, maze)) {
            printf("Player blocked by wall or invalid position. Staying at [%d,%d,%d]\n", 
                   player->pos[0], player->pos[1], player->pos[2]);
            return;
        }
        
        // Move to new position
        player->pos[1] = new_w;
        player->pos[2] = new_l;
        
        // Check for stairs (Rule 4 & 6 implementation)
        int stair_idx = find_stair_at(stairs, num_stairs, player->pos[0], player->pos[1], player->pos[2]);
        if (stair_idx >= 0) {
            printf("Player encounters stair %d (Type: %s)!\n", stair_idx,
                   stairs[stair_idx].direction_type == STAIR_UP_ONLY ? "UP_ONLY" :
                   stairs[stair_idx].direction_type == STAIR_DOWN_ONLY ? "DOWN_ONLY" : "BIDIRECTIONAL");
            
            // Teleport based on stair direction
            if (stairs[stair_idx].start_floor == player->pos[0] && 
                stairs[stair_idx].start_w == player->pos[1] && 
                stairs[stair_idx].start_l == player->pos[2]) {
                // At start position, move to end
                player->pos[0] = stairs[stair_idx].end_floor;
                player->pos[1] = stairs[stair_idx].end_w;
                player->pos[2] = stairs[stair_idx].end_l;
                printf("Teleported to end: [%d,%d,%d]\n", player->pos[0], player->pos[1], player->pos[2]);
            } else {
                // At end position, move to start
                player->pos[0] = stairs[stair_idx].start_floor;
                player->pos[1] = stairs[stair_idx].start_w;
                player->pos[2] = stairs[stair_idx].start_l;
                printf("Teleported to start: [%d,%d,%d]\n", player->pos[0], player->pos[1], player->pos[2]);
            }
        }
        
        // Check for poles
        int pole_idx = find_pole_at(poles, num_poles, player->pos[0], player->pos[1], player->pos[2]);
        if (pole_idx >= 0) {
            printf("Player encounters pole %d!\n", pole_idx);
            player->pos[0] = poles[pole_idx].end_floor;
            printf("Teleported down to: [%d,%d,%d]\n", player->pos[0], player->pos[1], player->pos[2]);
        }
    }
}

void play_turn(int player_id, Player players[3],
               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs,
               Pole poles[], int num_poles,
               Wall walls[], int num_walls,
               int flag[3]) {
                   
    Player *p = &players[player_id];
    
    printf("\n=== Player %c's Turn ===\n", 'A' + player_id);
    
    // Rule 5: Check if player is captured
    if (p->captured) {
        printf("Player %c is captured and in starting area.\n", 'A' + player_id);
    }
    
    // Wait for player to press Enter, then roll movement dice automatically
    wait_for_enter("Press Enter to roll movement dice: ");
    int move_roll = roll_movement_dice();
    p->movement_points -= 2; // Cost 2 points per roll
    printf("Movement dice: %d\n", move_roll);
    
    // Entry logic: need 6 to enter or re-enter after capture
    if (!p->in_game) {
        if (move_roll == 6) {
            if (p->captured) {
                reset_captured_player(p, player_id); // Rule 5
            }
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf("Player %c enters the maze at [%d,%d,%d]\n", 
                   'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter. Stay in starting area.\n", 'A' + player_id);
            return;
        }
    } else {
        p->roll_count++;
    }
    
    // Direction Die: only every 4th roll
    if (p->roll_count % 4 == 0) {
        int dir_roll = roll_direction_dice();
        const char* dir_names[] = {"North", "East", "South", "West"};
        
        if (dir_roll != DIR_EMPTY) {
            p->direction = dir_roll;
            printf("Direction changed to: %s\n", dir_names[p->direction]);
        } else {
            printf("Direction unchanged: %s\n", dir_names[p->direction]);
        }
    }
    
    // Move player
    int old_pos[3] = {p->pos[0], p->pos[1], p->pos[2]};
    move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, move_roll);
    
    // Rule 5: Check for player collision after movement
    if (p->in_game && !p->captured) {
        int captured_player_id = check_player_collision(players, player_id, p->pos[0], p->pos[1], p->pos[2]);
        if (captured_player_id != -1) {
            printf("Player %c captures Player %c!\n", 'A' + player_id, 'A' + captured_player_id);
            capture_player(&players[captured_player_id], captured_player_id);
        }
    }
    
    printf("Player %c at [%d,%d,%d]\n", 'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
    
    // Check for flag capture
    if (check_flag_capture(p, flag)) {
        printf("*** Player %c captured the flag! ***\n", 'A' + player_id);
        printf("GAME OVER! Player %c wins!\n", 'A' + player_id);
        exit(0);
    }
    
    // Rule 6: Check if round is completed and update stair directions
    if (check_round_completion()) {
        printf("Round %d completed!\n", rounds_completed);
        update_stair_directions(stairs, num_stairs);
    }
}

void display_game_state(Player players[3], Stair stairs[], int num_stairs,
                       Pole poles[], int num_poles, Wall walls[], int num_walls,
                       int flag[3]) {
    printf("\n=== GAME STATE ===\n");
    printf("Flag at [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    printf("Round: %d, Stair change in: %d rounds\n", rounds_completed, 5 - stair_change_counter);
    
    for (int i = 0; i < 3; i++) {
        printf("Player %c: [%d,%d,%d], Direction: %s, Points: %d", 
               'A' + i, players[i].pos[0], players[i].pos[1], players[i].pos[2],
               players[i].direction == DIR_NORTH ? "North" :
               players[i].direction == DIR_EAST ? "East" :
               players[i].direction == DIR_SOUTH ? "South" : "West",
               players[i].movement_points);
               
        if (players[i].captured) {
            printf(" [CAPTURED]");
        } else if (!players[i].in_game) {
            printf(" [IN STARTING AREA]");
        }
        printf("\n");
    }
    
    printf("\nStairs:\n");
    for (int i = 0; i < num_stairs; i++) {
        const char* dir_type = stairs[i].direction_type == STAIR_UP_ONLY ? "UP_ONLY" :
                              stairs[i].direction_type == STAIR_DOWN_ONLY ? "DOWN_ONLY" : "BIDIRECTIONAL";
        printf("  Stair %d: [%d,%d,%d] -> [%d,%d,%d] (%s)\n", i,
               stairs[i].start_floor, stairs[i].start_w, stairs[i].start_l,
               stairs[i].end_floor, stairs[i].end_w, stairs[i].end_l, dir_type);
    }
    
    printf("\nPoles:\n");
    for (int i = 0; i < num_poles; i++) {
        printf("  Pole %d: [%d->%d,%d,%d]\n", i,
               poles[i].start_floor, poles[i].end_floor, poles[i].w, poles[i].l);
    }
    
    printf("\nWalls:\n");
    for (int i = 0; i < num_walls; i++) {
        printf("  Wall %d: Floor %d from [%d,%d] to [%d,%d]\n", i,
               walls[i].floor, walls[i].start_w, walls[i].start_l, walls[i].end_w, walls[i].end_l);
    }
    printf("==================\n\n");
}

int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                     int start_floor, int start_w, int start_l,
                     int end_floor, int end_w, int end_l) {
    // Simple wall blocking check - can be enhanced
    if (start_floor != end_floor) return 0; // Different floors
    
    return maze[start_floor][end_w][end_l].has_wall;
}