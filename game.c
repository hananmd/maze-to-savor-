// game.c - Main game logic implementation

#include "game.h"
#include <stdlib.h>

// Helper function to convert direction enum to readable string
const char* get_direction_name(int direction) {
    switch(direction) {
        case DIR_NORTH: return "North";
        case DIR_EAST: return "East";
        case DIR_SOUTH: return "South";
        case DIR_WEST: return "West";
        default: return "Unknown"; // Should never happen, but good to have
    }
}

// Format position coordinates into a nice string like [floor,width,length]
const char* format_position(int floor, int width_pos, int length_pos) {
    static char position_buffer[20];
    snprintf(position_buffer, sizeof(position_buffer), "[%d,%d,%d]", floor, width_pos, length_pos);
    return position_buffer;
}

// Find a pole at a specific position (poles span multiple floors)
int find_pole_at(Pole poles[], int num_poles, int floor, int width_pos, int length_pos) {
    for (int pole_idx = 0; pole_idx < num_poles; pole_idx++) {
        if (poles[pole_idx].w == width_pos && poles[pole_idx].l == length_pos) {
            // Check if the pole spans through this floor
            if ((poles[pole_idx].start_floor >= floor && poles[pole_idx].end_floor <= floor) ||
                (poles[pole_idx].start_floor <= floor && poles[pole_idx].end_floor >= floor)) {
                return pole_idx;
            }
        }
    }
    return -1; // No pole found at this position
}

// Calculate Manhattan distance between two 3D points
int manhattan_distance(int floor1, int w1, int l1, int floor2, int w2, int l2) {
    return abs(floor1 - floor2) + abs(w1 - w2) + abs(l1 - l2);
}

// Comprehensive path validation - checks entire movement path before executing
int check_path_validity(Player *player,
                        Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                        Stair stairs[], int num_stairs,
                        Pole poles[], int num_poles,
                        Wall walls[], int num_walls,
                        int steps, int player_id, const int flag_position[3],
                        int *first_blocked_step, int *blocking_reason) {
    int current_width = player->pos[1];
    int current_length = player->pos[2];
    int current_floor = player->pos[0];
    
    *first_blocked_step = -1;
    *blocking_reason = BLOCK_NONE;
    
    // Simulate each step of the movement
    for (int step_num = 0; step_num < steps; step_num++) {
        int next_width = current_width;
        int next_length = current_length;

        // Calculate next position based on current direction
        switch (player->direction) {
            case DIR_NORTH: next_length--; break;
            case DIR_EAST:  next_width++; break;
            case DIR_SOUTH: next_length++; break;
            case DIR_WEST:  next_width--; break;
        }

        // Walls between cells
        if (is_wall_blocking(maze, current_floor, current_width, current_length, next_width, next_length, walls, num_walls)) {
            *first_blocked_step = step_num;
            *blocking_reason = BLOCK_WALL;
            return 0;
        }
        // Valid destination
        if (!is_valid_position(maze, current_floor, next_width, next_length)) {
            *first_blocked_step = step_num;
            *blocking_reason = BLOCK_INVALID_CELL;
            return 0;
        }
        // Bawana entrance restriction
        if (!can_enter_bawana_entrance(player, maze, next_width, next_length)) {
            *first_blocked_step = step_num;
            *blocking_reason = BLOCK_BAWANA_ENTRANCE;
            return 0;
        }

        // Advance to next cell
        current_width = next_width;
        current_length = next_length;

        // If landing on stairs, simulate teleport using same tie-break as runtime
        int stair_indices_found[MAX_STAIRS];
        int stairs_here = find_all_stairs_at(stairs, num_stairs, current_floor, current_width, current_length, stair_indices_found);
        if (stairs_here > 0) {
            int chosen_idx = -1;
            if (stairs_here == 1) {
                chosen_idx = stair_indices_found[0];
            } else {
                int best_distance = 999999, best_idx = -1;
                int tied[MAX_STAIRS]; int tied_n = 0;
                for (int i = 0; i < stairs_here; i++) {
                    Stair *st = &stairs[stair_indices_found[i]];
                    int df, dw, dl;
                    if (current_floor == st->start_floor) { df = st->end_floor; dw = st->end_w; dl = st->end_l; }
                    else { df = st->start_floor; dw = st->start_w; dl = st->start_l; }
                    int d = manhattan_distance(df, dw, dl, flag_position[0], flag_position[1], flag_position[2]);
                    if (d < best_distance) { best_distance = d; best_idx = stair_indices_found[i]; tied_n = 0; tied[tied_n++] = stair_indices_found[i]; }
                    else if (d == best_distance) { tied[tied_n++] = stair_indices_found[i]; }
                }
                chosen_idx = tied[0]; // deterministic for validation
            }
            Stair *sel = &stairs[chosen_idx];
            int df, dw, dl;
            if (current_floor == sel->start_floor) { df = sel->end_floor; dw = sel->end_w; dl = sel->end_l; }
            else { df = sel->start_floor; dw = sel->start_w; dl = sel->start_l; }

            // Check direction permission
            int allowed = (sel->direction_type == STAIR_BIDIRECTIONAL) ||
                          (sel->direction_type == STAIR_UP_ONLY && current_floor == sel->start_floor) ||
                          (sel->direction_type == STAIR_DOWN_ONLY && current_floor == sel->end_floor);
            if (!allowed) {
                *first_blocked_step = step_num;
                *blocking_reason = BLOCK_INVALID_CELL;
                return 0;
            }
            // Teleport
            current_floor = df; current_width = dw; current_length = dl;
        }

        // If landing on a pole start, simulate slide
        for (int p = 0; p < num_poles; p++) {
            if (poles[p].start_floor == current_floor && poles[p].w == current_width && poles[p].l == current_length) {
                current_floor = poles[p].end_floor;
                current_width = poles[p].w;
                current_length = poles[p].l;
                break;
            }
        }
    }
    
    return 1; // Entire path is clear
}

