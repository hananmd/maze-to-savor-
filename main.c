// main.c
#include "game.h"
#include <stdio.h>

static void wait_for_enter(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    fgets(buffer, sizeof(buffer), stdin);
}

void play_turn(int player_id, Player players[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs, Pole poles[], int num_poles,
               Wall walls[], int num_walls, int flag[3]) {
    Player *p = &players[player_id];
    printf("\n=== Player %c's Turn ===\n", 'A' + player_id);
    printf("Movement Points: %d\n", p->movement_points);

    // Handle Bawana effects first
    handle_bawana_turns(p);
    
    // Check if player is affected by food poisoning (skip turn)
    if (p->food_poisoning_turns > 0) {
        printf(" Player %c is food poisoned. Skipping turn. (%d turns left)\n", 
               'A' + player_id, p->food_poisoning_turns);
        return;
    }

    wait_for_enter("Press Enter to roll movement die: ");
    int move_roll = roll_movement_dice();
    printf("Movement die: %d\n", move_roll);

    // Rule 2: Enter maze with a 6
    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf(" Player %c enters the maze at Floor %d [%d,%d]\n", 'A'+player_id, p->pos[0] + 1, p->pos[1], p->pos[2]);
        } else {
            printf(" Player %c needs a 6 to enter. Stay in starting area.\n", 'A'+player_id);
            if (p->movement_points > 2) {
                p->movement_points -= 2; // Rule 12: Cost for not moving
                printf(" Lost 2 movement points for blocked movement. Remaining: %d\n", p->movement_points);
            }
            return;
        }
    }

    // Rule 2: Direction dice every 4th roll
    if (p->roll_count % 4 == 0) {
        wait_for_enter("Press Enter to roll direction die: ");
        int dir_roll = roll_direction_dice();
        printf("Direction die: %d\n", dir_roll);
        
        if (dir_roll == 2) { 
            p->direction = DIR_NORTH; 
            printf(" Direction changed to North\n"); 
        } else if (dir_roll == 3) { 
            p->direction = DIR_EAST;  
            printf(" Direction changed to East\n"); 
        } else if (dir_roll == 4) { 
            p->direction = DIR_SOUTH; 
            printf(" Direction changed to South\n"); 
        } else if (dir_roll == 5) { 
            p->direction = DIR_WEST;  
            printf(" Direction changed to West\n"); 
        } else { 
            printf(" Direction unchanged\n"); 
        }
    }

    // Handle special Bawana effects on movement
    if (p->disoriented_turns > 0) {
        printf(" Player is disoriented - movement direction will be random!\n");
    }
    
    int original_move = move_roll;
    if (p->bawana_effect == 3) { // Triggered effect
        printf(" Triggered effect active - movement will be doubled!\n");
    }

    // Move the player
    if (p->in_game) {
        move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, move_roll);
        printf(" Player %c is now at Floor %d [%d,%d]\n", 'A'+player_id, p->pos[0] + 1, p->pos[1], p->pos[2]);

        // Apply cell bonuses and multipliers (Rules 9, 10)
        Cell *cell = &maze[p->pos[0]][p->pos[1]][p->pos[2]];
        if (cell->bonus_value > 0) {
            p->movement_points += cell->bonus_value;
            printf(" Bonus! +%d movement points. Total: %d\n", cell->bonus_value, p->movement_points);
        }
        if (cell->multiplier > 1) {
            int old_mp = p->movement_points;
            p->movement_points *= cell->multiplier;
            printf(" Multiplier x%d applied! %d --> %d movement points\n", 
                   cell->multiplier, old_mp, p->movement_points);
        }

        // Check for player capture (Rule 5)
        check_player_capture(players, player_id);

        // Check for flag capture (Rule 3)
        if (check_flag_capture(p, flag)) {
            printf("\n VICTORY! \n");
            printf(" Player %c has captured the flag!\n", 'A' + player_id);
            printf(" Game Over!\n");
            exit(0);
        }

        // Rule 11: Check if movement points exhausted
        if (p->movement_points <= 0) {
            printf(" Player %c: Movement points exhausted!\n", 'A' + player_id);
            reset_to_bawana(p);
            return;
        }
    }

    p->roll_count++;
}

void print_game_status(Player players[3], int flag[3], int round) {
    printf("\n === Round %d Status ===\n", round);
    
    for (int i = 0; i < 3; i++) {
        Player *p = &players[i];
        printf("Player %c: ", 'A' + i);
        if (!p->in_game) {
            printf("Starting Area");
        } else {
            printf("Floor %d [%d,%d]", p->pos[0] + 1, p->pos[1], p->pos[2]);
        }
        printf(" | MP: %d", p->movement_points);
        if (p->food_poisoning_turns > 0) printf(" |  Sick");
        if (p->disoriented_turns > 0) printf(" |  Confused");
        if (p->bawana_effect == 3) printf(" |  Energized");
        if (p->bawana_effect == 4) printf(" |  Happy");
        printf("\n");
    }
    printf("==============================\n");
}

int main(void) {
    srand((unsigned int)time(NULL));

    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;
    int flag[3];

    printf(" === Maze of UCSC === \n");
    printf("Initializing game...\n");

    initialize_maze(maze);
    initialize_players(players);
    initialize_stairs(stairs, &num_stairs);
    initialize_poles(poles, &num_poles);
    initialize_walls(walls, &num_walls);
    place_random_flag(flag, maze, walls, num_walls);

   
    printf(" Flag is placed at [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    printf(" 3 Players start with 100 movement points each\n");
   
    printf(" First player to reach the flag wins!\n\n");

    

    wait_for_enter("Press Enter to start the game...");

    int round = 1;
    while (1) {
        print_game_status(players, flag, round);
        
        // Rule 6: Update stair directions every 5 rounds
        if (round % 5 == 0) {
            update_stair_directions(stairs, num_stairs);
        }

        // Play each player's turn
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, flag);
            
            // Small delay for readability
            printf("---\n");
        }
        
        round++;
        
        // Safety check - prevent infinite games
        if (round > 1000) {
            printf(" Game limit reached (1000 rounds). Ending game.\n");
            break;
        }
    }

    return 0;
}