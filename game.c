// game.c
#include "game.h"

void initialize_maze(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH]) {
    memset(maze, 0, sizeof(Cell) * NUM_FLOORS * FLOOR_WIDTH * FLOOR_LENGTH);

    // Floor 0: 856 sq ft = 214 blocks
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 0; l < FLOOR_LENGTH; l++) {
            if (w >= START_AREA_W_MIN && w <= START_AREA_W_MAX &&
                l >= START_AREA_L_MIN && l <= START_AREA_L_MAX) {
                maze[0][w][l].is_starting_area = 1;
                maze[0][w][l].is_valid = 0;
            } else {
                maze[0][w][l].is_valid = 1;
            }
            maze[0][w][l].has_wall = 0;
            maze[0][w][l].bawana_cell_type = -1;
            maze[0][w][l].consumable_value = 0;
            maze[0][w][l].bonus_value = 0;
            maze[0][w][l].multiplier = 1;
        }
    }

    // Bawana area: [0,6,20] to [0,9,24]
    for (int w = 6; w <= 9; w++) {
        for (int l = 20; l <= 24; l++) {
            maze[0][w][l].is_valid = 1;
            maze[0][w][l].has_wall = 0;
            maze[0][w][l].bawana_cell_type = -1;
            maze[0][w][l].consumable_value = 0;
            maze[0][w][l].bonus_value = 0;
            maze[0][w][l].multiplier = 1;
        }
    }

    // Bawana wall: [0,6,20] to [0,9,20] and [0,6,20] to [0,6,24]
    for (int w = 6; w <= 9; w++) {
        maze[0][w][20].has_wall = 1;
    }
    for (int l = 20; l <= 24; l++) {
        maze[0][6][l].has_wall = 1;
    }

    // Bawana interior: assign effects
    int bawana_cells[12][2] = {
        {6,21}, {6,22}, {6,23}, {6,24},
        {7,21}, {7,22}, {7,23}, {7,24},
        {8,21}, {8,22}, {8,23}, {8,24}
    };

    int types[] = {BA_FOOD_POISONING, BA_DISORIENTED, BA_TRIGGERED, BA_HAPPY};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            int idx = rand() % 12;
            while (maze[0][bawana_cells[idx][0]][bawana_cells[idx][1]].bawana_cell_type != -1) {
                idx = rand() % 12;
            }
            maze[0][bawana_cells[idx][0]][bawana_cells[idx][1]].bawana_cell_type = types[i];
        }
    }

    // Random MP cells (4 cells)
    for (int i = 0; i < 4; i++) {
        int idx = rand() % 12;
        while (maze[0][bawana_cells[idx][0]][bawana_cells[idx][1]].bawana_cell_type != -1) {
            idx = rand() % 12;
        }
        maze[0][bawana_cells[idx][0]][bawana_cells[idx][1]].bawana_cell_type = BA_RANDOM_MP;
    }

    // Floor 1: 748 sq ft = 187 blocks
    for (int w = 0; w < 10; w++) {
        for (int l = 0; l < 8; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
            maze[1][w][l].bawana_cell_type = -1;
            maze[1][w][l].consumable_value = 0;
            maze[1][w][l].bonus_value = 0;
            maze[1][w][l].multiplier = 1;
        }
    }
    for (int w = 0; w < 10; w++) {
        for (int l = 17; l < 25; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
            maze[1][w][l].bawana_cell_type = -1;
            maze[1][w][l].consumable_value = 0;
            maze[1][w][l].bonus_value = 0;
            maze[1][w][l].multiplier = 1;
        }
    }
    for (int w = 6; w < 10; w++) {
        for (int l = 8; l < 17; l++) {
            maze[1][w][l].is_valid = 1;
            maze[1][w][l].has_wall = 0;
            maze[1][w][l].bawana_cell_type = -1;
            maze[1][w][l].consumable_value = 0;
            maze[1][w][l].bonus_value = 0;
            maze[1][w][l].multiplier = 1;
        }
    }

    // Floor 2: 360 sq ft = 90 blocks
    for (int w = 0; w < FLOOR_WIDTH; w++) {
        for (int l = 8; l < 17; l++) {
            maze[2][w][l].is_valid = 1;
            maze[2][w][l].has_wall = 0;
            maze[2][w][l].bawana_cell_type = -1;
            maze[2][w][l].consumable_value = 0;
            maze[2][w][l].bonus_value = 0;
            maze[2][w][l].multiplier = 1;
        }
    }

    // Initialize consumable, bonus, multiplier (Rule 10)
    int total_cells = 0;
    int valid_cells[3][10][25] = {0};

    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid) {
                    valid_cells[f][w][l] = 1;
                    total_cells++;
                }
            }
        }
    }

    int n_zero = total_cells * 0.25;
    int n_consumable = total_cells * 0.35;
    int n_bonus12 = total_cells * 0.25;
    int n_bonus35 = total_cells * 0.10;
    int n_mult = total_cells * 0.05;

    // Set 25% to 0 consumable
    int placed = 0;
    while (placed < n_zero) {
        int f = rand() % NUM_FLOORS;
        int w = rand() % FLOOR_WIDTH;
        int l = rand() % FLOOR_LENGTH;
        if (!valid_cells[f][w][l]) continue;
        if (maze[f][w][l].consumable_value == 0) {
            maze[f][w][l].consumable_value = 0;
            placed++;
        }
    }

    // Set 35% to 1–4
    placed = 0;
    while (placed < n_consumable) {
        int f = rand() % NUM_FLOORS;
        int w = rand() % FLOOR_WIDTH;
        int l = rand() % FLOOR_LENGTH;
        if (!valid_cells[f][w][l]) continue;
        if (maze[f][w][l].consumable_value == 0) {
            maze[f][w][l].consumable_value = (rand() % 4) + 1;
            placed++;
        }
    }

    // Set 25% bonus 1 or 2
    placed = 0;
    while (placed < n_bonus12) {
        int f = rand() % NUM_FLOORS;
        int w = rand() % FLOOR_WIDTH;
        int l = rand() % FLOOR_LENGTH;
        if (!valid_cells[f][w][l]) continue;
        if (maze[f][w][l].bonus_value == 0 && maze[f][w][l].multiplier == 1) {
            maze[f][w][l].bonus_value = (rand() % 2) + 1;
            placed++;
        }
    }

    // Set 10% bonus 3–5
    placed = 0;
    while (placed < n_bonus35) {
        int f = rand() % NUM_FLOORS;
        int w = rand() % FLOOR_WIDTH;
        int l = rand() % FLOOR_LENGTH;
        if (!valid_cells[f][w][l]) continue;
        if (maze[f][w][l].bonus_value == 0 && maze[f][w][l].multiplier == 1) {
            maze[f][w][l].bonus_value = (rand() % 3) + 3;
            placed++;
        }
    }

    // Set 5% multiplier 2 or 3
    placed = 0;
    while (placed < n_mult) {
        int f = rand() % NUM_FLOORS;
        int w = rand() % FLOOR_WIDTH;
        int l = rand() % FLOOR_LENGTH;
        if (!valid_cells[f][w][l]) continue;
        if (maze[f][w][l].bonus_value == 0 && maze[f][w][l].multiplier == 1) {
            maze[f][w][l].multiplier = (rand() % 2) + 2;
            placed++;
        }
    }
}

