#include "game.h"

static void wait_for_enter(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        // Handle input error if needed
    }
}

void play_turn(int player_id, Player players[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs, Pole poles[], int num_poles,
               Wall walls[], int num_walls, int flag[3]) {
    Player *p = &players[player_id];
    char player_name = 'A' + player_id;
    
    printf("\n=== Player %c's Turn ===\n", player_name);
    
    // Handle food poisoning
    if (p->bawana_effect == EFFECT_FOOD_POISONING) {
        p->bawana_turns_left--;
        printf("%c is still food poisoned and misses the turn.\n", player_name);
        
        if (p->bawana_turns_left == 0) {
            p->bawana_effect = EFFECT_NONE;
            
            if (p->movement_points <= 0) {
                int bawana_cells[12][2] = {
                    {6,21}, {6,22}, {6,23}, {6,24},
                    {7,21}, {7,22}, {7,23}, {7,24},
                    {8,21}, {8,22}, {8,23}, {8,24}
                };
                int idx = rand() % 12;
                p->pos[0] = 0;
                p->pos[1] = bawana_cells[idx][0];
                p->pos[2] = bawana_cells[idx][1];
                
                // Get cell type for the message
                int cell_type = maze[p->pos[0]][p->pos[1]][p->pos[2]].bawana_cell_type;
                const char* cell_type_names[] = {"food poisoning", "disoriented", "triggered", "happy", "random MP"};
                const char* cell_type_name = (cell_type >= 0 && cell_type < 5) ? cell_type_names[cell_type] : "random";
                
                printf("%c is now fit to proceed from the food poisoning episode and now placed on a %s cell and the effects take place.\n", 
                       player_name, cell_type_name);
                apply_bawana_effect(p, maze, player_id);
            } else {
                printf("%c has recovered from food poisoning and can resume normal play.\n", player_name);
            }
        }
        return;
    }
    
    // Track throws after entering maze - each player has individual direction die timing
    int throw_count = p->roll_count - 1; // Subtract 1 since roll_count increments after each turn
    int is_direction_dice_roll = (p->in_game && throw_count > 0 && (throw_count % 4 == 0));
    
    if (is_direction_dice_roll) {
        wait_for_enter("Press Enter to roll direction die: ");
        int dir_roll = roll_direction_dice();
        const char* rolled_direction = "";
        
        if (dir_roll == 2) { 
            p->direction = DIR_NORTH; 
            rolled_direction = "North";
        } else if (dir_roll == 3) { 
            p->direction = DIR_EAST;  
            rolled_direction = "East";
        } else if (dir_roll == 4) { 
            p->direction = DIR_SOUTH; 
            rolled_direction = "South";
        } else if (dir_roll == 5) { 
            p->direction = DIR_WEST;  
            rolled_direction = "West";
        } else { 
            rolled_direction = "Empty";
        }
        
        printf("Direction die: %d (%s)\n", dir_roll, rolled_direction);
        
        if (dir_roll == 1 || dir_roll == 6) {
            printf("Direction unchanged: %s\n", get_direction_name(p->direction));
        } else {
            printf("Direction changed to: %s\n", get_direction_name(p->direction));
        }
    }
    
    wait_for_enter("Press Enter to roll movement die: ");
    int move_roll = roll_movement_dice();
    printf("Movement die: %d\n", move_roll);
    
    // Handle starting area logic
    if (!p->in_game) {
        if (move_roll == 6) {
            // Check if player is at Player A's starting area (captured/reset)
            if (p->pos[0] == 0 && p->pos[1] == 6 && p->pos[2] == 12) {
                // Enter maze like Player A
                enter_maze_like_player_a(p);
                printf("%c is at Player A's starting area and rolls 6 on the movement dice and is placed on Player A's first maze cell %s.\n", 
                       player_name, format_position(p->pos[0], p->pos[1], p->pos[2]));
            } else {
                // Normal entry for original starting positions
                enter_maze(p, player_id);
                printf("%c is at the starting area and rolls 6 on the movement dice and is placed on the first maze cell %s.\n", 
                       player_name, format_position(p->pos[0], p->pos[1], p->pos[2]));
            }
            
            printf("%c moved 0 cells that cost 0 movement points and is left with %d and is moving in the %s.\n", 
                   player_name, p->movement_points, get_direction_name(p->direction));
            
            return;
        } else {
            printf("%c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", 
                   player_name, move_roll);
            printf("%c moved 0 cells that cost 0 movement points and is left with %d and is moving in the %s.\n", 
                   player_name, p->movement_points, get_direction_name(p->direction));
            
            if (p->movement_points <= 0) {
                reset_to_bawana(p, player_id);
                // Apply Bawana effects after transportation
                apply_bawana_effect(p, maze, player_id);
            }
            
            return;
        }
    }

    // Player is in maze - handle movement
    int original_move = move_roll;
    int movement_cost = 0;
    int actual_steps = 0;
    int blockage_reason = BLOCK_NONE;
    
    // Apply Bawana effects on movement
    if (p->bawana_effect == EFFECT_DISORIENTED) {
        p->direction = rand() % 4;
    } else if (p->bawana_effect == EFFECT_TRIGGERED) {
        move_roll *= 2;
    }

    // Store old position
    int old_pos[3] = {p->pos[0], p->pos[1], p->pos[2]};
    
    // Print initial movement message based on dice rolls
    if (is_direction_dice_roll) {
        if (p->bawana_effect == EFFECT_TRIGGERED) {
            printf("%c is triggered and rolls and %d on the movement dice and move in the %s", 
                   player_name, original_move, get_direction_name(p->direction));
        } else {
            printf("%c rolls and %d on the movement dice", player_name, original_move);
            
            if (p->bawana_effect == EFFECT_DISORIENTED) {
                printf(" and is disoriented and move in the %s", get_direction_name(p->direction));
            } else {
                printf(" and moves %s", get_direction_name(p->direction));
            }
        }
    } else {
        if (p->bawana_effect == EFFECT_TRIGGERED) {
            printf("%c is triggered and rolls and %d on the movement dice and move in the %s", 
                   player_name, original_move, get_direction_name(p->direction));
        } else {
            printf("%c rolls and %d on the movement dice", player_name, original_move);
            
            if (p->bawana_effect == EFFECT_DISORIENTED) {
                printf(" and is disoriented and move in the %s", get_direction_name(p->direction));
            } else {
                printf(" and moves %s", get_direction_name(p->direction));
            }
        }
    }
    
    // FIXED: Try to move player with enhanced feedback
    int movement_successful = move_player_with_teleport(p, maze, stairs, num_stairs, 
                                                       poles, num_poles, walls, num_walls, 
                                                       move_roll, player_id, flag,
                                                       &movement_cost, &actual_steps, &blockage_reason);

    // FIXED: Enhanced output with clear blockage reasons and correct MP costs
    if (old_pos[0] == p->pos[0] && old_pos[1] == p->pos[1] && old_pos[2] == p->pos[2]) {
        // No movement occurred
        if (blockage_reason != BLOCK_NONE) {
            printf(" and cannot move in the %s due to %s. Player remains at %s\n", 
                   get_direction_name(p->direction), 
                   get_blockage_reason_description(blockage_reason),
                   format_position(p->pos[0], p->pos[1], p->pos[2]));
        } else {
            printf(" and cannot move in the %s. Player remains at %s\n", 
                   get_direction_name(p->direction), 
                   format_position(p->pos[0], p->pos[1], p->pos[2]));
        }
        
        // Apply unified movement cost, deduct from MP
        p->movement_points -= movement_cost;
        
        printf("%c moved 0 cells that cost %d movement points and is left with %d and is moving in the %s.\n", 
               player_name, movement_cost, p->movement_points, get_direction_name(p->direction));
    } else {
        // Movement occurred
        p->movement_points -= movement_cost;
        
        // Check if player is disoriented or triggered for special message format
        if (p->bawana_effect == EFFECT_DISORIENTED) {
            printf(" and moves %d cells and is placed at %s.\n", actual_steps, format_position(p->pos[0], p->pos[1], p->pos[2]));
        } else if (p->bawana_effect == EFFECT_TRIGGERED) {
            printf(" and moves %d cells and is placed at %s.\n", actual_steps, format_position(p->pos[0], p->pos[1], p->pos[2]));
        } else {
            printf(" by %d cells and is now at %s.\n", actual_steps, format_position(p->pos[0], p->pos[1], p->pos[2]));
        }
        
        printf("%c moved %d cells that cost %d movement points and is left with %d and is moving in the %s.\n", 
               player_name, actual_steps, movement_cost, p->movement_points, get_direction_name(p->direction));
    }

    // Handle other Bawana effects countdown
    if (p->bawana_effect > EFFECT_NONE && p->bawana_effect != EFFECT_FOOD_POISONING && p->bawana_effect != EFFECT_HAPPY) {
        p->bawana_turns_left--;
        if (p->bawana_turns_left == 0) {
            if (p->bawana_effect == EFFECT_DISORIENTED) {
                printf("%c has recovered from disorientation.\n", player_name);
            } else if (p->bawana_effect == EFFECT_TRIGGERED) {
                printf("%c has recovered from being triggered.\n", player_name);
            } else if (p->bawana_effect == EFFECT_RANDOM_MP) {
                printf("%c's random movement point effect has expired.\n", player_name);
            }
            p->bawana_effect = EFFECT_NONE;
        }
    }

    // Check if movement points are depleted
    if (p->movement_points <= 0) {
        reset_to_bawana(p, player_id);
        // Apply Bawana effects after transportation
        apply_bawana_effect(p, maze, player_id);
    }

    // Check for player captures
    check_player_capture(players, player_id);

    // Check for flag capture
    if (check_flag_capture(p, flag)) {
        printf("Player %c has captured the flag!\n", player_name);
        printf("Player %c wins the game!\n", player_name);
        exit(0);
    }

    p->roll_count++;
}

void print_game_status(Player players[3], int flag[3]) {
    printf("\n--- Game Status ---\n");
    printf("Flag location: [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    for (int i = 0; i < 3; i++) {
        char player_name = 'A' + i;
        printf("Player %c: [%d,%d,%d] - ", player_name, players[i].pos[0], players[i].pos[1], players[i].pos[2]);
        if (players[i].in_game) {
            printf("In maze, MP: %d", players[i].movement_points);
            if (players[i].bawana_effect > EFFECT_NONE) {
                const char* effect_names[] = {"None", "Food Poisoning", "Disoriented", "Triggered", "Happy", "Random MP"};
                printf(", Bawana effect: %s", effect_names[players[i].bawana_effect]);
                if (players[i].bawana_effect != EFFECT_HAPPY) {
                    printf(" (turns left: %d)", players[i].bawana_turns_left);
                }
            }
        } else {
            printf("In starting area, MP: %d", players[i].movement_points);
        }
        printf("\n");
    }
    printf("-------------------\n");
}

int main(void) {
    int seed = read_seed_from_file("seed.txt");
    srand((unsigned int)seed);
    printf("Using seed: %d\n", seed);
    
    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;
    int flag[3];
    
    initialize_maze(maze);
    initialize_players(players);
    
    if (!read_stairs_from_file("stairs.txt", stairs, &num_stairs)) {
        initialize_stairs(stairs, &num_stairs);
        printf("Using default stairs configuration.\n");
    }
    
    if (!read_poles_from_file("poles.txt", poles, &num_poles)) {
        initialize_poles(poles, &num_poles);
        printf("Using default poles configuration.\n");
    }
    
    if (!read_walls_from_file("walls.txt", walls, &num_walls)) {
        initialize_walls(walls, &num_walls);
        printf("Using default walls configuration.\n");
    }
    
    // Block intermediate floors for skipping stairs
    for (int i = 0; i < num_stairs; i++) {
        int start_floor = stairs[i].start_floor;
        int end_floor = stairs[i].end_floor;
        int start_w = stairs[i].start_w;
        int start_l = stairs[i].start_l;
        int end_w = stairs[i].end_w;
        int end_l = stairs[i].end_l;
        
        int min_f = (start_floor < end_floor) ? start_floor : end_floor;
        int max_f = (start_floor > end_floor) ? start_floor : end_floor;
        
        if (max_f - min_f > 1) {
            // Block intermediate floors - use the coordinates from the start floor
            for (int f = min_f + 1; f < max_f; f++) {
                maze[f][start_w][start_l].is_blocked_by_stair = 1;
                printf("Blocked cell [%d,%d,%d] for skipping stair.\n", f, start_w, start_l);
            }
        }
    }
    
    if (!read_flag_from_file("flag.txt", flag)) {
        place_random_flag(flag, maze);
        printf("Flag randomly placed at [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    }
    
    printf("\n=== Maze of UCSC ===\n");
    printf("Flag is placed at [%d,%d,%d]\n\n", flag[0], flag[1], flag[2]);
    
    print_game_status(players, flag);
    
    int round = 1;
    while (1) { 
        printf("\n=== Round %d ===\n", round);
        update_stair_directions(stairs, num_stairs);
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, flag);
        }
        print_game_status(players, flag);
        round++;
    }
    
    return 0;
}