// Apply special effects when player lands on a Bawana cell
void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id) {
    int current_floor = player->pos[0];
    int current_width = player->pos[1];
    int current_length = player->pos[2];
    
    // Only apply effects if player is in Bawana area (Floor 0, w=6-9, l=20-24)
    if (current_floor != 0 || current_width < 6 || current_width > 9 || current_length < 20 || current_length > 24) return;
    
    // Don't apply effect if player already has one
    if (player->bawana_effect != EFFECT_NONE) return;

    int cell_effect_type = maze[current_floor][current_width][current_length].bawana_cell_type;
    char player_letter = 'A' + player_id;
    
    // Required message: Announce what type of cell the player landed on
    const char* effect_names[] = {"food poisoning", "disoriented", "triggered", "happy", "random MP"};
    if (cell_effect_type >= 0 && cell_effect_type < 5) {
        printf("%c is placed on a %s cell and effects take place.\n", player_letter, effect_names[cell_effect_type]);
    } else {
        printf("%c is placed on a random cell and effects take place.\n", player_letter);
    }

    // Helper function to ensure MP awards are applied correctly
    // If player has negative MP, first normalize to 0, then add the bonus
    int normalize_mp_then_add(int mp_to_add) {
        if (player->movement_points < 0) {
            player->movement_points = 0;
        }
        player->movement_points += mp_to_add;
        return player->movement_points;
    }

    switch (cell_effect_type) {
        case BA_FOOD_POISONING:
            player->bawana_effect = EFFECT_FOOD_POISONING;
            player->bawana_turns_left = 3;
            printf("%c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", player_letter);
            break;
            
        case BA_DISORIENTED:
            player->bawana_effect = EFFECT_DISORIENTED;
            player->bawana_turns_left = 4;
            normalize_mp_then_add(50);
            // Move to Bawana entrance
            player->pos[1] = 9; 
            player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", player_letter);
            break;
            
        case BA_TRIGGERED:
            player->bawana_effect = EFFECT_TRIGGERED;
            player->bawana_turns_left = 4; // Fixed duration
            normalize_mp_then_add(50);
            // Move to Bawana entrance
            player->pos[1] = 9; 
            player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is triggered due to bad quality of food. %c is placed at the entrance of Bawana with 50 movement points.\n", player_letter, player_letter);
            break;
            
        case BA_HAPPY:
            // Happy effect gives immediate boost with no lasting status
            player->bawana_effect = EFFECT_NONE;
            player->bawana_turns_left = 0;
            normalize_mp_then_add(200);
            // Move to Bawana entrance
            player->pos[1] = 9; 
            player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and is happy. %c is placed at the entrance of Bawana with 200 movement points.\n", player_letter, player_letter);
            break;
            
        case BA_RANDOM_MP:
            player->bawana_effect = EFFECT_RANDOM_MP;
            player->bawana_turns_left = 4;
            player->bawana_random_mp = (rand() % 91) + 10; // Random 10-100
            normalize_mp_then_add(player->bawana_random_mp);
            // Move to Bawana entrance
            player->pos[1] = 9; 
            player->pos[2] = 19;
            player->direction = DIR_NORTH;
            printf("%c eats from Bawana and earns %d movement points and is placed at the %s.\n", 
                   player_letter, player->bawana_random_mp, format_position(player->pos[0], player->pos[1], player->pos[2]));
            break;
    }
}

// Check if a position is within the starting area rectangle
int is_in_starting_area(int floor, int width_pos, int length_pos) {
    return (floor == 0 &&
            width_pos >= START_AREA_W_MIN && width_pos <= START_AREA_W_MAX &&
            length_pos >= START_AREA_L_MIN && length_pos <= START_AREA_L_MAX);
}

// Reset player to starting area when trapped in infinite loop
void reset_to_starting_area(Player *player, int player_id) {
    char player_letter = 'A' + player_id;
    printf("Player %c trapped in Infinite Loop - resetting to Player A's starting area. Movement points preserved.\n", player_letter);
    
    // All players go to Player A's starting position when reset
    player->pos[0] = 0;
    player->pos[1] = 6;
    player->pos[2] = 12;
    player->direction = DIR_NORTH; // Same direction as Player A
    player->in_game = 0; // Back to starting area
    player->captured = 0;
}

