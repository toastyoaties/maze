/****************************************************************************************************
 * Name: maze.c                                                                                     *
 * Date created: 2021-12-19 (updated 2022-01-21)                                                    *
 * Author: Ryan Wells                                                                               *
 * Purpose: CLI video game for exploring a procedurally generated maze.                             *
 ****************************************************************************************************/

#include <stdbool.h> // for the macros "bool", "false", and "true"
#include <stdio.h> // for the type "FILE *", the macro "NULL", and printf(), scanf(), getchar(), fopen(), fclose(), fseek(), and fread()
#include <stdlib.h> // for exit()
#include <string.h> // for strcat(), strcpy(), and strlen()
#include <ctype.h> // for tolower()
#include <stdint.h> // for the type "uint8_t"
#include "shared.h" // for macros and error_check()
#include "generation.h" // for draw_maze()

/* Object-Like Macros */
#define MAX_INPUT 10
#define SCAN_MAX "%" STRINGIZE2(MAX_INPUT) "s"
#define MAP_OF_I_OF_J *(map + ((i * x_dimension) + j))

/* Parameterized Macros */
// 2-layer stringization macro, used to allow SCAN_MAX to be dependent on MAX_INPUT:
#define STRINGIZE2(x) STRINGIZE(x)
#define STRINGIZE(x) #x
// Macro to call scanf(), check for errors, and gobble remainder of input line:
#define SCAN(specification_string, destination)                       \
error_check("scanf()", 1, scanf(specification_string, destination), maze_file);  \
while (getchar() != '\n');

/* Function Prototypes */
bool caseless_cmp(char *str1, char *str2);
void play(FILE *maze_file);
void update_map(char *map, char *maze, int player_y, int player_x, int x_dimension);
void print_map(char *map, int y_dimension, int x_dimension);
void read_player(char *command, FILE *maze_file);
void obey_player(char *command, bool *movement, char *maze, int y_dimension, int x_dimension, int *player_y, int *player_x,
                 bool *won, int start_y, int start_x, char *map, FILE *maze_file);

/* Definition of main */
/****************************************************************************************
 * main:     Purpose: Handles file operations and preliminary user input,               *
 *                    then calls play() to run game.                                    *
 *           Parameters: int argc, char **argv                                          *
 *           Return value: int                                                          *
 *           Side effects: - prints to stdout                                           *
 *                         - fetches from stdin                                         *
 *                         - creates or overwrites user-designated file at user request *
 *                         - terminates program                                         *
 ****************************************************************************************/
int main(int argc, char **argv)
// Requires <stdbool.h> for the macros "bool", "false", and "true",
//  requires <stdio.h> for the type "FILE *", the macro "NULL", and printf(), scanf(), getchar(), fopen(), fclose(), and fseek(),
//  requires <stdlib.h> for exit(),
//  requires <string.h> for strcat() and strcpy(),
//  requires <ctype.h> for tolower(),
//  requires "shared.h" for macros and error_check(),
//  requires "generation.h" for draw_maze(),
//  & requires caseless_cmp() and play()
{
    // Variable declarations:
    char input[MAX_INPUT + 1] = {0};
    char output_filename[MAX_INPUT + 1 + 4] = {0}; // "+ 4" is for ".txt"
    FILE *maze_file;
    int x, y;
    bool valid = false, changed_mind = false;
    int y_n;

    // Loop allows users who entered an invalid command-line filename argument to create a new maze file instead:
    do
    {
        // If user did not enter exactly two command-line arguments:
        if (argc != 2)
        {
            // Print usage instructions for user, and terminate program:
            (void) printf("Usage:\n"
                          "\"<program_filename> new\" for new maze\n"
                          "\"<program_filename> <maze_filename>\" for old maze\n");
            exit(0);
        }
        
        // If user wants to create a fresh maze:
        if (caseless_cmp(argv[1], "new") == true || changed_mind == true)
        {
            // Flip changed_mind flag back to normal to prevent infinite loop:
            changed_mind = false;
            // Loop prompts for and stores valid filename to be used for the maze:
            do
            {
                // Prompt user for filename:
                (void) printf("Desired filename (%d characters maximum, no spaces):\n", MAX_INPUT);
                SCAN(SCAN_MAX, input)
                // Add extension:
                (void) strcat(strcpy(output_filename, input), ".txt");
                // Test whether a file with this name already exists:
                maze_file = fopen(output_filename, "r");
                if (maze_file == NULL)
                    valid = true;
                // If a file with this name already exists, confirm whether to save over it:
                else
                {
                    (void) printf("A file with this filename already exists. Overwrite file? (y/n)\n");
                    do
                    {
                        y_n = tolower(getchar());
                        while (getchar() != '\n');
                    } while (y_n != 'y' && y_n != 'n');
                    if (y_n == 'y')
                        valid = true;
                    (void) fclose(maze_file);
                }
            } while (!valid);
            // Prompt user for width and height (in characters) of maze:
            do
            {
                (void) printf("Desired width (10 - 150): ");
                SCAN("%d", &x)
            } while (x < 10 || x > 150);
            do
            {
                (void) printf("Desired height (10 - 50): ");
                SCAN("%d", &y)
            } while (y < 10 || y > 50);
            // Create (or overwrite) designated file:
            maze_file = fopen(output_filename, "w+");
            // Create maze and save to file:
            draw_maze(maze_file, y, x);
            // Ready file for reading:
            error_check("fseek()", 0, fseek(maze_file, 0, SEEK_SET), maze_file);
            // Run the game, using the new-maze file:
            play(maze_file);
        }
        // If the user wants to play a previously generated maze:
        else
        {
            // Open maze file:
            maze_file = fopen(argv[1], "r");
            // Check whether file with given name exists:
            if (maze_file == NULL)
            {
                // Report missing file and prompt for whether user wants to generate new maze instead:
                (void) printf("Invalid filename; could not open.\n");
                (void) printf("Would you like to create a new maze instead? (y/n)\n");
                do
                {
                    y_n = tolower(getchar());

                    while (getchar() != '\n')
                        ;
                } while (y_n != 'y' && y_n != 'n');

                if (y_n == 'y')
                    changed_mind = true;
            }
            // If file with given name does exist, run the game using that file:
            else
            {
                play(maze_file);
            }
        }
        // After the game is over, close the maze file:
        (void) fclose(maze_file);
    } while (changed_mind == true);

    return 0;
}

