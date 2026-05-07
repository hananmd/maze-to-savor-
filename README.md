
# 🧩 Maze of UCSC: Snake & Ladders Variant

**Course:** SCS1301 - Data Structures and Program Design in C  
**Student:** M.Y Hanan Mohamed  
**Institution:** University of Colombo School of Computing  
**Status:** ✅ Fully Implemented — Passes All Critical Test Cases

---

## 📖 Overview

**"Maze to Savor!"** is a 3-player, 3-floor turn-based maze game implemented in C. Players roll dice to move through a complex maze, utilize stairs and poles for teleportation, avoid walls, survive randomized "Bawana" effects, and race to capture the flag.

The game features dynamic mechanics, including direction-changing stairs, movement point (MP) management, and immediate win conditions upon flag capture.

###  Core Features
- **Dice-Based Movement:** Controlled movement with direction logic.
- **Dynamic Stairs:** Direction changes every 5 rounds (Up/Down/Bidirectional).
- **One-Way Poles:** Slide down only; higher priority than stairs.
- **Wall Obstacles:** Configurable walls that block movement.
- **Bawana Area:** A special zone with randomized player effects (e.g., Food Poisoning, Disorientation, MP Bonuses).
- **MP System:** Movement Points with bonuses, penalties, and caps.
- **Player Capture:** Opponents can be captured and reset.
- **Robust Error Handling:** Infinite loop detection, wall sanitization, and unreachable flag validation.
- **Comprehensive Logging:** All actions and assumptions logged to `log.txt`.

---

## ⚙️ Assumptions & Design Decisions

| Feature | Logic |
| :--- | :--- |
| **Bawana Interior** | 12 cells at `[0,6-9,20-24]`. Contains 2 of each effect type + 4 random MP cells. |
| **Bawana Entrance** | Cells `[0,9,19]`. Enterable **only** when MP=0. Forces direction=NORTH. |
| **Starting Area** | `[0,6-9,8-16]`. Players start here and must roll a **6** to enter the maze. |
| **Player Entry** | **A:** `[0,5,12]` (NORTH)<br>**B:** `[0,9,7]` (WEST)<br>**C:** `[0,9,17]` (EAST) |
| **Captured Reset** | Reset to Player A’s start: `[0,6,12]`, Direction NORTH. |
| **Movement Cost** | Each cell has a consumable value (0-4) deducted from MP. |
| **MP Bonuses** | 25% chance: +1-2 MP<br>10% chance: +3-5 MP<br>5% chance: 2x or 3x MP (capped at 250). |
| **Flag Validation** | BFS used at startup to ensure flag is reachable. If not, replaced with a valid cell. |
| **Infinite Loops** | Detected if position repeats within 100 steps. Player reset to `[0,6,12]` with MP preserved. |
| **Wall Sanitization** | Walls overlapping spawn/Bawana/start cells are automatically disabled and logged. |
| **Overlap Priority** | If Stair and Pole exist on same cell: **Pole > Stair**. Tie-breaker: Manhattan distance to flag. |

### 🌀 Bawana Effects
- **Food Poisoning:** Skip 3 turns → Placed randomly in Bawana → New effect applied.
- **Disoriented:** Random direction for 4 turns.
- **Triggered:** Move 2x rolled amount for 4 turns.
- **Happy:** Instant +200 MP (no lasting effect).
- **Random MP:** +10 to +100 MP for 4 turns.
- **MP 0:** Player sent to random Bawana interior cell → Effect applied.

---

## 🚀 How to Compile

You need a C compiler (`gcc` recommended). The `-lm` flag links the math library for `abs()` calculations.

### Linux / macOS
```bash
gcc -o maze main.c game.c -lm
```

### Windows (MinGW/MSYS2)
```bash
gcc -o maze.exe main.c game.c -lm
```

---

## ▶️ How to Run