// Main movement function with teleportation handling (stairs/poles)
int move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps, int player_id, const int flag_position[3],
                               int *total_movement_cost, int *actual_steps_taken, int *blocking_reason) {
    
    int blocked_at_step;
    int reason_for_blocking;
    
    // Initialize return values
    if (total_movement_cost) *total_movement_cost = 0;
    if (actual_steps_taken) *actual_steps_taken = 0;
    if (blocking_reason) *blocking_reason = BLOCK_NONE;
    
    // Pre-validate the entire path
    int path_is_clear = check_path_validity(player, maze,
                                           stairs, num_stairs,
                                           poles, num_poles,
                                           walls, num_walls,
                                           steps, player_id, flag_position,
                                           &blocked_at_step, &reason_for_blocking);
    
    if (blocking_reason) {
        *blocking_reason = reason_for_blocking;
    }
    
    if (!path_is_clear) {
        if (total_movement_cost) *total_movement_cost = 2; // Standard cost for being blocked
        return 0; // Movement failed due to obstruction
    }
    
    // Path is valid, execute the movement step by step
    int visited_positions[MAX_LOOP_HISTORY][3]; // Track positions to detect loops
    int visit_counter = 0;
    char player_letter = 'A' + player_id;

    for (int current_step = 0; current_step < steps; current_step++) {
        int old_floor = player->pos[0];
        int old_width = player->pos[1];
        int old_length = player->pos[2];

        // Record current position for loop detection
        visited_positions[visit_counter][0] = old_floor;
        visited_positions[visit_counter][1] = old_width;
        visited_positions[visit_counter][2] = old_length;
        visit_counter++;

        if (visit_counter >= MAX_LOOP_HISTORY) {
            visit_counter = 0; // Wrap around if we exceed history limit
        }

        int new_width = old_width;
        int new_length = old_length;

        // Calculate new position based on current direction
        switch (player->direction) {
            case DIR_NORTH: new_length--; break;
            case DIR_EAST:  new_width++; break;
            case DIR_SOUTH: new_length++; break;
            case DIR_WEST:  new_width--; break;
        }

        // Execute the basic movement
        player->pos[1] = new_width;
        player->pos[2] = new_length;

        // Add consumable cost of this cell to total cost
        if (total_movement_cost) {
            *total_movement_cost += maze[player->pos[0]][player->pos[1]][player->pos[2]].consumable_value;
        }

        // Check for infinite loop after basic movement
        for (int history_idx = 0; history_idx < visit_counter - 1; history_idx++) {
            if (visited_positions[history_idx][0] == player->pos[0] &&
                visited_positions[history_idx][1] == player->pos[1] &&
                visited_positions[history_idx][2] == player->pos[2]) {
                printf("Infinite loop detected at [%d,%d,%d]!\n", player->pos[0], player->pos[1], player->pos[2]);
                reset_to_starting_area(player, player_id);
                return 1; // Movement completed (via loop reset)
            }
        }

        // Check for stairs at new position
        int stair_indices_found[MAX_STAIRS];
        int num_stairs_found = find_all_stairs_at(stairs, num_stairs, old_floor, new_width, new_length, stair_indices_found);

        if (num_stairs_found > 0) {
            printf("%c lands on %s which is a stair cell.\n", player_letter, format_position(old_floor, new_width, new_length));

            int chosen_stair_idx = -1;

            if (num_stairs_found == 1) {
                chosen_stair_idx = stair_indices_found[0];
            } else {
                // Multiple stairs: choose the one that gets closest to flag
                int best_distance = 999999;
                int best_stair_idx = -1;
                int tied_stairs[MAX_STAIRS];
                int num_tied = 0;

                for (int i = 0; i < num_stairs_found; i++) {
                    int stair_idx = stair_indices_found[i];
                    Stair *current_stair = &stairs[stair_idx];

                    int destination_floor, dest_width, dest_length;
                    if (old_floor == current_stair->start_floor) {
                        destination_floor = current_stair->end_floor; 
                        dest_width = current_stair->end_w; 
                        dest_length = current_stair->end_l;
                    } else {
                        destination_floor = current_stair->start_floor; 
                        dest_width = current_stair->start_w; 
                        dest_length = current_stair->start_l;
                    }

                    int distance_to_flag = manhattan_distance(destination_floor, dest_width, dest_length, flag_position[0], flag_position[1], flag_position[2]);
                    if (distance_to_flag < best_distance) {
                        best_distance = distance_to_flag;
                        best_stair_idx = stair_idx;
                        num_tied = 0;
                        tied_stairs[num_tied++] = stair_idx;
                    } else if (distance_to_flag == best_distance) {
                        tied_stairs[num_tied++] = stair_idx;
                    }
                }

                if (num_tied > 1) {
                    chosen_stair_idx = tied_stairs[rand() % num_tied];
                    printf("Multiple stairs at same distance - randomly chose one.\n");
                } else {
                    chosen_stair_idx = best_stair_idx;
                }
            }

            Stair *selected_stair = &stairs[chosen_stair_idx];
            // Check if stair allows movement in this direction
            if (selected_stair->direction_type == STAIR_BIDIRECTIONAL ||
                (selected_stair->direction_type == STAIR_UP_ONLY && old_floor == selected_stair->start_floor) ||
                (selected_stair->direction_type == STAIR_DOWN_ONLY && old_floor == selected_stair->end_floor)) {

                int destination_floor, dest_width, dest_length;
                if (old_floor == selected_stair->start_floor) {
                    destination_floor = selected_stair->end_floor; 
                    dest_width = selected_stair->end_w; 
                    dest_length = selected_stair->end_l;
                } else {
                    destination_floor = selected_stair->start_floor; 
                    dest_width = selected_stair->start_w; 
                    dest_length = selected_stair->start_l;
                }

                player->pos[0] = destination_floor;
                player->pos[1] = dest_width;
                player->pos[2] = dest_length;

                printf("%c takes the stairs and now placed at %s in floor %d.\n",
                       player_letter, format_position(player->pos[0], player->pos[1], player->pos[2]), player->pos[0]);

                // Check for infinite loop after stair teleportation
                for (int history_idx = 0; history_idx < visit_counter; history_idx++) {
                    if (visited_positions[history_idx][0] == player->pos[0] &&
                        visited_positions[history_idx][1] == player->pos[1] &&
                        visited_positions[history_idx][2] == player->pos[2]) {
                        printf("Infinite loop detected after stair teleportation at [%d,%d,%d]!\n", 
                               player->pos[0], player->pos[1], player->pos[2]);
                        reset_to_starting_area(player, player_id);
                        return 1; // Movement completed (via loop reset)
                    }
                }

                // Check if player fell back into starting area via stairs
                if (is_in_starting_area(player->pos[0], player->pos[1], player->pos[2])) {
                    printf("%c fell into starting area via stair - must roll 6 to re-enter.\n", player_letter);
                    player->in_game = 0;
                }

                continue; // Skip to next step
            }
        }

        // Check for poles at new position
        int pole_idx = find_pole_at(poles, num_poles, old_floor, new_width, new_length);
        if (pole_idx != -1) {
            Pole *current_pole = &poles[pole_idx];
            printf("%c lands on %s which is a pole cell.\n", player_letter, format_position(old_floor, new_width, new_length));

            player->pos[0] = current_pole->end_floor;
            player->pos[1] = current_pole->w;
            player->pos[2] = current_pole->l;

            printf("%c slides down and now placed at %s in floor %d.\n",
                   player_letter, format_position(player->pos[0], player->pos[1], player->pos[2]), player->pos[0]);

            // Check for infinite loop after pole teleportation
            for (int history_idx = 0; history_idx < visit_counter; history_idx++) {
                if (visited_positions[history_idx][0] == player->pos[0] &&
                    visited_positions[history_idx][1] == player->pos[1] &&
                    visited_positions[history_idx][2] == player->pos[2]) {
                    printf("Infinite loop detected after pole teleportation at [%d,%d,%d]!\n", 
                           player->pos[0], player->pos[1], player->pos[2]);
                    reset_to_starting_area(player, player_id);
                    return 1; // Movement completed (via loop reset)
                }
            }

            // Check if player fell back into starting area via pole
            if (is_in_starting_area(player->pos[0], player->pos[1], player->pos[2])) {
                printf("%c fell into starting area via pole - must roll 6 to re-enter.\n", player_letter);
                player->in_game = 0;
            }

            continue; // Skip to next step
        }

        // Apply Bawana effects if player landed in Bawana area
        if (old_floor == 0 && new_width >= 6 && new_width <= 9 && new_length >= 20 && new_length <= 24) {
            apply_bawana_effect(player, maze, player_id);
        }
        
        // Apply movement bonus if available at this cell
        apply_movement_bonus(player, maze, player_id);
    }
    
    // Set final return values for successful movement
    if (actual_steps_taken) *actual_steps_taken = steps;
    
    return 1; // Movement completed successfully
}

