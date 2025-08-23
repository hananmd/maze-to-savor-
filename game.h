#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Maze dimensions
#define NUM_FLOORS 3
#define FLOOR_WIDTH 10
#define FLOOR_LENGTH 25

// Max counts
#define MAX_STAIRS 10
#define MAX_POLES  10
#define MAX_WALLS  20

// Player IDs
#define PLAYER_A 0
#define PLAYER_B 1
#define PLAYER_C 2

// Directions
#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3

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
} Cell;

typedef struct {
    int pos[3];          // floor, width, length
    int in_game;         // 0 = in starting area, 1 = in maze
    int direction;       // current direction
    int movement_points; // starts at 100
    int roll_count;      // for direction dice every 4th roll
} Player;

// Function declarations
void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]);
void initialize_players(Player players[3]);
void initialize_stairs(Stair stairs[], int *num_stairs);
void initialize_poles(Pole poles[], int *num_poles);
void initialize_walls(Wall walls[], int *num_walls);
void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls);

int roll_movement_dice(void);
int roll_direction_dice(void);

void enter_maze(Player *player, int player_id);
void move_player_with_teleport(Player *player,
                               Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls,
                               int steps);
int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                     int floor, int w1, int l1, int w2, int l2);
int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l);
int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l);
int check_flag_capture(Player *player, const int flag[3]);

#endif
