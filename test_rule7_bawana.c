#include "game.h"
#include <stdio.h>

static int in_bawana(int f, int w, int l) {
    return (f == 0 && w >= 6 && w <= 9 && l >= 20 && l <= 24);
}

int main(void) {
    srand(123456);

    Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH];
    Player players[3];
    Stair stairs[MAX_STAIRS];
    Pole poles[MAX_POLES];
    Wall walls[MAX_WALLS];
    int num_stairs, num_poles, num_walls;

    initialize_maze(maze);
    initialize_players(players);
    initialize_stairs(stairs, &num_stairs);
    initialize_poles(poles, &num_poles);
    initialize_walls(walls, &num_walls);

    int ok = 1;

    // Geometry: All Bawana cells valid; entrance at [0,9,19]
    if (!maze[0][9][19].is_bawana_entrance) { printf("✗ Entrance not set at [0,9,19]\n"); ok = 0; }
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            if (!maze[0][w][l].is_valid) { printf("✗ Bawana interior invalid at [0,%d,%d]\n", w, l); ok = 0; }
        }
    }

    // Walls: single-width from [0,6,20..24] vertical and [0,6..9,20] horizontal
    for (int l = 20; l <= 24; l++) {
        if (!maze[0][6][l].has_wall) { printf("✗ Missing wall at [0,6,%d]\n", l); ok = 0; }
    }
    for (int w = 6; w <= 9; w++) {
        if (!maze[0][w][20].has_wall) { printf("✗ Missing wall at [0,%d,20]\n", w); ok = 0; }
    }

    // Effect distribution: exactly 12 cells; two of each BA_* 0..3; remaining BA_RANDOM_MP
    int counts[5] = {0};
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            counts[ maze[0][w][l].bawana_cell_type ]++;
        }
    }
    if (counts[BA_FOOD_POISONING] != 2) { printf("✗ Food Poisoning count = %d (expected 2)\n", counts[BA_FOOD_POISONING]); ok = 0; }
    if (counts[BA_DISORIENTED] != 2) { printf("✗ Disoriented count = %d (expected 2)\n", counts[BA_DISORIENTED]); ok = 0; }
    if (counts[BA_TRIGGERED] != 2) { printf("✗ Triggered count = %d (expected 2)\n", counts[BA_TRIGGERED]); ok = 0; }
    if (counts[BA_HAPPY] != 2) { printf("✗ Happy count = %d (expected 2)\n", counts[BA_HAPPY]); ok = 0; }
    int random_expected = 12 - 2 - 2 - 2 - 2; // remaining 4
    if (counts[BA_RANDOM_MP] != random_expected) { printf("✗ Random MP count = %d (expected %d)\n", counts[BA_RANDOM_MP], random_expected); ok = 0; }

    // One-way entrance: cannot walk into Bawana via movement if MP > 0
    Player *p = &players[PLAYER_A];
    p->in_game = 1; p->movement_points = 10; p->direction = DIR_SOUTH; // try walking from [0,9,18] -> [0,9,19]
    p->pos[0] = 0; p->pos[1] = 9; p->pos[2] = 18;
    int movement_cost=0, actual_steps=0, blocking_reason=0; int flag[3] = {1,0,0};
    int moved = move_player_with_teleport(p, maze, stairs, num_stairs, poles, num_poles, walls, num_walls, 1, PLAYER_A, flag, &movement_cost, &actual_steps, &blocking_reason);
    if (p->pos[0] == 0 && p->pos[1] == 9 && p->pos[2] == 19) { printf("✗ Entered Bawana entrance by movement while MP>0\n"); ok = 0; }

    // Effects application checks – land inside Bawana -> placed at entrance with correct state
    // Place on each type and call apply_bawana_effect
    int types_to_test[5] = {BA_FOOD_POISONING, BA_DISORIENTED, BA_TRIGGERED, BA_HAPPY, BA_RANDOM_MP};
    for (int i = 0; i < 5; i++) {
        p->bawana_effect = EFFECT_NONE; p->bawana_turns_left = 0; p->bawana_random_mp = 0; p->movement_points = 100;
        p->pos[0] = 0; p->pos[1] = 6; p->pos[2] = 20; // inside Bawana
        // Temporarily set the cell type to the one we want to test
        maze[0][6][20].bawana_cell_type = types_to_test[i];
        apply_bawana_effect(p, maze, PLAYER_A);
        // After effect, player should be at entrance [0,9,19] with North direction (except food poisoning - stays inside but misses turns)
        if (types_to_test[i] == BA_FOOD_POISONING) {
            if (p->bawana_effect != EFFECT_FOOD_POISONING || p->bawana_turns_left != 3) { printf("✗ Food Poisoning effect state invalid\n"); ok = 0; }
        } else if (types_to_test[i] == BA_DISORIENTED) {
            if (!(p->pos[0] == 0 && p->pos[1] == 9 && p->pos[2] == 19 && p->direction == DIR_NORTH && p->movement_points >= 150 && p->bawana_effect == EFFECT_DISORIENTED && p->bawana_turns_left == 4)) { printf("✗ Disoriented placement/state invalid\n"); ok = 0; }
        } else if (types_to_test[i] == BA_TRIGGERED) {
            if (!(p->pos[0] == 0 && p->pos[1] == 9 && p->pos[2] == 19 && p->direction == DIR_NORTH && p->movement_points >= 150 && p->bawana_effect == EFFECT_TRIGGERED && p->bawana_turns_left == 4)) { printf("✗ Triggered placement/state invalid\n"); ok = 0; }
        } else if (types_to_test[i] == BA_HAPPY) {
            if (!(p->pos[0] == 0 && p->pos[1] == 9 && p->pos[2] == 19 && p->direction == DIR_NORTH && p->movement_points >= 300 && p->bawana_effect == EFFECT_HAPPY)) { printf("✗ Happy placement/state invalid\n"); ok = 0; }
        } else if (types_to_test[i] == BA_RANDOM_MP) {
            if (!(p->pos[0] == 0 && p->pos[1] == 9 && p->pos[2] == 19 && p->direction == DIR_NORTH && p->bawana_effect == EFFECT_RANDOM_MP && p->bawana_turns_left == 4 && p->bawana_random_mp >= 10 && p->bawana_random_mp <= 100)) { printf("✗ Random MP placement/state invalid\n"); ok = 0; }
        }
    }

    if (ok) {
        printf("✓ Rule 7 tests passed. Bawana geometry, entrance behavior, and effects verified.\n");
    }
    return ok ? 0 : 1;
}