// Randomly place the flag in a valid maze position
void place_random_flag(int flag_position[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    int valid_flag_positions[1000][3]; // Store all valid positions
    int num_valid_positions = 0;
    
    for (int floor_num = 0; floor_num < NUM_FLOORS; floor_num++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                // Must be a valid maze cell
                if (!maze[floor_num][w][l].is_valid) continue;
                // Exclude starting area
                if (maze[floor_num][w][l].is_starting_area) continue;
                // Exclude walls
                if (maze[floor_num][w][l].has_wall) continue;
                // Exclude cells blocked by stairs
                if (maze[floor_num][w][l].is_blocked_by_stair) continue;
                // Exclude Bawana entrance
                if (maze[floor_num][w][l].is_bawana_entrance) continue;
                // Exclude Bawana interior
                if (floor_num == 0 && w >= 6 && w <= 9 && l >= 20 && l <= 24) continue;

                valid_flag_positions[num_valid_positions][0] = floor_num;
                valid_flag_positions[num_valid_positions][1] = w;
                valid_flag_positions[num_valid_positions][2] = l;
                num_valid_positions++;
            }
        }
    }
    
    if (num_valid_positions == 0) { 
        printf("Error: No valid positions for flag placement!\n");
        exit(1); 
    }
    
    int random_choice = rand() % num_valid_positions;
    flag_position[0] = valid_flag_positions[random_choice][0];
    flag_position[1] = valid_flag_positions[random_choice][1];
    flag_position[2] = valid_flag_positions[random_choice][2];
}

// Check if player has reached the flag
int check_flag_capture(Player *player, const int flag_position[3]) {
    return (player->pos[0] == flag_position[0] && 
            player->pos[1] == flag_position[1] && 
            player->pos[2] == flag_position[2]);
}

// Check if current player has captured any other players at the same position
void check_player_capture(Player players[3], int current_player_id) {
    for (int other_player = 0; other_player < 3; other_player++) {
        if (other_player == current_player_id || !players[other_player].in_game) continue;
        
        // Check if both players are at the same position
        if (players[current_player_id].pos[0] == players[other_player].pos[0] &&
            players[current_player_id].pos[1] == players[other_player].pos[1] &&
            players[current_player_id].pos[2] == players[other_player].pos[2]) {
            
            printf("Player %c captures Player %c!\n", 'A' + current_player_id, 'A' + other_player);
            
            players[other_player].in_game = 0;
            players[other_player].captured = 1;
            
            // All captured players go to Player A's starting position
            players[other_player].pos[0] = 0; 
            players[other_player].pos[1] = 6; 
            players[other_player].pos[2] = 12;
            players[other_player].direction = DIR_NORTH; // Same direction as Player A
            
            printf("Player %c sent back to Player A's starting area - must roll 6 to re-enter like Player A\n", 'A' + other_player);
            return; // Only one capture per turn
        }
    }
}

// Validate that a flag cell is on a playable tile (not starting area, wall, blocked, or Bawana)
int is_valid_flag_cell(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w, int l) {
    if (floor < 0 || floor >= NUM_FLOORS || w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) return 0;
    if (!maze[floor][w][l].is_valid) return 0;
    if (maze[floor][w][l].is_starting_area) return 0;
    if (maze[floor][w][l].has_wall) return 0;
    if (maze[floor][w][l].is_blocked_by_stair) return 0;
    if (maze[floor][w][l].is_bawana_entrance) return 0;
    if (floor == 0 && w >= 6 && w <= 9 && l >= 20 && l <= 24) return 0; // Bawana interior
    return 1;
}

