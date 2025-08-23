// game.h
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_FLOORS      3
#define FLOOR_WIDTH     10
#define FLOOR_LENGTH    25

// Starting area
#define START_AREA_W_MIN  6
#define START_AREA_W_MAX  9
#define START_AREA_L_MIN  8
#define START_AREA_L_MAX  16

// Player IDs
#define PLAYER_A 0
#define PLAYER_B 1
#define PLAYER_C 2

// Directions
#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3

// Max counts
#define MAX_STAIRS  10
#define MAX_POLES   10
#define MAX_WALLS   20

// Structures
typedef struct {
    int start_floor, start_w, start_l;
    int end_floor, end_w, end_l;
} Stair;

typedef struct {
    int start_floor;
    int end_floor;
    int w, l;
} Pole;

typedef struct {
    int floor;
    int start_w, start_l;
    int end_w, end_l;
} Wall;

typedef struct {
    int is_valid;
    int is_starting_area;
    int has_wall;
    int consumable_value;
    int bonus_value;
    int multiplier;
} Cell;

typedef struct {
    int pos[3];             // [floor, width, length]
    int movement_points;
    int direction;
    int in_game;
    int captured;           // 1 if captured by another player
} Player;

// Function declarations
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void initialize_players(Player players[3]);
void initialize_stairs(Stair stairs[], int *num_stairs);
void initialize_poles(Pole poles[], int *num_poles);
void initialize_walls(Wall walls[], int *num_walls);
void initialize_consumables_and_bonuses(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls);
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2);
void apply_stair_teleport(Player *player, Stair stairs[], int num_stairs);
void apply_pole_teleport(Player *player, Pole poles[], int num_poles);
int check_flag_capture(Player *player, int flag[3]);
int is_game_over(Player players[3], int flag[3]);
void print_player_positions(Player players[3]);

#endif // GAME_H