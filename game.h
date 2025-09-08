// game.h - Header file for Maze of UCSC game
// Contains all the data structures, constants, and function prototypes
// Author: [Your Name]

#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Basic maze dimensions - these define the 3D structure
#define NUM_FLOORS      3
#define FLOOR_WIDTH     10
#define FLOOR_LENGTH    25

// Starting area boundaries on Floor 0 (where players begin)
#define START_AREA_W_MIN  6
#define START_AREA_W_MAX  9
#define START_AREA_L_MIN  8
#define START_AREA_L_MAX  16

// Player identifiers - makes code more readable than using 0,1,2
#define PLAYER_A 0
#define PLAYER_B 1
#define PLAYER_C 2

// Movement directions (North decreases length, East increases width)
#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3

// Stair movement restrictions
#define STAIR_UP_ONLY       0  // Can only go from start to end floor
#define STAIR_DOWN_ONLY     1  // Can only go from end to start floor  
#define STAIR_BIDIRECTIONAL 2  // Can go either direction

// Bawana cell effect types (what's assigned during maze initialization)
#define BA_FOOD_POISONING   0
#define BA_DISORIENTED      1
#define BA_TRIGGERED        2
#define BA_HAPPY            3
#define BA_RANDOM_MP        4

// Player status effects (what players experience during gameplay)
#define EFFECT_NONE             0
#define EFFECT_FOOD_POISONING   1
#define EFFECT_DISORIENTED      2
#define EFFECT_TRIGGERED        3
#define EFFECT_HAPPY            4  // Actually not used as lasting effect
#define EFFECT_RANDOM_MP        5

// Movement bonus types for special cells
#define BONUS_NONE              0
#define BONUS_ADD_1            1  // Add 1 movement point
#define BONUS_ADD_2            2  // Add 2 movement points
#define BONUS_ADD_3            3  // Add 3 movement points
#define BONUS_ADD_4            4  // Add 4 movement points
#define BONUS_ADD_5            5  // Add 5 movement points
#define BONUS_MULTIPLY_2       6  // Double movement points
#define BONUS_MULTIPLY_3       7  // Triple movement points

// Reasons why player movement might be blocked
#define BLOCK_NONE              0
#define BLOCK_WALL              1
#define BLOCK_INVALID_CELL      2
#define BLOCK_BAWANA_ENTRANCE   3  // Can only enter Bawana with 0 MP

// Maximum limits for game elements
#define MAX_STAIRS  10
#define MAX_POLES   10
#define MAX_WALLS   20
#define MAX_LOOP_HISTORY 100  // For infinite loop detection

// Data structure for stairs connecting different floors
typedef struct {
    int start_floor, start_w, start_l;  // Starting position
    int end_floor, end_w, end_l;        // Ending position
    int direction_type;                 // Movement restriction type
} Stair;

// Data structure for poles (slide down from higher to lower floor)
typedef struct {
    int start_floor;  // Where pole starts (higher floor)
    int end_floor;    // Where pole ends (lower floor)
    int w, l;         // Position coordinates (same on both floors)
} Pole;

// Data structure for walls that block movement between adjacent cells
typedef struct {
    int floor;                    // Which floor this wall is on
    int start_w, start_l;        // One end of the wall
    int end_w, end_l;            // Other end of the wall
} Wall;

// Data structure for individual maze cells
typedef struct {
    int is_valid;                // Can players move through this cell?
    int is_starting_area;        // Is this part of the starting area?
    int has_wall;               // Does this cell have a wall structure?
    int bawana_cell_type;       // What Bawana effect does this cell have? (-1 = not Bawana)
    int is_blocked_by_stair;    // Is movement blocked due to stair skipping?
    int is_bawana_entrance;     // Special entrance cell for Bawana area
    int consumable_value;       // How much MP does it cost to move through? (0-4)
    int movement_bonus_type;    // What movement bonus does this cell provide?
} Cell;

// Data structure for player state and position
typedef struct {
    int pos[3];                 // Current position [floor, width, length]
    int in_game;                // 0 = in starting area, 1 = in maze
    int direction;              // Current facing direction
    int movement_points;        // Current MP (starts at 100)
    int roll_count;             // Number of dice rolls (for direction dice timing)
    int captured;               // Has this player been captured?
    int capture_start_pos[3];   // Where player was when captured (not currently used)
    int bawana_effect;          // Current Bawana status effect
    int bawana_turns_left;      // How many turns until effect wears off
    int bawana_random_mp;       // Random MP value for random MP effect
    int just_entered;           // Flag for players who just entered maze
} Player;

// Function prototypes - organized by category

// Initialization functions
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void initialize_players(Player players[3]);
void initialize_stairs(Stair stairs[], int *num_stairs);
void initialize_poles(Pole poles[], int *num_poles);
void initialize_walls(Wall walls[], int *num_walls);

// File I/O functions for loading game configurations
int read_stairs_from_file(const char *filename, Stair stairs[], int *num_stairs);
int read_poles_from_file(const char *filename, Pole poles[], int *num_poles);
int read_walls_from_file(const char *filename, Wall walls[], int *num_walls);
int read_flag_from_file(const char *filename, int flag_position[3]);
int read_seed_from_file(const char *filename);

// Dice and random functions
int roll_movement_dice(void);
int roll_direction_dice(void);

// Player movement and entry functions
void enter_maze(Player *player, int player_id);
void enter_maze_like_player_a(Player *player);
int move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps, int player_id, const int flag_position[3],
                               int *total_movement_cost, int *actual_steps_taken, int *blocking_reason);

// Movement validation and obstacle detection
int is_valid_position(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int width_pos, int length_pos);
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int from_w, int from_l, int to_w, int to_l, Wall walls[], int num_walls);
int find_all_stairs_at(Stair stairs[], int num_stairs, int floor, int width_pos, int length_pos, int found_indices[]);
int find_pole_at(Pole poles[], int num_poles, int floor, int width_pos, int length_pos);

// Game objective and win condition functions
void place_random_flag(int flag_position[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
int check_flag_capture(Player *player, const int flag_position[3]);
void check_player_capture(Player players[3], int current_player_id);

// Dynamic game mechanics
void update_stair_directions(Stair stairs[], int num_stairs);

// Special area functions (Bawana effects and movement bonuses)
void reset_to_bawana(Player *player, int player_id);
void apply_bawana_effect(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id);
void apply_movement_bonus(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int player_id);

// Helper and utility functions
int is_in_starting_area(int floor, int width_pos, int length_pos);
void reset_to_starting_area(Player *player, int player_id);
int manhattan_distance(int floor1, int w1, int l1, int floor2, int w2, int l2);
const char* get_direction_name(int direction);
const char* format_position(int floor, int width_pos, int length_pos);
int can_enter_bawana_entrance(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int new_w, int new_l);
int check_path_validity(Player *player,
                        Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                        Stair stairs[], int num_stairs,
                        Pole poles[], int num_poles,
                        Wall walls[], int num_walls,
                        int steps, int player_id, const int flag_position[3],
                        int *first_blocked_step, int *blocking_reason);
const char* get_blockage_reason_description(int blocking_reason);

// Flag validation and reachability helpers
int is_valid_flag_cell(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w, int l);
int is_flag_reachable(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                      Stair stairs[], int num_stairs,
                      Pole poles[], int num_poles,
                      Wall walls[], int num_walls,
                      const int flag_position[3]);

#endif // GAME_H