/* Definitions of other functions */
/***************************************************************************************************************
 * caseless_cmp():    Purpose: Compares two strings irrespective of character case.                            *
 *                    Parameters: char *str1 --> the string to be compared against                             *
 *                                char *str2 --> the string to compare                                         *
 *                    Return value: bool --> 1 if the strings are the same (ignoring case), 0 if different     *
 *                    Side effects: none                                                                       *
 ***************************************************************************************************************/
bool caseless_cmp(char *str1, char *str2)
// Requires <stdbool.h> for the macros "bool", "false", and "true"
//  requires <string.h> for strlen(),
//  & requires <ctype.h> for tolower()
{
    if (strlen(str1) != strlen(str2))
        return false;
    // Loop terminates at null character:
    for (int i = 0; str1[i]; i++)
        if (tolower(str1[i]) != tolower(str2[i]))
            return false;
    return true;
}


/********************************************************************************
 * play():    Purpose: Plays the game.                                          *
 *            Parameters: FILE *maze_file --> file to be used for the game      *
 *            Return value: none                                                *
 *            Side effects: - moves the file position indicator for maze_file   *
 *                          - clears the terminal screen                        *
 *                          - clears the terminal scrollback                    *
 *                          - prints to stdout                                  *
 *                          - fetches from stdin                                *
 ********************************************************************************/
