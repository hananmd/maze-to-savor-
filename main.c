#include "game.h"

/* Wait for user to press Enter, then roll dice automatically */
static void wait_for_enter(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    fgets(buffer, sizeof buffer, stdin);
}

void play_turn(int player_id,
               Player players[3],
               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs,
               Pole poles[], int num_poles,
               Wall walls[], int num_walls,
               int flag[3]) {
    
    Player *p = &players[player_id];
    printf("\n=== Player %c's Turn ===\n", 'A' + player_id);
    
    // Wait for player to press Enter, then roll movement die automatically
    wait_for_enter("Press Enter to roll dice: ");
    int move_roll = roll_movement_dice();
    printf("Movement die: %d\n", move_roll);
    
    /* Entry logic: needs a 6 to enter */
    if (!p->in_game) {
        if (move_roll == 6) {
            enter_maze(p, player_id);
            p->roll_count = 1;
            printf("Player %c enters the maze at [%d,%d,%d]\n", 
                   'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter. Stay in starting area.\n", 'A' + player_id);
            p->movement_points -= 2;
            return;
        }
    } else {
        p->roll_count++;
    }
    
    /* Direction DIE: only every 4th roll */
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
    move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, 
                             walls, num_walls, move_roll);
    
    printf("Player %c at [%d,%d,%d]\n", 
           'A' + player_id, p->pos[0], p->pos[1], p->pos[2]);
    
    // Check for player collision after movement
    for (int i = 0; i < 3; i++) {
        if (i != player_id && players[i].in_game) {
            if (p->pos[0] == players[i].pos[0] && 
                p->pos[1] == players[i].pos[1] && 
                p->pos[2] == players[i].pos[2]) {
                
                printf("Player %c captures Player %c!\n", 'A' + player_id, 'A' + i);
                
                // Send captured player back to starting area
                Player *captured = &players[i];
                switch (i) {
                    case PLAYER_A:
                        captured->pos[0] = 0; captured->pos[1] = 6; captured->pos[2] = 12;
                        break;
                    case PLAYER_B:
                        captured->pos[0] = 0; captured->pos[1] = 9; captured->pos[2] = 8;
                        break;
                    case PLAYER_C:
                        captured->pos[0] = 0; captured->pos[1] = 9; captured->pos[2] = 16;
                        break;
                }
                captured->in_game = 0;
                captured->entered_maze = 0;
                captured->roll_count = 0;
                printf("Player %c sent back to starting area\n", 'A' + i);
                break;
            }
        }
    }
    
    // Check for flag capture
    if (check_flag_capture(p, flag)) {
        printf("🏁 Player %c captured the flag!\n", 'A' + player_id);
        printf("🎉 GAME OVER! Player %c wins!\n", 'A' + player_id);
        exit(0);
    }
}

int main(void) {
    srand((unsigned int)time(NULL)); // Seed for random flag placement
    
    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;
    int flag[3];
    
    initialize_maze(maze);
    initialize_players(players);
    initialize_stairs(stairs, &num_stairs);
    initialize_poles(poles, &num_poles);
    initialize_walls(walls, &num_walls);
    place_random_flag(flag, maze, walls, num_walls);
    
    printf("=== MAZE GAME ===\n");
    printf("Flag is at [%d,%d,%d]\n", flag[0], flag[1], flag[2]);
    printf("Press Enter to roll dice for each player.\n");
    printf("=========================================\n");
    
    while (1) {
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, 
                     poles, num_poles, walls, num_walls, flag);
        }
    }
    
    return 0;
}