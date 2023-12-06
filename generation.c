/****************************************************************************************************
 * Name: generation.c                                                                               *
 * Date created: 2021-12-19                                                                         *
 * Author: Ryan Wells                                                                               *
 * Purpose: Implements all the functions necessary to procedurally generate a maze.                 *
 ****************************************************************************************************/

#include <stdio.h> // for the type "FILE *" and fwrite()
#include <stdint.h> // for the type "uint8_t"
#include <stdlib.h> // for srand() and rand()
#include <time.h> // for time()
#include <stdbool.h> // for the macros "bool", "false", and "true"
#include "shared.h" // for macros and error_check()

/* Object-Like Macros */
#define DIRECTIONS 4
#define GO_UP 0
#define GO_DOWN 1
#define GO_LEFT 2
#define GO_RIGHT 3
#define NO_VALID_STEP -1
#define NOT_YET_FOUND 4
#define UP_UP (*(&MAZE_OF_I_OF_J - x_dimension - x_dimension))
#define UP_LEFT (*(&MAZE_OF_I_OF_J - x_dimension - 1))
#define UP_RIGHT (*(&MAZE_OF_I_OF_J - x_dimension + 1))
#define DOWN_DOWN (*(&MAZE_OF_I_OF_J + x_dimension + x_dimension))
#define DOWN_LEFT (*(&MAZE_OF_I_OF_J + x_dimension - 1))
#define DOWN_RIGHT (*(&MAZE_OF_I_OF_J + x_dimension + 1))
#define LEFT_UP (*(&MAZE_OF_I_OF_J - 1 - x_dimension))
#define LEFT_DOWN (*(&MAZE_OF_I_OF_J - 1 + x_dimension))
#define LEFT_LEFT (*(&MAZE_OF_I_OF_J - 1 - 1))
#define RIGHT_UP (*(&MAZE_OF_I_OF_J + 1 - x_dimension))
#define RIGHT_DOWN (*(&MAZE_OF_I_OF_J + 1 + x_dimension))
#define RIGHT_RIGHT (*(&MAZE_OF_I_OF_J + 1 + 1))
#define FLIP_COIN (rand() % 2)

/* Internal Function Prototypes */
void draw_border(char *maze, int y_dimension, int x_dimension);
void draw_critical_path(char *maze, int y_dimension, int x_dimension, int start_y, int start_x);
int find_move(char *maze, int current_y, int current_x, int x_dimension);
void draw_dead_ends(char *maze, int y_dimension, int x_dimension);

/********************************************************************************************
 * draw_maze():    Purpose: Procedurally generates maze with the help of subfunctions.      *
 *                          Also saves maze to file.                                        *
 *                 Parameters: FILE *maze_file --> pointer to the file to write to.         *
 *                             int y_dimension --> the height of the maze (in characters)   *
 *                             int x_dimension --> the width of the maze (in characters)    *
 *                 Return value: none                                                       *
 *                 Side effects: - seeds rand()                                             *
 *                               - modifies the file pointed to by maze_file                *
 ********************************************************************************************/
void draw_maze(FILE *maze_file, int y_dimension, int x_dimension)
// Requires <stdio.h> for the type "FILE *",
//  requires <stdint.h> for the type "uint8_t",
//  requires <stdlib.h> for srand() and rand(),
//  requires <time.h> for time(),
//  requires "shared.h" for macros and error_check(),
//  & requires draw_border(), draw_critical_path(), and draw_dead_ends()
{
    // Variable declarations:
    char maze[y_dimension][x_dimension];
    int start_y, start_x;
    uint8_t header[HEADER_SIZE];

    //Initialize maze with purely walls:
    for (int i = 0; i < y_dimension; i++)
        for (int j = 0; j < x_dimension; j++)
            maze[i][j] = WALL;

    // Surround the maze with a special-character border:
    draw_border(*maze, y_dimension, x_dimension);

    // Seeding rand():
    srand(time(NULL));

    // Pick a random Start location:
    start_y = rand() % (y_dimension - 1 - 1); // the "- 1 - 1" is to invalidate starting on the top or bottom
                                              //    since there are borders there.
    start_x = rand() % (x_dimension - 1 - 1); // ditto for the left and right borders
    start_y++; // This iterations offset the start by 1 to ensure the start is not on the top border.
    start_x++; // Offset to ensure the start is not on the left border.
    maze[start_y][start_x] = START;

    // Draw a path to a randomized finish, and mark the End location:
    draw_critical_path(*maze, y_dimension, x_dimension, start_y, start_x);

    // Fill the remainder of the maze with dead ends:
    draw_dead_ends(*maze, y_dimension, x_dimension);

    // Encode the header:
    header[0] = (uint8_t) x_dimension;
    header[1] = (uint8_t) y_dimension;
    header[2] = (uint8_t) start_x;
    header[3] = (uint8_t) start_y;

    // Write the header to file:
    error_check("fwrite()", 1, fwrite(header, HEADER_SIZE, 1, maze_file), maze_file);

    // Write the maze to file:
    for (int i = 0; i < y_dimension; i++)
        for (int j = 0; j < x_dimension; j++)
            error_check("fwrite()", 1, fwrite(&maze[i][j], sizeof(char), 1, maze_file), maze_file);

    return;
}