void play(FILE *maze_file)
// Requires <stdio.h> for the type "FILE *" and for fread(), printf(), and getchar()
//  requires <stdint.h> for the type "uint8_t",
//  requires <stdbool.h> for the macros "bool" and "false",
//  requires "shared.h" for macros and error_check(),
//  & requires update_map(), print_map(), read_player(), and obey_player()
{
    // Early variable declarations:
    uint8_t header[HEADER_SIZE];
    int x_dimension, y_dimension, start_x, start_y, player_x, player_y;
    char command[MAX_INPUT + 1] = {0};
    bool movement, won = false;

    // Decode file header:
    error_check("fread()", 1, fread(header, HEADER_SIZE, 1, maze_file), maze_file);
    x_dimension = header[0];
    y_dimension = header[1];
    start_x = header[2];
    start_y = header[3];

    // Set player start location:
    player_x = start_x;
    player_y = start_y;

    // Late variable declarations (dependent on header being decoded):
    char maze[y_dimension][x_dimension];
    char map[y_dimension][x_dimension];

    // Read maze from file into variable:
    error_check("fread()", 1, fread(maze[0], (int) sizeof(char) * y_dimension * x_dimension, 1, maze_file), maze_file);

    // Initialize map to all null characters + border:
    for (int i = 0; i < y_dimension; i++)
        for (int j = 0; j < x_dimension; j++)
            map[i][j] = maze[i][j] == BORDER ? WALL : '\0';

    // Gameplay loop:
    (void) printf("\a");
    while (!won)
    {
        CLEAR_CONSOLE;
        update_map(*map, *maze, player_y, player_x, x_dimension);
        print_map(*map, y_dimension, x_dimension);

        movement = false;
        do
        {
            read_player(command, maze_file);
            obey_player(command, &movement, *maze, y_dimension, x_dimension, &player_y, &player_x, &won, start_y, start_x, *map, maze_file);
        } while (!movement);
    }

    // Winning sequence (from here to end of function):
    CLEAR_CONSOLE;
    (void) printf("\a\a\a");
    (void) printf(
           "************************************************************\n"
           "* __   __   ___    _   _    __        __  ___   _   _   _  *\n"
           "* \\ \\ / /  / _ \\  | | | |   \\ \\      / / |_ _| | \\ | | | | *\n"
           "*  \\ V /  | | | | | | | |    \\ \\ /\\ / /   | |  |  \\| | | | *\n"
           "*   | |   | |_| | | |_| |     \\ V  V /    | |  | |\\  | |_| *\n"
           "*   |_|    \\___/   \\___/       \\_/\\_/    |___| |_| \\_| (_) *\n"
           "************************************************************\n");

    (void) printf("\n\n\n\n\n----press ENTER----\n\n");
    while (getchar() != '\n');

    CLEAR_CONSOLE;
    (void) printf("Complete map:\n");
    for (int i = 0; i < y_dimension; i++)
    {
        for (int j = 0; j < x_dimension; j++)
            (void) printf("%c", maze[i][j] == '0' ? ' ' : maze[i][j] == BORDER ? WALL : maze[i][j]);
        (void) printf("\n");
    }
    (void) printf("\n\n\n\n\n----press ENTER----\n\n");
    while (getchar() != '\n');

    CLEAR_CONSOLE;
    return;
}


/****************************************************************************************
 * update_map():    Purpose: Removes "fog of war" from map based on player location.    *
 *                  Parameters: char *map --> the array containing the player map       *
 *                              char *maze --> the array containing the maze            *
 *                              int player_y --> the y-value of the player's location   *
 *                              int player_x --> the x-value of the player's location   *
 *                              int x_dimension --> the width of the maze               *
 *                  Return value: none                                                  *
 *                  Side effects: modifies the array containing the player's map        *
 ****************************************************************************************/
void update_map(char *map, char *maze, int player_y, int player_x, int x_dimension)
//  Requires "shared.h" for macros
{
    int i, j;

    // Mark current position and surrounding four cardinal positions on player map:
    i = player_y, j = player_x;
    MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : MAZE_OF_I_OF_J;
    i--;
    MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : MAZE_OF_I_OF_J;
    i++, i++;
    MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : MAZE_OF_I_OF_J;
    i--, j++;
    MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : MAZE_OF_I_OF_J;
    j--, j--;
    MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : MAZE_OF_I_OF_J;

    // Mark player on map:
    i = player_y, j = player_x;
    MAP_OF_I_OF_J = '*';

    return;
}


/************************************************************************************
 * print_map():    Purpose: Prints player's map and its key.                        *
 *                 Parameters: char *map --> the array containing the player's map  *
 *                             int y_dimension --> the height of the maze           *
 *                             int x_dimension --> the width of the maze            *
 *                 Return value: none                                               *
 *                 Side effects: prints to stdout                                   *
 ************************************************************************************/
void print_map(char *map, int y_dimension, int x_dimension)
// Requires <stdio.h> for printf(),
//  & requires "shared.h" for macros
{
    (void) printf("Current map:\n");
    for (int i = 0; i < y_dimension; i++)
    {
        for (int j = 0; j < x_dimension; j++)
            (void) printf("%c", MAP_OF_I_OF_J == '0' ? ' ' : MAP_OF_I_OF_J == '\0' ? ' ' : MAP_OF_I_OF_J);
        (void) printf("\n");
    }
    (void) printf("\nKey:\n'*' = player | '1' = wall | 'S' = starting point | 'E' = exit\n\n");
}


/************************************************************************************
 * read_player():    Purpose: Prompts for and stores player commands.               *
 *                   Parameters: char *command --> the array to store a command     *
 *                               FILE *maze_file --> the file containing the maze   *
 *                   Return value: none                                             *
 *                   Side effects: - prints to stdout                               *
 *                                 - fetches from stdin                             *
 ************************************************************************************/
void read_player(char *command, FILE *maze_file)
// Requires <stdio.h> for the type "FILE *" and for printf(), scanf(), and getchar(),
//  & requires "shared.h" for macros and error_check()
{
    (void) printf("Type command ('help' for help): ");
    SCAN(SCAN_MAX, command)
}


