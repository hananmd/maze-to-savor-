// main.c
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
    
    // Fixed food poisoning logic
    if (p->bawana_effect == EFFECT_FOOD_POISONING) {
        p->bawana_turns_left--;
        printf("%c is still food poisoned and misses the turn.\n", player_name);
        
        if (p->bawana_turns_left == 0) {
            p->bawana_effect = EFFECT_NONE;
            
            // Check if player has zero movement points after food poisoning
            if (p->movement_points <= 0) {
                printf("%c is now fit to proceed from the food poisoning episode and now placed on a random cell and the effects take place.\n", player_name);
                int bawana_cells[12][2] = {
                    {6,21}, {6,22}, {6,23}, {6,24},
                    {7,21}, {7,22}, {7,23}, {7,24},
                    {8,21}, {8,22}, {8,23}, {8,24}
                };
                int idx = rand() % 12;
                p->pos[0] = 0;
                p->pos[1] = bawana_cells[idx][0];
                p->pos[2] = bawana_cells[idx][1];
                apply_bawana_effect(p, maze, player_id);
            } else {
                printf("%c has recovered from food poisoning and can resume normal play.\n", player_name);
            }
        }
        return;
    }
    
    wait_for_enter("Press Enter to roll movement die: ");
    int move_roll = roll_movement_dice();
    printf("Movement die: %d\n", move_roll);
    
    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf("%c is at the starting area and rolls 6 on the movement dice and is placed on [%d,%d,%d] of the maze.\n", 
                   player_name, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("%c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", 
                   player_name, move_roll);
            // No movement point deduction for failing to enter maze from starting area
            printf("%c moved 0 cells and remains in the starting area.\n", player_name);
            if (p->movement_points <= 0) {
                reset_to_bawana(p);
            }
            return;
        }
    }
    
    const char* directions[] = {"North", "East", "South", "West"};
    int original_move = move_roll;

    if (p->roll_count % 4 == 0) {
        int dir_roll = roll_direction_dice();
        if (dir_roll == 2) { 
            p->direction = DIR_NORTH; 
            printf("%c rolls %d on the movement dice and North on the direction dice, changes direction to North", 
                   player_name, move_roll);
        } else if (dir_roll == 3) { 
            p->direction = DIR_EAST;  
            printf("%c rolls %d on the movement dice and East on the direction dice, changes direction to East", 
                   player_name, move_roll);
        } else if (dir_roll == 4) { 
            p->direction = DIR_SOUTH; 
            printf("%c rolls %d on the movement dice and South on the direction dice, changes direction to South", 
                   player_name, move_roll);
        } else if (dir_roll == 5) { 
            p->direction = DIR_WEST;  
            printf("%c rolls %d on the movement dice and West on the direction dice, changes direction to West", 
                   player_name, move_roll);
        } else { 
            printf("%c rolls %d on the movement dice and Empty on the direction dice, direction unchanged (%s)", 
                   player_name, move_roll, directions[p->direction]);
        }
    } else {
        printf("%c rolls %d on the movement dice and moves %s", 
               player_name, move_roll, directions[p->direction]);
    }

    if (p->bawana_effect == EFFECT_DISORIENTED) {
        p->direction = rand() % 4;
        printf(" and is disoriented and move in the %s", directions[p->direction]);
    } else if (p->bawana_effect == EFFECT_TRIGGERED) {
        move_roll *= 2;
        printf(" and is triggered and move in the %s", directions[p->direction]);
    }

    int old_pos[3] = {p->pos[0], p->pos[1], p->pos[2]};
    
    // Fixed movement function call and movement point logic
    int was_blocked_by_wall = move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, move_roll, player_id, flag);

    if (old_pos[0] == p->pos[0] && old_pos[1] == p->pos[1] && old_pos[2] == p->pos[2]) {
        printf("%c rolls %d on the movement dice and cannot move in the %s. Player remains at [%d,%d,%d]\n", 
               player_name, original_move, directions[p->direction], p->pos[0], p->pos[1], p->pos[2]);
        
        // Only deduct movement points if blocked by wall in maze (not starting area)
        if (was_blocked_by_wall && p->in_game) {
            p->movement_points -= 2;
            printf("%c moved 0 cells that cost 2 movement points and is left with %d and is moving in the %s.\n", 
                   player_name, p->movement_points, directions[p->direction]);
        } else {
            printf("%c moved 0 cells and is left with %d and is moving in the %s.\n", 
                   player_name, p->movement_points, directions[p->direction]);
        }
    } else {
        int cells_moved = abs(old_pos[1] - p->pos[1]) + abs(old_pos[2] - p->pos[2]);
        printf(" by %d cells and is now at [%d,%d,%d].\n", move_roll, p->pos[0], p->pos[1], p->pos[2]);
        printf("%c moved %d cells that cost 0 movement points and is left with %d and is moving in the %s.\n", 
               player_name, cells_moved, p->movement_points, directions[p->direction]);
    }

    // Handle other Bawana effects countdown
    if (p->bawana_effect > EFFECT_NONE && p->bawana_effect != EFFECT_FOOD_POISONING) {
        p->bawana_turns_left--;
        if (p->bawana_turns_left == 0) {
            if (p->bawana_effect == EFFECT_DISORIENTED) {
                printf("%c has recovered from disorientation.\n", player_name);
            } else if (p->bawana_effect == EFFECT_TRIGGERED) {
                printf("%c has recovered from being triggered.\n", player_name);
            } else if (p->bawana_effect == EFFECT_HAPPY) {
                printf("%c is no longer extra happy.\n", player_name);
            } else if (p->bawana_effect == EFFECT_RANDOM_MP) {
                printf("%c's random movement point effect has expired.\n", player_name);
            }
            p->bawana_effect = EFFECT_NONE;
        }
    }

    if (p->movement_points <= 0) {
        reset_to_bawana(p);
    }

    check_player_capture(players, player_id);

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
                printf(", Bawana effect: %s (turns left: %d)", effect_names[players[i].bawana_effect], players[i].bawana_turns_left);
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
        int start = stairs[i].start_floor;
        int end = stairs[i].end_floor;
        int w = (start == stairs[i].start_floor) ? stairs[i].start_w : stairs[i].end_w;
        int l = (start == stairs[i].start_floor) ? stairs[i].start_l : stairs[i].end_l;
        
        int min_f = (start < end) ? start : end;
        int max_f = (start > end) ? start : end;
        
        if (max_f - min_f > 1) {
            for (int f = min_f + 1; f < max_f; f++) {
                maze[f][w][l].is_blocked_by_stair = 1;
                printf("Blocked cell [%d,%d,%d] for skipping stair.\n", f, w, l);
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