/******************************************************************************************
 * draw_border():    Purpose: Draws a border around the maze.                             *
 *                   Parameters: char *maze --> pointer to the array containing the maze  *
 *                               int y_dimension --> the height of the maze               *
 *                               int x_dimension --> the width of the maze                *
 *                   Return value: none                                                   *
 *                   Side effects: modifies the maze array                                *
 ******************************************************************************************/
void draw_border(char *maze, int y_dimension, int x_dimension)
// Requires "shared.h" for macros
{
    for (int i = 0; i < y_dimension; i++)
        for (int j = 0; j < x_dimension; j++)
        {
            if (i == 0 || i == y_dimension - 1)
                MAZE_OF_I_OF_J = BORDER;
            else if (j == 0 || j == x_dimension - 1)
                MAZE_OF_I_OF_J = BORDER;
        }
}


/****************************************************************************************************
 * draw_critical_path():    Purpose: Pathfinds from Start until it can't move anymore,              *
 *                                   changing WALLs to FLOORs as it goes. Places End at terminus.   *
 *                          Parameters: char *maze --> pointer to the array storing the maze        *
 *                                      int y_dimension --> the height of the maze                  *
 *                                      int x_dimension --> the width of the maze                   *
 *                                      int start_y --> the y-value of the Start location           *
 *                                      int start_x --> the x-value of the Start location           *
 *                                      int *end_y --> pointer to the y-value of the End location   *
 *                                      int *end_x --> pointer to the x-value of the End location   *
 *                          Return value: none                                                      *
 *                          Side effects: - modifies the maze array                                 *
 *                                        - modifies end_y                                          *
 *                                        - modifies end_x                                          *
 ****************************************************************************************************/
void draw_critical_path(char *maze, int y_dimension, int x_dimension, int start_y, int start_x)
// Requires <stdbool.h> for the macros "bool", "true", and "false",
//  requires "shared.h" for macros,
//  & requires find_move()
{
    // Variable declarations:
    bool valid_moves;
    int i, j;

    valid_moves = true;
    i = start_y;
    j = start_x;

    // Loop until there are no more valid places to move to:
    do
    {
        switch (find_move(maze, i, j, x_dimension))
        {
            case GO_UP: // Randomly-chosen Movement Direction
                UP = FLOOR; // Mark path
                i--; // Move to newly marked path
                break;
            case GO_DOWN:
                DOWN = FLOOR;
                i++;
                break;
            case GO_LEFT:
                LEFT = FLOOR;
                j--;
                break;
            case GO_RIGHT:
                RIGHT = FLOOR;
                j++;
                break;
            case NO_VALID_STEP: // No more steps are valid.
                valid_moves = false; // Break loop.
                break;
        }
    } while (valid_moves);

    // Place End at path terminus:
    MAZE_OF_I_OF_J = END;

    return;
}


/********************************************************************************************
 * find_move():    Purpose: Determines what directions are valid for movement,              *
 *                          and picks a random valid direction.                             *
 *                 Parameters: char *maze --> the array containing the maze                 *
 *                             int current_y --> the y-value of the location to move from   *
 *                             int current_x --> the x-value of the location to move from   *
 *                             int x_dimension --> the width of the maze                    *
 *                 Return value: int --> the direction to move in (or an alert that there   *
 *                                  is no valid move)                                       *
 *                 Side effects: none                                                       *
 ********************************************************************************************/
