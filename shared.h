/****************************************************************************************************
 * Name: shared.h                                                                                   *
 * Date created: 2021-12-19                                                                         *
 * Author: Ryan Wells                                                                               *
 * Purpose: Header file for shared.c                                                                *
 ****************************************************************************************************/

#include <stdio.h> // for the type "FILE *"

#define HEADER_SIZE 4
#define BORDER 'B'
#define WALL '1'
#define FLOOR '0'
#define START 'S'
#define END 'E'
#define MAZE_OF_I_OF_J *(maze + ((i * x_dimension) + j))
#define UP (*(&MAZE_OF_I_OF_J - x_dimension))
#define DOWN (*(&MAZE_OF_I_OF_J + x_dimension))
#define LEFT (*(&MAZE_OF_I_OF_J - 1))
#define RIGHT (*(&MAZE_OF_I_OF_J + 1))
#define CLEAR_CONSOLE (void) printf("\033[H\033[2J\033[3J"); // ANSI escapes for clearing screen and scrollback.

void error_check(char *function_name, int check_against, int return_value, FILE *maze_file);