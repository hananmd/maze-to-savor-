// main.c
#include "game.h"

// Simulate dice roll (1-6)
int roll_dice() {
    return (rand() % 6) + 1;
}

// Move player in current direction for 'steps' cells
void move_player(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                 Stair stairs[], int num_stairs,
                 Pole poles[], int num_poles,
                 Wall walls[], int num_walls) {
    int steps = roll_dice();
    int moved = 0;

    for (int i = 0; i < steps; i++) {
        int f = player->pos[0];
        int w = player->pos[1];
        int l = player->pos[2];
        int nf = f, nw = w, nl = l;

        // Compute next cell based on direction
        switch (player->direction) {
            case DIR_NORTH: nl--; break;
            case DIR_EAST:  nw++; break;
            case DIR_SOUTH: nl++; break;
            case DIR_WEST:  nw--; break;
        }

        // Check bounds
        if (nw < 0 || nw >= FLOOR_WIDTH || nl < 0 || nl >= FLOOR_LENGTH) {
            printf("Blocked by edge at [%d,%d,%d] -> [%d,%d,%d]\n", f, w, l, nf, nw, nl);
            break; // Stop movement
        }

        // Check wall blocking
        if (is_wall_blocking(maze, f, w, l, nw, nl)) {
            printf("Blocked by wall at [%d,%d,%d] -> [%d,%d,%d]\n", f, w, l, nf, nw, nl);
            break; // Stop movement
        }

        // Move to next cell
        player->pos[1] = nw;
        player->pos[2] = nl;
        moved++;

        // Check stair teleport after landing
        apply_stair_teleport(player, stairs, num_stairs);

        // Check pole teleport after landing
        apply_pole_teleport(player, poles, num_poles);

        // Recalculate current floor after teleport
        f = player->pos[0];
        w = player->pos[1];
        l = player->pos[2];
    }

    if (moved < steps) {
        player->movement_points -= 2; // Rule 12: blocked move costs 2 MP
    }
}

// Simulate one turn for a player
void play_turn(int player_id, Player players[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
               Stair stairs[], int num_stairs,
               Pole poles[], int num_poles,
               Wall walls[], int num_walls,
               int flag[3]) {
    Player *p = &players[player_id];
    if (p->captured) return;

    printf("\n--- Player %c's Turn ---\n", 'A' + player_id);

    int dice = roll_dice();
    printf("Rolled movement dice: %d\n", dice);

    if (!p->in_game) {
        if (dice == 6) {
            // Enter maze
            p->in_game = 1;
            switch (player_id) {
                case PLAYER_A: p->pos[1] = 5; p->pos[2] = 12; break;
                case PLAYER_B: p->pos[1] = 9; p->pos[2] = 7;  break;
                case PLAYER_C: p->pos[1] = 9; p->pos[2] = 17; break;
            }
            printf("Player %c entered the maze at [%d,%d,%d]\n", 'A'+player_id, p->pos[0], p->pos[1], p->pos[2]);
        } else {
            printf("Player %c needs a 6 to enter. Rolled %d.\n", 'A'+player_id, dice);
        }
        return;
    }

    // Move player
    move_player(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls);
    printf("Player %c ended at [%d,%d,%d]\n", 'A'+player_id, p->pos[0], p->pos[1], p->pos[2]);

    // Check flag capture
    if (check_flag_capture(p, flag)) {
        printf("🎉 Player %c has captured the flag!\n", 'A' + player_id);
    }
}

int main() {
    srand(time(NULL));

    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;
    int flag[3];

    // Initialize game
    initialize_maze(maze);
    initialize_players(players);
    initialize_stairs(stairs, &num_stairs);
    initialize_poles(poles, &num_poles);
    initialize_walls(walls, &num_walls);
    initialize_consumables_and_bonuses(maze);
    place_random_flag(flag, maze, walls, num_walls);

    printf("=== Maze of UCSC ===\n");
    printf("Flag is at: [%d,%d,%d]\n", flag[0], flag[1], flag[2]);

    // Game loop
    while (1) {
        for (int i = 0; i < 3; i++) {
            play_turn(i, players, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, flag);
            if (check_flag_capture(&players[i], flag)) {
                printf("🏆 Game Over! Player %c wins!\n", 'A' + i);
                return 0;
            }
        }
    }

    return 0;
}