int find_move(char *maze, int current_y, int current_x, int x_dimension)
// Requires <stdbool.h> for the macros "bool", "true", and "false",
//  requires <stdlib.h> for rand(),
//  & requires "shared.h" for macros
{
    // Variable declarations:
    bool valid_up = true, valid_down = true, valid_left = true, valid_right = true;
    bool valid_move = false;
    int step = NOT_YET_FOUND;
    int i, j;
    int direction;

    // Set location to move from:
    i = current_y;
    j = current_x;

    // Test whether UP is a valid move:
    if (UP != WALL) // This triggers when UP == BORDER, FLOOR, or START.
        valid_up = false;
    else if (UP_UP == FLOOR || UP_UP == START || UP_UP == END) // This makes sure FLOORs are only one-character wide.
        valid_up = false;
    else if (UP_LEFT == FLOOR || UP_LEFT == START || UP_LEFT == END)
        valid_up = false;
    else if (UP_RIGHT == FLOOR || UP_RIGHT == START || UP_RIGHT == END)
        valid_up = false;

    // Test whether DOWN is a valid move:
    if (DOWN != WALL)
        valid_down = false;
    else if (DOWN_DOWN == FLOOR || DOWN_DOWN == START || DOWN_DOWN == END)
        valid_down = false;
    else if (DOWN_LEFT == FLOOR || DOWN_LEFT == START || DOWN_LEFT == END)
        valid_down = false;
    else if (DOWN_RIGHT == FLOOR || DOWN_RIGHT == START || DOWN_RIGHT == END)
        valid_down = false;

    // Test whether LEFT is a valid move:
    if (LEFT != WALL)
        valid_left = false;
    else if (LEFT_UP == FLOOR || LEFT_UP == START || LEFT_UP == END)
        valid_left = false;
    else if (LEFT_DOWN == FLOOR || LEFT_DOWN == START || LEFT_DOWN == END)
        valid_left = false;
    else if (LEFT_LEFT == FLOOR || LEFT_LEFT == START || LEFT_LEFT == END)
        valid_left = false;

    // Test whether RIGHT is a valid move:
    if (RIGHT != WALL)
        valid_right = false;
    else if (RIGHT_UP == FLOOR || RIGHT_UP == START || RIGHT_UP == END)
        valid_right = false;
    else if (RIGHT_DOWN == FLOOR || RIGHT_DOWN == START || RIGHT_DOWN == END)
        valid_right = false;
    else if (RIGHT_RIGHT == FLOOR || RIGHT_RIGHT == START || RIGHT_RIGHT == END)
        valid_right = false;

    // Test whether all directions are invalid:
    if (!valid_up && !valid_down && !valid_left && !valid_right)   
        step = NO_VALID_STEP;

    // Test whether there is a possible step and it's not the End:
    if (step == NOT_YET_FOUND)
    {
        // Loop picks random directions until it picks one that is valid:
        do
        {
            direction = rand() % DIRECTIONS;

            if (direction == GO_UP && !valid_up)
                continue;
            else if (direction == GO_DOWN && !valid_down)
                continue;
            else if (direction == GO_LEFT && !valid_left)
                continue;
            else if (direction == GO_RIGHT && !valid_right)
                continue;
            else
            {
                step = direction;
                valid_move = true;
            }
        } while (!valid_move);
    }

    return step;
}


/*****************************************************************************************
 * draw_dead_ends():    Purpose: Fills remainder of maze with dead ends.                 *
 *                      Parameters: char *maze --> the array containing the maze         *
 *                                  int y_dimension --> the height of the maze           *
 *                                  int x_dimension --> the width of the maze            *
 *                      Return value: none                                               *
 *                      Side effects: modifies the maze array                            *
 *****************************************************************************************/
void draw_dead_ends(char *maze, int y_dimension, int x_dimension)
// Requires <stdbool.h> for the macros "bool", "false", and "true",
//  requires "shared.h" for macros,
//  & requires find_move()
{
    // Variable declarations:
    bool completed = false;
    bool moved;
    bool coin;

    // Loop breaks when, in the whole array, there is not a single valid place to move.
    while (!completed)
    {
        moved = false;
        for (int i = 0; i < y_dimension; i++)
            for (int j = 0; j < x_dimension; j++)
            {
                if (MAZE_OF_I_OF_J == FLOOR)
                {
                    // Decide whether to turn a nearby WALL into a FLOOR:
                    coin = FLIP_COIN;
                    switch (find_move(maze, i, j, x_dimension))
                    {
                        case GO_UP:
                            if (coin)
                                UP = FLOOR;
                            moved = true;
                            break;
                        case GO_DOWN:
                            if (coin)
                                DOWN = FLOOR;
                            moved = true;
                            break;
                        case GO_LEFT:
                            if (coin)
                                LEFT = FLOOR;
                            moved = true;
                            break;
                        case GO_RIGHT:
                            if (coin)
                                RIGHT = FLOOR;
                            moved = true;
                            break;
                        case NO_VALID_STEP:
                            break;
                    }
                }
            }
        // If the whole array was iterated through but not one valid move was found:
        if (moved == false)
            completed = true;
    }
    return;
}