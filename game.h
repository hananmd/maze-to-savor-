// game.h
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_FLOORS      3
#define FLOOR_WIDTH     10
#define FLOOR_LENGTH    25

// Starting area on Floor 0
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

// Stair directions (Rule 6)
#define STAIR_UP_ONLY       0
#define STAIR_DOWN_ONLY     1
#define STAIR_BIDIRECTIONAL 2

// Bawana Cell Types (Rule 7)
#define BA_FOOD_POISONING   0
#define BA_DISORIENTED      1
#define BA_TRIGGERED        2
#define BA_HAPPY            3
#define BA_RANDOM_MP        4

// Max counts
#define MAX_STAIRS  10
#define MAX_POLES   10
#define MAX_WALLS   20

// Structures
typedef struct {
    int start_floor, start_w, start_l;
    int end_floor, end_w, end_l;
    int direction_type; // 0=up only, 1=down only, 2=bidirectional
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
    int bawana_cell_type; // -1 = not Bawana, 0-4 = effect type
    int consumable_value; // 0 to 4 (Rule 8)
    int bonus_value;      // 0, 1-2 (25%), 3-5 (10%)
    int multiplier;       // 1 (no effect), 2 or 3 (5%)
} Cell;

typedef struct {
    int pos[3];             // [floor, width, length]
    int in_game;            // 0 = in starting area, 1 = in maze
    int direction;          // current direction
    int movement_points;    // starts at 100
    int roll_count;         // for direction dice every 4th roll
    int captured;           // 1 if captured
    int capture_start_pos[3];
    int bawana_effect;      // 0 = none, 1 = food poisoning, etc.
    int bawana_turns_left;
    int bawana_random_mp;
} Player;

// Function declarations
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void initialize_players(Player players[3]);
void initialize_stairs(Stair stairs[], int *num_stairs);
void initialize_poles(Pole poles[], int *num_poles);
void initialize_walls(Wall walls[], int *num_walls);
int roll_movement_dice(void);
int roll_direction_dice(void);
void enter_maze(Player *player, int player_id);
void move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps);
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2);
int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l);
int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l);
void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls);
int check_flag_capture(Player *player, const int flag[3]);
void check_player_capture(Player players[3], int current_player);
void update_stair_directions(Stair stairs[], int num_stairs);
void reset_to_bawana(Player *player);

#endif