void initialize_players(Player players[3]) {
    players[PLAYER_A].pos[0] = 0; players[PLAYER_A].pos[1] = 6; players[PLAYER_A].pos[2] = 12;
    players[PLAYER_A].in_game = 0;
    players[PLAYER_A].direction = DIR_NORTH;
    players[PLAYER_A].movement_points = 100;
    players[PLAYER_A].roll_count = 0;
    players[PLAYER_A].captured = 0;
    players[PLAYER_A].capture_start_pos[0] = 0; players[PLAYER_A].capture_start_pos[1] = 6; players[PLAYER_A].capture_start_pos[2] = 12;
    players[PLAYER_A].bawana_effect = 0;
    players[PLAYER_A].bawana_turns_left = 0;
    players[PLAYER_A].bawana_random_mp = 0;

    players[PLAYER_B].pos[0] = 0; players[PLAYER_B].pos[1] = 9; players[PLAYER_B].pos[2] = 8;
    players[PLAYER_B].in_game = 0;
    players[PLAYER_B].direction = DIR_WEST;
    players[PLAYER_B].movement_points = 100;
    players[PLAYER_B].roll_count = 0;
    players[PLAYER_B].captured = 0;
    players[PLAYER_B].capture_start_pos[0] = 0; players[PLAYER_B].capture_start_pos[1] = 9; players[PLAYER_B].capture_start_pos[2] = 8;
    players[PLAYER_B].bawana_effect = 0;
    players[PLAYER_B].bawana_turns_left = 0;
    players[PLAYER_B].bawana_random_mp = 0;

    players[PLAYER_C].pos[0] = 0; players[PLAYER_C].pos[1] = 9; players[PLAYER_C].pos[2] = 16;
    players[PLAYER_C].in_game = 0;
    players[PLAYER_C].direction = DIR_EAST;
    players[PLAYER_C].movement_points = 100;
    players[PLAYER_C].roll_count = 0;
    players[PLAYER_C].captured = 0;
    players[PLAYER_C].capture_start_pos[0] = 0; players[PLAYER_C].capture_start_pos[1] = 9; players[PLAYER_C].capture_start_pos[2] = 16;
    players[PLAYER_C].bawana_effect = 0;
    players[PLAYER_C].bawana_turns_left = 0;
    players[PLAYER_C].bawana_random_mp = 0;
}