// Helper: enqueue for BFS if not visited
static void enqueue_if_valid(int queue[][3], int *q_tail, int visited[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int f, int w, int l) {
    if (f < 0 || f >= NUM_FLOORS || w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) return;
    if (visited[f][w][l]) return;
    queue[*q_tail][0] = f;
    queue[*q_tail][1] = w;
    queue[*q_tail][2] = l;
    (*q_tail)++;
    visited[f][w][l] = 1;
}

// Determine if flag is reachable from any player's entry cell considering walls, stairs, and poles
int is_flag_reachable(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                      Stair stairs[], int num_stairs,
                      Pole poles[], int num_poles,
                      Wall walls[], int num_walls,
                      const int flag_position[3]) {
    // Early reject if flag not on a valid cell per rules
    if (!is_valid_flag_cell(maze, flag_position[0], flag_position[1], flag_position[2])) return 0;

    int visited[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH] = {0};
    int queue[NUM_FLOORS * FLOOR_WIDTH * FLOOR_LENGTH][3];
    int q_head = 0, q_tail = 0;

    // Starting entry cells for A, B, C (after entering maze)
    const int starts[3][3] = {
        {0, 5, 12}, // A
        {0, 9, 7},  // B
        {0, 9, 17}  // C
    };

    // Seed BFS with valid starts
    for (int s = 0; s < 3; s++) {
        int sf = starts[s][0], sw = starts[s][1], sl = starts[s][2];
        if (is_valid_position(maze, sf, sw, sl)) {
            enqueue_if_valid(queue, &q_tail, visited, sf, sw, sl);
        }
    }

    while (q_head < q_tail) {
        int cf = queue[q_head][0];
        int cw = queue[q_head][1];
        int cl = queue[q_head][2];
        q_head++;

        if (cf == flag_position[0] && cw == flag_position[1] && cl == flag_position[2]) return 1;

        // 4-directional neighbors if valid and not blocked by walls
        const int dw[4] = {0, 1, 0, -1};
        const int dl[4] = {-1, 0, 1, 0};
        for (int dir = 0; dir < 4; dir++) {
            int nw = cw + dw[dir];
            int nl = cl + dl[dir];
            if (nw < 0 || nw >= FLOOR_WIDTH || nl < 0 || nl >= FLOOR_LENGTH) continue;
            // Check cell validity first
            if (!is_valid_position(maze, cf, nw, nl)) continue;
            // Check walls between (cw,cl)->(nw,nl)
            if (is_wall_blocking(maze, cf, cw, cl, nw, nl, walls, num_walls)) continue;
            enqueue_if_valid(queue, &q_tail, visited, cf, nw, nl);
        }

        // Stairs edges: from a stair endpoint, traverse to the other endpoint if allowed
        int stair_indices[MAX_STAIRS];
        int stairs_here = find_all_stairs_at(stairs, num_stairs, cf, cw, cl, stair_indices);
        for (int i = 0; i < stairs_here; i++) {
            Stair *st = &stairs[stair_indices[i]];
            int df, dw2, dl2;
            if (cf == st->start_floor && cw == st->start_w && cl == st->start_l) {
                // From start to end allowed if up-only or bidirectional
                if (st->direction_type == STAIR_UP_ONLY || st->direction_type == STAIR_BIDIRECTIONAL) {
                    df = st->end_floor; dw2 = st->end_w; dl2 = st->end_l;
                    if (is_valid_position(maze, df, dw2, dl2)) enqueue_if_valid(queue, &q_tail, visited, df, dw2, dl2);
                }
            }
            if (cf == st->end_floor && cw == st->end_w && cl == st->end_l) {
                // From end to start allowed if down-only or bidirectional
                if (st->direction_type == STAIR_DOWN_ONLY || st->direction_type == STAIR_BIDIRECTIONAL) {
                    df = st->start_floor; dw2 = st->start_w; dl2 = st->start_l;
                    if (is_valid_position(maze, df, dw2, dl2)) enqueue_if_valid(queue, &q_tail, visited, df, dw2, dl2);
                }
            }
        }

        // Pole edge: if this cell has a pole start at this floor/coord, allow sliding to end
        for (int p = 0; p < num_poles; p++) {
            if (poles[p].start_floor == cf && poles[p].w == cw && poles[p].l == cl) {
                int df = poles[p].end_floor;
                int dw2 = poles[p].w;
                int dl2 = poles[p].l;
                if (is_valid_position(maze, df, dw2, dl2)) enqueue_if_valid(queue, &q_tail, visited, df, dw2, dl2);
            }
        }
    }

    return 0; // Not reachable
}

// Periodically update stair directions to add dynamic gameplay
void update_stair_directions(Stair stairs[], int num_stairs) {
    static int total_rounds = 0;
    total_rounds++;
    
    if (total_rounds % 5 == 0) {
        for (int stair_idx = 0; stair_idx < num_stairs; stair_idx++) {
            stairs[stair_idx].direction_type = rand() % 3; // Random between up, down, bidirectional
        }
        printf("Stair directions updated after 5 rounds.\n");
    }
}

// Apply movement bonuses when player lands on bonus cells
void apply_movement_bonus(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id) {
    int current_floor = player->pos[0];
    int current_width = player->pos[1];
    int current_length = player->pos[2];
    
    // Boundary check
    if (current_floor < 0 || current_floor >= NUM_FLOORS || current_width < 0 || current_width >= FLOOR_WIDTH || current_length < 0 || current_length >= FLOOR_LENGTH) {
        return;
    }
    
    int bonus_type = maze[current_floor][current_width][current_length].movement_bonus_type;
    if (bonus_type == BONUS_NONE) return; // No bonus at this cell
    
    char player_letter = 'A' + player_id;
    int old_movement_points = player->movement_points;
    
    switch (bonus_type) {
        case BONUS_ADD_1:
            player->movement_points += 1;
            printf("%c lands on a movement bonus cell and gains 1 movement point! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_ADD_2:
            player->movement_points += 2;
            printf("%c lands on a movement bonus cell and gains 2 movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_ADD_3:
            player->movement_points += 3;
            printf("%c lands on a movement bonus cell and gains 3 movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_ADD_4:
            player->movement_points += 4;
            printf("%c lands on a movement bonus cell and gains 4 movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_ADD_5:
            player->movement_points += 5;
            printf("%c lands on a movement bonus cell and gains 5 movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_MULTIPLY_2:
            player->movement_points *= 2;
            printf("%c lands on a movement bonus cell and doubles movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
        case BONUS_MULTIPLY_3:
            player->movement_points *= 3;
            printf("%c lands on a movement bonus cell and triples movement points! (%d -> %d)\n", 
                   player_letter, old_movement_points, player->movement_points);
            break;
    }
    
    // Clear the bonus after use (one-time bonus per cell)
    maze[current_floor][current_width][current_length].movement_bonus_type = BONUS_NONE;
}

// Transport player to Bawana when movement points are depleted
void reset_to_bawana(Player *player, int player_id) {
    char player_letter = 'A' + player_id;
    printf("%c movement points are depleted and requires replenishment. Transporting to Bawana.\n", player_letter);
    
    // Place player randomly in one of the Bawana interior cells
    int bawana_interior_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };
    
    int random_cell_idx = rand() % 12;
    player->pos[0] = 0;
    player->pos[1] = bawana_interior_cells[random_cell_idx][0];
    player->pos[2] = bawana_interior_cells[random_cell_idx][1];
    player->direction = DIR_NORTH;
    player->in_game = 1;
    
    // Clear any existing Bawana effects before applying new ones
    player->bawana_effect = EFFECT_NONE;
    player->bawana_turns_left = 0;
    player->bawana_random_mp = 0;
    
    // Note: The actual Bawana effects will be applied by the calling function
    // when apply_bawana_effect() is called after this function returns
}

// Get human-readable description of why movement was blocked
const char* get_blockage_reason_description(int blocking_reason) {
    switch(blocking_reason) {
        case BLOCK_WALL: return "wall";
        case BLOCK_INVALID_CELL: return "invalid cell";
        case BLOCK_BAWANA_ENTRANCE: return "Bawana entrance restriction";
        default: return "unknown obstacle";
    }
}

// Initialize the entire 3D maze structure with default values
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    // First pass: set all cells to invalid/empty state
    for (int current_floor = 0; current_floor < NUM_FLOORS; current_floor++) {
        for (int width_idx = 0; width_idx < FLOOR_WIDTH; width_idx++) {
            for (int length_idx = 0; length_idx < FLOOR_LENGTH; length_idx++) {
                // Start with everything disabled/empty
                maze[current_floor][width_idx][length_idx].is_valid = 0;
                maze[current_floor][width_idx][length_idx].is_starting_area = 0;
                maze[current_floor][width_idx][length_idx].has_wall = 0;
                maze[current_floor][width_idx][length_idx].bawana_cell_type = -1; // -1 means not a Bawana cell
                maze[current_floor][width_idx][length_idx].is_blocked_by_stair = 0;
                maze[current_floor][width_idx][length_idx].is_bawana_entrance = 0;
                maze[current_floor][width_idx][length_idx].consumable_value = 0;
                maze[current_floor][width_idx][length_idx].movement_bonus_type = BONUS_NONE;
            }
        }
    }

    // Floor 0 setup - ground level with starting area
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            // Check if this cell is in the starting area rectangle
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0; // Starting area cells aren't playable
            } else {
                maze[0][w][l].is_valid = 1; // Everything else on floor 0 is valid
            }
        }
    }

    // Floor 1 setup - more restrictive layout
    // First section: full width, limited length
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    
    // Middle section: narrow corridor
    for (int w = 3; w <= 6; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }
    
    // End section: full width again
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
        }
    }

    // Floor 2 setup - just the middle strip
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {
            maze[2][w][l].is_valid = 1;
        }
    }

    // Special Bawana area setup - this is where players get special effects
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            maze[0][w][l].is_valid = 1;
            maze[0][w][l].consumable_value = 0; // Bawana doesn't cost movement points
            maze[0][w][l].movement_bonus_type = BONUS_NONE; // No bonuses in Bawana
        }
    }

    // Special entrance cell for Bawana
    maze[0][9][19].is_valid = 1;
    maze[0][9][19].is_bawana_entrance = 1;
    maze[0][9][19].consumable_value = 0;
    maze[0][9][19].movement_bonus_type = BONUS_NONE;

    // Build walls around Bawana area
    // Horizontal wall across the top
    for (int w = 6; w <= 9; w++) {
        maze[0][w][20].has_wall = 1;
    }
    // Vertical wall on the left side
    for (int l = 20; l <= 24; l++) {
        maze[0][6][l].has_wall = 1;
    }

    // Randomly assign special effects to Bawana interior cells
    int bawana_interior_positions[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };

    int available_effects[] = {BA_FOOD_POISONING, BA_DISORIENTED, BA_TRIGGERED, BA_HAPPY};
    int cell_assignment_tracker[12] = {0}; // 0 = unassigned, 1 = assigned

    // Assign 2 cells to each of the 4 effect types
    for (int effect_type = 0; effect_type < 4; effect_type++) {
        for (int count = 0; count < 2; count++) {
            int random_cell_idx;
            // Find an unassigned cell
            do {
                random_cell_idx = rand() % 12;
            } while (cell_assignment_tracker[random_cell_idx]);
            
            int cell_w = bawana_interior_positions[random_cell_idx][0];
            int cell_l = bawana_interior_positions[random_cell_idx][1];
            maze[0][cell_w][cell_l].bawana_cell_type = available_effects[effect_type];
            cell_assignment_tracker[random_cell_idx] = 1;
        }
    }

    // Remaining 4 cells get random MP bonus
    for (int i = 0; i < 12; i++) {
        if (!cell_assignment_tracker[i]) {
            int cell_w = bawana_interior_positions[i][0];
            int cell_l = bawana_interior_positions[i][1];
            maze[0][cell_w][cell_l].bawana_cell_type = BA_RANDOM_MP;
        }
    }

    // Rule 10: Distribute consumable values and movement bonuses across valid cells
    // This creates variety in the maze - some cells cost MP, others give bonuses
    
    // Collect all eligible cells (excluding starting area and Bawana)
    int eligible_cells[1000][3]; // Store [floor, w, l] for each valid cell
    int total_eligible = 0;
    
    for (int floor_num = 0; floor_num < NUM_FLOORS; floor_num++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                // Must be a valid maze cell
                if (!maze[floor_num][w][l].is_valid) continue;
                // Skip starting area
                if (maze[floor_num][w][l].is_starting_area) continue;
                // Skip Bawana area and entrance
                if ((floor_num == 0 && w >= 6 && w <= 9 && l >= 20 && l <= 24) || 
                    maze[floor_num][w][l].is_bawana_entrance) {
                        continue;
                    }
                
                eligible_cells[total_eligible][0] = floor_num;
                eligible_cells[total_eligible][1] = w;
                eligible_cells[total_eligible][2] = l;
                total_eligible++;
            }
        }
    }
    
    
    // Now distribute according to the specified percentages
    int cell_counter = 0;
    
    // 25% get zero consumable value (no cost to move through)
    int zero_value_count = (total_eligible * 25) / 100;
    for (int i = 0; i < zero_value_count && cell_counter < total_eligible; i++) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = 0;
        maze[floor_num][w][l].movement_bonus_type = BONUS_NONE;
        cell_counter++;
    }
    
    // 35% get consumable value 1-4 (costs MP to move through)
    int consumable_count = (total_eligible * 35) / 100;
    for (int i = 0; i < consumable_count && cell_counter < total_eligible; i++) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = (rand() % 4) + 1; // Random 1-4
        maze[floor_num][w][l].movement_bonus_type = BONUS_NONE;
        cell_counter++;
    }
    
    // 25% get small movement bonuses (add 1-2 MP)
    int small_bonus_count = (total_eligible * 25) / 100;
    for (int i = 0; i < small_bonus_count && cell_counter < total_eligible; i++) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = 0;
        maze[floor_num][w][l].movement_bonus_type = (rand() % 2) + 1; // BONUS_ADD_1 or BONUS_ADD_2
        cell_counter++;
    }
    
    // 10% get medium movement bonuses (add 3-5 MP)
    int medium_bonus_count = (total_eligible * 10) / 100;
    for (int i = 0; i < medium_bonus_count && cell_counter < total_eligible; i++) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = 0;
        maze[floor_num][w][l].movement_bonus_type = (rand() % 3) + 3; // BONUS_ADD_3, 4, or 5
        cell_counter++;
    }
    
    // 5% get multiplier bonuses (multiply by 2-3)
    int multiplier_count = (total_eligible * 5) / 100;
    for (int i = 0; i < multiplier_count && cell_counter < total_eligible; i++) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = 0;
        maze[floor_num][w][l].movement_bonus_type = (rand() % 2) + 6; // BONUS_MULTIPLY_2 or 3
        cell_counter++;
    }
    
    // Any remaining cells get zero value (due to rounding in percentages)
    while (cell_counter < total_eligible) {
        int floor_num = eligible_cells[cell_counter][0];
        int w = eligible_cells[cell_counter][1];
        int l = eligible_cells[cell_counter][2];
        maze[floor_num][w][l].consumable_value = 0;
        maze[floor_num][w][l].movement_bonus_type = BONUS_NONE;
        cell_counter++;
    }
}