/********************************************************************************************************
 * obey_player():    Purpose: Enacts player commands.                                                   *
 *                   Parameters: char *command --> the array containing the current command             *
 *                               bool *movement --> pointer to a bool stating whether player has moved  *
 *                               char *maze --> the array containing the maze                           *
 *                               int y_dimension --> the height of the maze                             *
 *                               int x_dimension --> the width of the maze                              *
 *                               int *player_y --> pointer to the y-value of the player's location      *
 *                               int *player_x --> pointer to the x-value of the player's location      *
 *                               bool *won --> pointer to a bool stating whether the player has won     *
 *                               int start_y --> pointer to the y-value of the Start location           *
 *                               int start_x --> pointer to the x-value of the Start location           *
 *                               char *map --> the array containing the player's map                    *
 *                               FILE *maze_file --> the file containing the maze
 *                   Return value: none                                                                 *
 *                   Side effects: - modifies int *player_y                                             *
 *                                 - modifies bool *movement                                            *
 *                                 - modifies bool *won                                                 *
 *                                 - prints to stdout                                                   *
 *                                 - modifies int *player_x                                             *
 *                                 - modifies the array containing the player's map                     *
 *                                 - terminates the program                                             *
 ********************************************************************************************************/
void obey_player(char *command, bool *movement, char *maze, int y_dimension, int x_dimension, int *player_y, int *player_x,
                 bool *won, int start_y, int start_x, char *map, FILE *maze_file)
// Requires <stdbool.h> for the macros "bool" and "true",
//  requires <stdio.h> for the type "FILE *" and printf(),
//  requires <stdlib.h> for exit(),
//  requires "shared.h" for macros,
//  & requires caseless_cmp()
{
    int i = *player_y, j = *player_x;

    // Handle player movement:
    if (caseless_cmp(command, "up") == true || caseless_cmp(command, "w") == true)
    {
        if (UP == FLOOR || UP == START)
        {
            --*player_y;
            *movement = true;
        }
        else if (UP == END)
        {
            *won = true;
            *movement = true;
        }
        else
            (void) printf("Cannot move into wall.\n");
    }
    else if (caseless_cmp(command, "down") == true || caseless_cmp(command, "s") == true)
    {
        if (DOWN == FLOOR || DOWN == START)
        {
            ++*player_y;
            *movement = true;
        }
        else if (DOWN == END)
        {
            *won = true;
            *movement = true;
        }
        else
            (void) printf("Cannot move into wall.\n");
    }
    else if (caseless_cmp(command, "left") == true || caseless_cmp(command, "a") == true)
    {
        if (LEFT == FLOOR || LEFT == START)
        {
            --*player_x;
            *movement = true;
        }
        else if (LEFT == END)
        {
            *won = true;
            *movement = true;
        }
        else
            (void) printf("Cannot move into wall.\n");
    }
    else if (caseless_cmp(command, "right") == true || caseless_cmp(command, "d") == true)
    {
        if (RIGHT == FLOOR || RIGHT == START)
        {
            ++*player_x;
            *movement = true;
        }
        else if (RIGHT == END)
        {
            *won = true;
            *movement = true;
        }
        else
            (void) printf("Cannot move into wall.\n");
    }
    // Print help menu:
    else if (caseless_cmp(command, "help") == true)
    {
        (void) printf("----Valid Commands----\n"
                      "Function commands:\n"
                      "\tHelp: prints this listing\n"
                      "\tRestart: erases the map and places player back at start\n"
                      "\tQuit: terminates the program\n"
                      "Movement commands:\n"
                      "\tUp or W: moves the player up one space\n"
                      "\tDown or S: moves the player down one space\n"
                      "\tLeft or A: moves the player left one space\n"
                      "\tRight or D: moves the player right one space\n"
                      "\nCommands are not case-sensitive.\n");
    }
    // Reset current maze:
    else if (caseless_cmp(command, "restart") == true)
    {
        for (int i = 0; i < y_dimension; i++)
            for (int j = 0; j < x_dimension; j++)
                MAP_OF_I_OF_J = MAZE_OF_I_OF_J == BORDER ? WALL : '\0';
        *player_y = start_y;
        *player_x = start_x;
        *movement = true;
    }
    // Quit game:
    else if (caseless_cmp(command, "quit") == true)
    {
        CLEAR_CONSOLE;
        (void) fclose(maze_file);
        exit(0);
    }
    //  Default:
    else
    {
        (void) printf("Unrecognized command. Type 'help' for help.\n");
    }

    return;
}