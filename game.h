// game.h
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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

// Stair directions
#define STAIR_UP_ONLY       0
#define STAIR_DOWN_ONLY     1
#define STAIR_BIDIRECTIONAL 2

// Bawana Cell Types (for Cell.bawana_cell_type initialization)
#define BA_FOOD_POISONING   0
#define BA_DISORIENTED      1
#define BA_TRIGGERED        2
#define BA_HAPPY            3
#define BA_RANDOM_MP        4

// Bawana EFFECT STATES (for Player.bawana_effect runtime state)
#define EFFECT_NONE             0
#define EFFECT_FOOD_POISONING   1
#define EFFECT_DISORIENTED      2
#define EFFECT_TRIGGERED        3
#define EFFECT_HAPPY            4
#define EFFECT_RANDOM_MP        5

// Max counts
#define MAX_STAIRS  10
#define MAX_POLES   10
#define MAX_WALLS   20
#define MAX_LOOP_HISTORY 100

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
    int is_blocked_by_stair; // For intermediate floors
    int is_bawana_entrance;   // 1 if this is the Bawana entrance cell
} Cell;

typedef struct {
    int pos[3];             // [floor, width, length]
    int in_game;            // 0 = in starting area, 1 = in maze
    int direction;          // current direction
    int movement_points;    // starts at 100
    int roll_count;         // for direction dice every 4th roll
    int captured;           // 1 if captured
    int capture_start_pos[3]; // starting position when captured
    int bawana_effect;      // 0 = none, 1-5 = effect types
    int bawana_turns_left;  // countdown for effects
    int bawana_random_mp;   // random MP value if applicable
    int just_entered;       // flag to indicate player just entered maze this turn
} Player;

// Function declarations
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void initialize_players(Player players[3]);
void initialize_stairs(Stair stairs[], int *num_stairs);
void initialize_poles(Pole poles[], int *num_poles);
void initialize_walls(Wall walls[], int *num_walls);
int read_stairs_from_file(const char *filename, Stair stairs[], int *num_stairs);
int read_poles_from_file(const char *filename, Pole poles[], int *num_poles);
int read_walls_from_file(const char *filename, Wall walls[], int *num_walls);
int read_flag_from_file(const char *filename, int flag[3]);
int read_seed_from_file(const char *filename);
int roll_movement_dice(void);
int roll_direction_dice(void);
void enter_maze(Player *player, int player_id);
int move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps, int player_id, const int flag[3]);
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2, Wall walls[], int num_walls);
int find_all_stairs_at(Stair stairs[], int num_stairs, int floor, int w, int l, int result_indices[]);
int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l);
void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
int check_flag_capture(Player *player, const int flag[3]);
void check_player_capture(Player players[3], int current_player);
void update_stair_directions(Stair stairs[], int num_stairs);
void reset_to_bawana(Player *player);
void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id);
int is_in_starting_area(int floor, int w, int l);
void reset_to_starting_area(Player *player, int player_id);
int manhattan_distance(int f1, int w1, int l1, int f2, int w2, int l2);
const char* get_direction_name(int direction);
const char* format_position(int floor, int w, int l);
int can_enter_bawana_entrance(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int new_w, int new_l);
int check_path_validity(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls, int steps, int player_id);

#endif