void initialize_stairs(Stair stairs[], int *num_stairs) {
    *num_stairs = 0;
    stairs[(*num_stairs)].start_floor = 0;
    stairs[(*num_stairs)].start_w = 5;
    stairs[(*num_stairs)].start_l = 10;
    stairs[(*num_stairs)].end_floor = 1;
    stairs[(*num_stairs)].end_w = 5;
    stairs[(*num_stairs)].end_l = 10;
    stairs[(*num_stairs)].direction_type = rand() % 3;
    (*num_stairs)++;

    stairs[(*num_stairs)].start_floor = 1;
    stairs[(*num_stairs)].start_w = 7;
    stairs[(*num_stairs)].start_l = 12;
    stairs[(*num_stairs)].end_floor = 2;
    stairs[(*num_stairs)].end_w = 7;
    stairs[(*num_stairs)].end_l = 12;
    stairs[(*num_stairs)].direction_type = rand() % 3;
    (*num_stairs)++;
}

void initialize_poles(Pole poles[], int *num_poles) {
    *num_poles = 0;
    poles[(*num_poles)].start_floor = 2;
    poles[(*num_poles)].end_floor = 0;
    poles[(*num_poles)].w = 5;
    poles[(*num_poles)].l = 24;
    (*num_poles)++;
}

void initialize_walls(Wall walls[], int *num_walls) {
    *num_walls = 0;
    walls[(*num_walls)].floor = 0;
    walls[(*num_walls)].start_w = 6;
    walls[(*num_walls)].start_l = 20;
    walls[(*num_walls)].end_w = 9;
    walls[(*num_walls)].end_l = 20;
    (*num_walls)++;

    walls[(*num_walls)].floor = 0;
    walls[(*num_walls)].start_w = 6;
    walls[(*num_walls)].start_l = 20;
    walls[(*num_walls)].end_w = 6;
    walls[(*num_walls)].end_l = 24;
    (*num_walls)++;

    walls[(*num_walls)].floor = 1;
    walls[(*num_walls)].start_w = 0;
    walls[(*num_walls)].start_l = 2;
    walls[(*num_walls)].end_w = 8;
    walls[(*num_walls)].end_l = 2;
    (*num_walls)++;
}

int roll_movement_dice(void) {
    return (rand() % 6) + 1;
}

int roll_direction_dice(void) {
    return (rand() % 6) + 1;
}

