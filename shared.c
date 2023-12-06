/****************************************************************************************************
 * Name: shared.c                                                                                   *
 * Date created: 2021-12-19                                                                         *
 * Author: Ryan Wells                                                                               *
 * Purpose: Implements the error_check() function. Header file contains shared macros.              *
 ****************************************************************************************************/

#include <stdio.h> // for the type "FILE *"
#include <string.h> // for strcmp()
#include <stdlib.h> // for exit()
#define CLEAR_CONSOLE (void) printf("\033[H\033[2J\033[3J"); // ANSI escapes for clearing screen and scrollback.

/********************************************************************************************************
 * error_check():    Purpose: Checks for errors returned by scanf(), fwrite(), fread(), or fseek()      *
 *                   Parameters: char *function_name --> a string containing a function name            *
 *                               int check_against --> the desired return value                         *
 *                               int return_value --> the actual return value                           *
 *                               FILE *maze_file --> the currently open file to close before exiting    *
 *                   Return value: none                                                                 *
 *                   Side effects: - closes maze_file                                                   *
 *                                 - prints to stdout                                                   *
 *                                 - terminates program                                                 *
 ********************************************************************************************************/
void error_check(char *function_name, int check_against, int return_value, FILE *maze_file)
// Requires <string.h> for strcmp(),
//  requires <stdio.h> for printf() and fclose()
//  & requires <stdlib.h> for exit()
{
    if (return_value != check_against)
    {
        if (strcmp(function_name, "scanf()") == 0)
        {
            CLEAR_CONSOLE;
            (void) printf("Error 1: Could not read input from stdin.\n");
            (void) printf("scanf() returned %d, but should have returned %d\n", return_value, check_against);
            (void) fclose(maze_file);
            exit(1);
        }

        if (strcmp(function_name, "fwrite()") == 0)
        {
            CLEAR_CONSOLE;
            (void) printf("Error 2: Write-to-file error.\n");
            (void) printf("fwrite() returned %d, but should have returned %d\n", return_value, check_against);
            (void) fclose(maze_file);
            exit(2);
        }

        if (strcmp(function_name, "fread()") == 0)
        {
            CLEAR_CONSOLE;
            (void) printf("Error 3: Read-from-file error.\n");
            (void) printf("fread() returned %d, but should have returned %d\n", return_value, check_against);
            (void) fclose(maze_file);
            exit(3);
        }

        if (strcmp(function_name, "fseek()") == 0)
        {
            CLEAR_CONSOLE;
            (void) printf("Error 4: File-position error.\n");
            (void) printf("fseek() returned %d, but should have returned %d\n", return_value, check_against);
            (void) fclose(maze_file);
            exit(4);
        }
    }

    return;
}