// Set up initial player positions and states
void initialize_players(Player players[3]) {
    // Player A starts at position [0,6,12] facing North
    players[PLAYER_A].pos[0] = 0; 
    players[PLAYER_A].pos[1] = 6; 
    players[PLAYER_A].pos[2] = 12;
    players[PLAYER_A].in_game = 0; // Starts outside maze
    players[PLAYER_A].direction = DIR_NORTH;
    players[PLAYER_A].movement_points = 100;
    players[PLAYER_A].roll_count = 0;
    players[PLAYER_A].captured = 0;
    players[PLAYER_A].bawana_effect = EFFECT_NONE;
    players[PLAYER_A].bawana_turns_left = 0;
    players[PLAYER_A].bawana_random_mp = 0;
    players[PLAYER_A].just_entered = 0;

    // Player B starts at position [0,9,8] facing West
    players[PLAYER_B].pos[0] = 0; 
    players[PLAYER_B].pos[1] = 9; 
    players[PLAYER_B].pos[2] = 8;
    players[PLAYER_B].in_game = 0;
    players[PLAYER_B].direction = DIR_WEST;
    players[PLAYER_B].movement_points = 100;
    players[PLAYER_B].roll_count = 0;
    players[PLAYER_B].captured = 0;
    players[PLAYER_B].bawana_effect = EFFECT_NONE;
    players[PLAYER_B].bawana_turns_left = 0;
    players[PLAYER_B].bawana_random_mp = 0;
    players[PLAYER_B].just_entered = 0;

    // Player C starts at position [0,9,16] facing East
    players[PLAYER_C].pos[0] = 0; 
    players[PLAYER_C].pos[1] = 9; 
    players[PLAYER_C].pos[2] = 16;
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

// Set up default stair connections between floors
void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 2;
    
    // First stair: connects floor 0 and 1 at position [5,10]
    stairs[0].start_floor = 0; 
    stairs[0].start_w = 5; 
    stairs[0].start_l = 10;
    stairs[0].end_floor = 1; 
    stairs[0].end_w = 5; 
    stairs[0].end_l = 10;
    stairs[0].direction_type = STAIR_BIDIRECTIONAL;

    // Second stair: connects floor 1 and 2 at position [4,12]
    stairs[1].start_floor = 1; 
    stairs[1].start_w = 4; 
    stairs[1].start_l = 12;
    stairs[1].end_floor = 2; 
    stairs[1].end_w = 4; 
    stairs[1].end_l = 12;
    stairs[1].direction_type = STAIR_BIDIRECTIONAL;
}