void enter_maze(Player *player, int player_id) {
    switch (player_id) {
        case PLAYER_A: player->pos[1] = 5; player->pos[2] = 12; break;
        case PLAYER_B: player->pos[1] = 9; player->pos[2] = 7;  break;
        case PLAYER_C: player->pos[1] = 9; player->pos[2] = 17; break;
    }
    player->in_game = 1;
}

int is_wall_blocking(Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], int floor, int w1, int l1, int w2, int l2) {
    if (!((w1 == w2 && abs(l1 - l2) == 1) || (l1 == l2 && abs(w1 - w2) == 1))) {
        return 0;
    }

    Wall walls[MAX_WALLS];
    int num_walls;
    initialize_walls(walls, &num_walls);

    for (int i = 0; i < num_walls; i++) {
        Wall wall = walls[i];
        if (wall.floor != floor) continue;

        if (wall.start_l == wall.end_l) {
            int l_wall = wall.start_l;
            int w_min = (wall.start_w < wall.end_w) ? wall.start_w : wall.end_w;
            int w_max = (wall.start_w > wall.end_w) ? wall.start_w : wall.end_w;
            if (l1 == l_wall && l2 == l_wall) {
                int w_min_c = (w1 < w2) ? w1 : w2;
                int w_max_c = (w1 > w2) ? w1 : w2;
                if (w_max_c == w_min_c + 1 && w_max_c <= w_max && w_min_c >= w_min) {
                    return 1;
                }
            }
        }

        if (wall.start_w == wall.end_w) {
            int w_wall = wall.start_w;
            int l_min = (wall.start_l < wall.end_l) ? wall.start_l : wall.end_l;
            int l_max = (wall.start_l > wall.end_l) ? wall.start_l : wall.end_l;
            if (w1 == w_wall && w2 == w_wall) {
                int l_min_c = (l1 < l2) ? l1 : l2;
                int l_max_c = (l1 > l2) ? l1 : l2;
                if (l_max_c == l_min_c + 1 && l_max_c <= l_max && l_min_c >= l_min) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int find_stair_at(Stair stairs[], int num_stairs, int floor, int w, int l) {
    for (int i = 0; i < num_stairs; i++) {
        if ((stairs[i].start_floor == floor && stairs[i].start_w == w && stairs[i].start_l == l) ||
            (stairs[i].end_floor == floor && stairs[i].end_w == w && stairs[i].end_l == l)) {
            return i;
        }
    }
    return -1;
}

int find_pole_at(Pole poles[], int num_poles, int floor, int w, int l) {
    for (int i = 0; i < num_poles; i++) {
        if (poles[i].start_floor == floor && poles[i].w == w && poles[i].l == l) {
            return i;
        }
    }
    return -1;
}

void move_player_with_teleport(Player *player, Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH],
                               Stair stairs[], int num_stairs,
                               Pole poles[], int num_poles,
                               Wall walls[], int num_walls, int steps) {
    for (int s = 0; s < steps; s++) {
        int f = player->pos[0];
        int w = player->pos[1];
        int l = player->pos[2];

        switch (player->direction) {
            case DIR_NORTH: l--; break;
            case DIR_EAST:  w++; break;
            case DIR_SOUTH: l++; break;
            case DIR_WEST:  w--; break;
        }

        if (w < 0 || w >= FLOOR_WIDTH || l < 0 || l >= FLOOR_LENGTH) {
            return;
        }

        if (is_wall_blocking(maze, f, w, l, w, l)) {
            return;
        }

        // Apply consumable cost
        player->movement_points -= maze[f][w][l].consumable_value;
        if (player->movement_points <= 0) {
            reset_to_bawana(player);
            return;
        }

        player->pos[1] = w;
        player->pos[2] = l;

        // Check for stair
        int stair_index = find_stair_at(stairs, num_stairs, f, w, l);
        if (stair_index != -1) {
            Stair s = stairs[stair_index];
            if (s.direction_type == STAIR_UP_ONLY && f == s.start_floor) {
                player->pos[0] = s.end_floor;
                player->pos[1] = s.end_w;
                player->pos[2] = s.end_l;
            } else if (s.direction_type == STAIR_DOWN_ONLY && f == s.end_floor) {
                player->pos[0] = s.start_floor;
                player->pos[1] = s.start_w;
                player->pos[2] = s.start_l;
            } else if (s.direction_type == STAIR_BIDIRECTIONAL) {
                if (f == s.start_floor) {
                    player->pos[0] = s.end_floor;
                    player->pos[1] = s.end_w;
                    player->pos[2] = s.end_l;
                } else {
                    player->pos[0] = s.start_floor;
                    player->pos[1] = s.start_w;
                    player->pos[2] = s.start_l;
                }
            }
            continue;
        }

        // Check for pole
        int pole_index = find_pole_at(poles, num_poles, f, w, l);
        if (pole_index != -1 && f == poles[pole_index].start_floor) {
            player->pos[0] = poles[pole_index].end_floor;
            player->pos[1] = poles[pole_index].w;
            player->pos[2] = poles[pole_index].l;
            continue;
        }
    }
}

void place_random_flag(int flag[3], Cell maze[NUM_FLOORS][FLOOR_WIDTH][FLOOR_LENGTH], Wall walls[], int num_walls) {
    int valid_cells[3][10][25] = {0};
    int total_valid = 0;

    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (maze[f][w][l].is_valid && !maze[f][w][l].has_wall) {
                    valid_cells[f][w][l] = 1;
                    total_valid++;
                }
            }
        }
    }

    if (total_valid == 0) {
        printf("Error: No valid cells for flag placement!\n");
        exit(1);
    }

    int choice = rand() % total_valid;
    int count = 0;

    for (int f = 0; f < NUM_FLOORS; f++) {
        for (int w = 0; w < FLOOR_WIDTH; w++) {
            for (int l = 0; l < FLOOR_LENGTH; l++) {
                if (valid_cells[f][w][l]) {
                    if (count == choice) {
                        flag[0] = f;
                        flag[1] = w;
                        flag[2] = l;
                        return;
                    }
                    count++;
                }
            }
        }
    }
}

