## 🗡️ Prince of Inertia
**Prince of Inertia** is a text-based adventure game written in C, inspired by th classic *Prince of Persia*. This project was made for the Procedural Programming exam (A.Y. 2024/2025).

---

## ⚙️ Architetcture
The project was developed following C11 standards and it uses standard C libraries. The software's architecture is modulare and it was divided in three different files:
*main.c*: Contains the *main()* function, managing the game input, as well as the user input.
*gamelib.c*: Contains the game engine, as well as the implementation of the core game functions.
*gamelib.h*: Contains function declarations and defines the custom data structures used in the game.

### Technical Features:
- **Dynamic Data Structures**: The game map is generated procedurally and its implemented as a linked list of *Stanza* (Room) structs.
- **Memory Management**: Extensive use of the dynamic memory allocation functions (*malloc* and *free*) to create and remove the players (1 to 3) and the map rooms.
- **Procedural Generation**: Randomly generates a 15-room map. The algorithm calculates the probability for the appearence of enemies (Skeletons, Guards), traps (Tiles, Bladees. Chasms), and treasures that buff/debuff stats (Heal, Poison).
- **Turn-Based Combat System**: Custom logic that uses a 6-sided die for attacks and defence, with critical hits accounted for.

---

## 🚀 Compilation & Execution
The game is designed to be compiled in a Linux environment (tested on Ubuntu).

To compile the project, run the following commands in your terminal:

```Bash
gcc -c main.c -std=c11 -Wall
gcc -c gamelib.c -std=c11 -Wall
gcc -o gioco main.o gamelib.o
```

The -std=c11 and -Wall flags ensure compliance with the C 2011 standard and help remove all warnings.

To start the game, run:

```Bash
./gioco
```
---
Author: Filippo Castagnola - 376476