// Set up default poles for quick descent between floors
void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 1;
    
    // One pole that goes from floor 2 straight down to floor 0
    poles[0].start_floor = 2;
    poles[0].end_floor = 0;
    poles[0].w = 5;
    poles[0].l = 24;
}

// Set up default wall barriers in the maze
void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 3;
    
    // Wall around Bawana area - horizontal section
    walls[0].floor = 0; 
    walls[0].start_w = 6; 
    walls[0].start_l = 20; 
    walls[0].end_w = 9; 
    walls[0].end_l = 20;
    
    // Wall around Bawana area - vertical section
    walls[1].floor = 0; 
    walls[1].start_w = 6; 
    walls[1].start_l = 20; 
    walls[1].end_w = 6; 
    walls[1].end_l = 24;
    
    // Wall on floor 1
    walls[2].floor = 1; 
    walls[2].start_w = 0; 
    walls[2].start_l = 2; 
    walls[2].end_w = 8; 
    walls[2].end_l = 2;
}

// Try to read random seed from file, fallback to current time
int read_seed_from_file(const char *filename) {
    FILE *seed_file = fopen(filename, "r");
    if (!seed_file) {
        printf("Warning: Could not open %s. Using default seed.\n", filename);
        return time(NULL); // Use current time as seed
    }
    
    int seed_value;
    if (fscanf(seed_file, "%d", &seed_value) != 1) {
        printf("Warning: Could not read seed from %s. Using default seed.\n", filename);
        fclose(seed_file);
        return time(NULL);
    }
    
    fclose(seed_file);
    return seed_value;
}

// Load stair configurations from external file
int read_stairs_from_file(const char *filename, Stair stairs[], int *num_stairs) {
    FILE *stair_file = fopen(filename, "r");
    if (!stair_file) return 0;
    
    *num_stairs = 0;
    char line_buffer[256];
    
    while (fgets(line_buffer, sizeof(line_buffer), stair_file) && *num_stairs < MAX_STAIRS) {
        // Expected format: [start_floor, start_w, start_l, end_floor, end_w, end_l]
        if (sscanf(line_buffer, "[%d, %d, %d, %d, %d, %d]",
                   &stairs[*num_stairs].start_floor,
                   &stairs[*num_stairs].start_w,
                   &stairs[*num_stairs].start_l,
                   &stairs[*num_stairs].end_floor,
                   &stairs[*num_stairs].end_w,
                   &stairs[*num_stairs].end_l) == 6) {
            stairs[*num_stairs].direction_type = STAIR_BIDIRECTIONAL; // Default to bidirectional
            (*num_stairs)++;
        }
    }
    
    fclose(stair_file);
    printf("Loaded %d stairs from %s\n", *num_stairs, filename);
    return 1;
}