Place the following files in the same directory as the executable:
- `main.c`, `game.c`, `game.h`
- `seed.txt` (optional)
- `stairs.txt` (optional)
- `poles.txt` (optional)
- `walls.txt` (optional)
- `flag.txt` (optional)

Run the executable:

```bash
./maze       # Linux/macOS
maze.exe     # Windows
```

The game runs automatically in the console. Press **Enter** after each turn if prompted.

###  Output Includes:
- Dice rolls & Movement steps
- Stair/Pole teleports
- Bawana effects & Captures
- MP/Direction status
- Win condition announcement

---

## 📂 Input File Format

All configuration files are plain text with one entry per line.

### `seed.txt`
Single integer for `rand()` seeding.
```text
12345
```

### `stairs.txt`
Format: `[start floor, start w, start l, end floor, end w, end l]`
```text
[0,5,10,1,5,10]
```

### `poles.txt`
Format: `[start floor, end floor, w, l]`
*(Slides from start floor to end floor at coordinates w,l)*
```text
[2,0,5,24]
```

### `walls.txt`
Format: `[floor, start w, start l, end w, end l]`
```text
[1,0,2,8,2]
```

### `flag.txt`
Format: `[floor, w, l]`
```text
[1,3,8]
```

> ️ **Note:** Invalid or missing files will load defaults and log warnings.

---

## 🛡️ Special Game Logics

### 1. Infinite Loop Detection
- Tracks last 100 positions during movement.
- If a revisit occurs: Player reset to `[0,6,12]`, MP preserved, Direction NORTH.
- Logged: `LOOP DETECTION: Player X trapped...`

### 2. Flag Capture Mid-Movement
- Game ends **immediately** if flag is captured at any step, even if movement steps remain.
- Logged: `FLAG CAPTURE OVERRIDE: Player X captured flag at step N...`

### 3. Wall Sanitization
- Walls overlapping critical zones (Spawn, Bawana, Start) are automatically disabled.
- Logged: `ASSUMPTION: Wall at [...] disabled — overlaps spawn cell`

### 4. Stair Direction Updates
- Every 5 rounds, stair directions are randomly updated (UP/DOWN/BIDIRECTIONAL).
- Logged: `ASSUMPTION: Stair directions updated after 5 rounds.`

### 5. Multiple Stairs/Poles at Same Cell
- **Priority:** Poles > Stairs.
- **Tie-Breaker:** Manhattan distance to flag.
- **Determinism:** First encountered wins on distance tie (not random).

---

## ✅ Critical Test Cases Passed

| Test Case | Description | Result |
| :--- | :--- | :--- |
| **1** | Walls on Spawn Cells | Walls disabled → Players enter normally. |
| **2** | Walls Inside Bawana | Walls removed → Bawana remains accessible. |
| **3** | Infinite Stair Loop | Loop detected → Player reset → MP preserved. |
| **4** | Stair-Pole Infinite Loop | Loop detected → Player reset → MP preserved. |
| **5** | Flag in Unreachable Location | BFS fails → Flag replaced → Warning logged. |
| **6** | Overlapping Stairs and Poles | Pole priority respected → Distance-based selection. |
| **7** | Maximum Stair Density (2 per cell) | Distance-based selection → Direction permissions respected. |

---

## 📝 Logging System (`log.txt`)

The game automatically logs all critical events to `log.txt` for debugging and verification.

### Example Log Entries:
```text
ASSUMPTION: Wall at [0,5,12]-[5,12] disabled — overlaps spawn cell
WARNING: Flag in flag.txt at [1,5,5] is unreachable, replacing...
LOOP DETECTION: Player B trapped in infinite loop at [1,5,10]...
FLAG CAPTURE OVERRIDE: Player A captured flag at step 2...
```

---

##  Contact & Credits

**Author:** M.Y Hanan Mohamed  
**Course:** SCS1301 Data Structures and Program Design in C    
**Institution:** University of Colombo School of Computing  
**Date:** September 14, 2025