int check_flag_capture(Player *player, const int flag[3]) {
    return (player->pos[0] == flag[0] &&
            player->pos[1] == flag[1] &&
            player->pos[2] == flag[2]);
}

void check_player_capture(Player players[3], int current_player) {
    for (int i = 0; i < 3; i++) {
        if (i == current_player) continue;
        if (!players[i].in_game) continue;
        if (players[current_player].pos[0] == players[i].pos[0] &&
            players[current_player].pos[1] == players[i].pos[1] &&
            players[current_player].pos[2] == players[i].pos[2]) {
            printf("Player %c captures Player %c!\n", 'A' + current_player, 'A' + i);
            players[i].in_game = 0;
            players[i].captured = 1;
            switch (i) {
                case 0: players[i].pos[1] = 6; players[i].pos[2] = 12; break;
                case 1: players[i].pos[1] = 9; players[i].pos[2] = 8;  break;
                case 2: players[i].pos[1] = 9; players[i].pos[2] = 16; break;
            }
            printf("Player %c sent back to starting area\n", 'A' + i);
            return;
        }
    }
}

void update_stair_directions(Stair stairs[], int num_stairs) {
    static int round_counter = 0;
    round_counter++;
    if (round_counter % 5 == 0) {
        for (int i = 0; i < num_stairs; i++) {
            stairs[i].direction_type = rand() % 3;
        }
        printf("🔄 Stair directions updated after 5 rounds.\n");
    }
}

void reset_to_bawana(Player *player) {
    player->pos[1] = 9;
    player->pos[2] = 19;
    player->direction = DIR_NORTH;
    player->in_game = 1;
    player->bawana_effect = 0;
    player->bawana_turns_left = 0;
    player->bawana_random_mp = 0;
    printf("🚨 Player %c: Movement points exhausted. Sent to Bawana!\n", 'A' + player->pos[0]);
}