// Load pole configurations from external file
int read_poles_from_file(const char *filename, Pole poles[], int *num_poles) {
    FILE *pole_file = fopen(filename, "r");
    if (!pole_file) return 0;
    
    *num_poles = 0;
    char line_buffer[256];
    
    while (fgets(line_buffer, sizeof(line_buffer), pole_file) && *num_poles < MAX_POLES) {
        // Expected format: [start_floor, end_floor, w, l]
        if (sscanf(line_buffer, "[%d, %d, %d, %d]",
                   &poles[*num_poles].start_floor,
                   &poles[*num_poles].end_floor,
                   &poles[*num_poles].w,
                   &poles[*num_poles].l) == 4) {
            (*num_poles)++;
        }
    }
    
    fclose(pole_file);
    printf("Loaded %d poles from %s\n", *num_poles, filename);
    return 1;
}

// Load wall configurations from external file
int read_walls_from_file(const char *filename, Wall walls[], int *num_walls) {
    FILE *wall_file = fopen(filename, "r");
    if (!wall_file) return 0;
    
    *num_walls = 0;
    char line_buffer[256];
    
    while (fgets(line_buffer, sizeof(line_buffer), wall_file) && *num_walls < MAX_WALLS) {
        // Expected format: [floor, start_w, start_l, end_w, end_l]
        if (sscanf(line_buffer, "[%d, %d, %d, %d, %d]",
                   &walls[*num_walls].floor,
                   &walls[*num_walls].start_w,
                   &walls[*num_walls].start_l,
                   &walls[*num_walls].end_w,
                   &walls[*num_walls].end_l) == 5) {
            (*num_walls)++;
        }
    }
    
    fclose(wall_file);
    printf("Loaded %d walls from %s\n", *num_walls, filename);
    return 1;
}

// Load flag position from external file
int read_flag_from_file(const char *filename, int flag_position[3]) {
    FILE *flag_file = fopen(filename, "r");
    if (!flag_file) return 0;
    
    char line_buffer[256];
    if (fgets(line_buffer, sizeof(line_buffer), flag_file)) {
        if (sscanf(line_buffer, "[%d, %d, %d]", &flag_position[0], &flag_position[1], &flag_position[2]) == 3) {
            fclose(flag_file);
            printf("Loaded flag position [%d,%d,%d] from %s\n", flag_position[0], flag_position[1], flag_position[2], filename);
            return 1;
        }
    }
    
    fclose(flag_file);
    return 0;
}

// Roll a 6-sided die for movement
int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

// Roll a 6-sided die for direction changes
int roll_direction_dice(void) {
    return (rand() % 6) + 1;
}

// Move player from starting area into the maze
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

    // When captured or reset, all players enter like Player A
void enter_maze_like_player_a(Player *player) {
    player->pos[0] = 0; 
    player->pos[1] = 5; 
    player->pos[2] = 12;
    player->direction = DIR_NORTH; // Same direction as Player A
    player->in_game = 1;
    player->just_entered = 1;
}

// Check if a position is valid for player movement
int is_valid_position(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int width_pos, int length_pos) {
    // Boundary checks first
    if (floor < 0 || floor >= NUM_FLOORS || width_pos < 0 || width_pos >= FLOOR_WIDTH || length_pos < 0 || length_pos >= FLOOR_LENGTH) {
        return 0;
    }
    
    // Check if cell is blocked by stairs
    if (maze[floor][width_pos][length_pos].is_blocked_by_stair) return 0;
    
    return maze[floor][width_pos][length_pos].is_valid;
}

// Special rule: Can only enter Bawana entrance with 0 movement points
int can_enter_bawana_entrance(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int new_w, int new_l) {
    if (maze[0][new_w][new_l].is_bawana_entrance) {
        if (player->movement_points > 0) {
            return 0; // Can't enter with positive MP
        }
    }
    return 1;
}

// Check if there's a wall blocking movement between two adjacent cells
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int from_w, int from_l, int to_w, int to_l, Wall walls[], int num_walls) {
    for (int wall_idx = 0; wall_idx < num_walls; wall_idx++) {
        if (walls[wall_idx].floor != floor) continue;
        
        // Handle vertical walls (same width coordinate)
        if (walls[wall_idx].start_w == walls[wall_idx].end_w) {
            int wall_width = walls[wall_idx].start_w;
            int wall_min_length = (walls[wall_idx].start_l < walls[wall_idx].end_l) ? walls[wall_idx].start_l : walls[wall_idx].end_l;
            int wall_max_length = (walls[wall_idx].start_l > walls[wall_idx].end_l) ? walls[wall_idx].start_l : walls[wall_idx].end_l;
            
            // Check if movement crosses this wall
            if (((from_w == wall_width && to_w == wall_width + 1) || (from_w == wall_width + 1 && to_w == wall_width)) &&
                from_l == to_l && from_l >= wall_min_length && from_l <= wall_max_length) {
                return 1;
            }
        }
        
        // Handle horizontal walls (same length coordinate)
        if (walls[wall_idx].start_l == walls[wall_idx].end_l) {
            int wall_length = walls[wall_idx].start_l;
            int wall_min_width = (walls[wall_idx].start_w < walls[wall_idx].end_w) ? walls[wall_idx].start_w : walls[wall_idx].end_w;
            int wall_max_width = (walls[wall_idx].start_w > walls[wall_idx].end_w) ? walls[wall_idx].start_w : walls[wall_idx].end_w;
            
            // Check if movement crosses this wall
            if (((from_l == wall_length && to_l == wall_length + 1) || (from_l == wall_length + 1 && to_l == wall_length)) &&
                from_w == to_w && from_w >= wall_min_width && from_w <= wall_max_width) {
                return 1;
            }
        }
    }
            return 0;
        }
        
// Find all stairs that exist at a given position
int find_all_stairs_at(Stair stairs[], int num_stairs, int floor, int width_pos, int length_pos, int found_stair_indices[]) {
    int stairs_found = 0;
    for (int stair_idx = 0; stair_idx < num_stairs; stair_idx++) {
        // Check if this stair has either endpoint at the given position
        if ((stairs[stair_idx].start_floor == floor && stairs[stair_idx].start_w == width_pos && stairs[stair_idx].start_l == length_pos) ||
            (stairs[stair_idx].end_floor == floor && stairs[stair_idx].end_w == width_pos && stairs[stair_idx].end_l == length_pos)) {
            found_stair_indices[stairs_found++] = stair_idx;
        }
    }
    return stairs_found;
}