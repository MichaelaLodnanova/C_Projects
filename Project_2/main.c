#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "minesweeper.h"

#define DEFAULT_BOARD_SIZE 10
#define MINE_RATIO ((double) 0.1)

#define GM_RANDOM 'r'
#define GM_LOAD 'l'
#define GM_FIND_MINES 'f'

#define ACTION_UNKNOWN 'u'
#define ACTION_REVEAL 'r'
#define ACTION_FLAG 'f'
#define ACTION_DEBUG 'd'

const char *LOGO = "███╗░░░███╗██╗███╗░░██╗███████╗░██████╗░██╗░░░░░░░██╗███████╗███████╗██████╗░███████╗██████╗░\n"
                   "████╗░████║██║████╗░██║██╔════╝██╔════╝░██║░░██╗░░██║██╔════╝██╔════╝██╔══██╗██╔════╝██╔══██╗\n"
                   "██╔████╔██║██║██╔██╗██║█████╗░░╚█████╗░░╚██╗████╗██╔╝█████╗░░█████╗░░██████╔╝█████╗░░██████╔╝\n"
                   "██║╚██╔╝██║██║██║╚████║██╔══╝░░░╚═══██╗░░████╔═████║░██╔══╝░░██╔══╝░░██╔═══╝░██╔══╝░░██╔══██╗\n"
                   "██║░╚═╝░██║██║██║░╚███║███████╗██████╔╝░░╚██╔╝░╚██╔╝░███████╗███████╗██║░░░░░███████╗██║░░██║\n"
                   "╚═╝░░░░░╚═╝╚═╝╚═╝░░╚══╝╚══════╝╚═════╝░░░░╚═╝░░░╚═╝░░╚══════╝╚══════╝╚═╝░░░░░╚══════╝╚═╝░░╚═╝\n";

#define MAX_PROMPT_INPUT_LEN 101

static int prompt_turn(size_t rows, size_t cols, char *action, size_t *row, size_t *col);

static int parse_row_and_col(size_t rows, size_t cols, size_t *row, size_t *col, char *input);

static int play_single_turn(size_t rows, size_t cols, uint16_t board[rows][cols]);

static int play_game(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines);

static char *parse_action(char *action, char *input);

static int
init_board(size_t rows, size_t cols, uint16_t board[rows][cols], char game_mode, size_t *mines);

static int load_args(int argc, char **argv, size_t *rows, size_t *cols, size_t *mines, char *game_mode);

static int parse_arg(int *argc, char **argv, const char *name, char **value);

static int parse_arg_size_t(int *argc, char **argv, const char *name, size_t *value);

static int do_action_reveal(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col);

static int
do_action_flag(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col);

static void strip_newline(char *input);

static void print_help(void);

static int play_find_mines(size_t rows, size_t cols, uint16_t board[rows][cols]);

/**
 * Args:
 * - `--rows (int)`     - number of rows
 * - `--cols (int)`     - number of cols
 * - `--mines (int)`    - number of mines
 * - `--generate`       - generate the board
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
    // yes struct would be better
    size_t rows = DEFAULT_BOARD_SIZE;
    size_t cols = DEFAULT_BOARD_SIZE;
    size_t mines = 0; // don't change
    char game_mode = GM_LOAD;

    // Lets generate some randomness
    srand(time(NULL));
    // NOTICE: In order to have fixed seed, you can use:
    // srand(12345); // where 12345 is your seed

    load_args(argc, argv, &rows, &cols, &mines, &game_mode);

    if ((rows < MIN_SIZE) || (cols < MIN_SIZE)) {
        fprintf(stderr, "Invalid board size: (rows=%zu; cols=%zu)\n", rows, cols);
        return EXIT_FAILURE;
    }

    // Print logo
    printf("\n\n%s\n\n", LOGO);

    uint16_t board[MAX_SIZE][MAX_SIZE] = { 0 };

    if (game_mode == GM_FIND_MINES) {
        return play_find_mines(rows, cols, board);
    }

    if (init_board(rows, cols, board, game_mode, &mines) < 0) {
        fprintf(stderr, "Invalid board: Unable to initialize\n");
        return EXIT_FAILURE;
    }

    if (play_game(rows, cols, board, mines) < 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int play_find_mines(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    puts("You are now in Find Mines mode!\n");
    printf("Board input (rows=%02zu, cols=%02zu) for Find Mines: ", rows, cols);
    int result = find_mines(rows, cols, board);
    if (result < 0) {
        fprintf(stderr, "Unable to find any mine\n");
        return EXIT_FAILURE;
    }
    printf("\nFound mines: %d\n", result);
    print_board(rows, cols, board);
    return EXIT_SUCCESS;
}

int load_args(int argc, char **argv, size_t *rows, size_t *cols, size_t *mines, char *game_mode)
{
    for (int arg_idx = 1; arg_idx < argc; arg_idx++) {
        if (parse_arg_size_t(&arg_idx, argv, "--rows", rows) < 0) {
            return -1;
        }

        if (parse_arg_size_t(&arg_idx, argv, "--cols", cols) < 0) {
            return -1;
        }

        if (parse_arg_size_t(&arg_idx, argv, "--mines", mines) < 0) {
            return -1;
        }

        if (parse_arg(&arg_idx, argv, "--generate", NULL) == 1) {
            *game_mode = GM_RANDOM;
        }

        if (parse_arg(&arg_idx, argv, "--find-mines", NULL) == 1) {
            *game_mode = GM_FIND_MINES;
        }
    }
    return 0;
}

static int parse_arg_size_t(int *argc, char **argv, const char *name, size_t *value)
{
    char *arg, *end;
    int res = parse_arg(argc, argv, name, &arg);
    if (res < 0) {
        fprintf(stderr, "Unable to parse the arg: \"%s\"\n", name);
        return -1;
    }
    if (res == 0) {
        return 0;
    }

    size_t parsed = strtoul(arg, &end, 10);
    if (*end != '\0') {
        fprintf(stderr, "Invalid number: \"%s\" for \"%s\" \n", arg, name);
        return -2;
    }

    *value = parsed;

    return 0;
}

static int parse_arg(int *argc, char **argv, const char *name, char **value)
{
    int i = *argc;
    if (strcasecmp(name, argv[i]) == 0) {
        if (value == NULL) { // it is boolean arg
            return 1;        // found
        }
        *value = argv[i + 1];
        if (*value == NULL) {
            return -2; // no argument provided
        }
        (*argc)++; // skip the arg
        return (int) strlen(*value);
    }
    return 0; // No argument found
}

// Initialization related

static size_t calc_default_mines(size_t rows, size_t cols)
{
    double mines = MINE_RATIO * (double) rows * (double) cols;
    if (mines > 1) {
        return (size_t) mines;
    }
    return 2;
}

static void stdin_cleanup(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // Do nothing
    }
}

static int
init_board(size_t rows, size_t cols, uint16_t board[rows][cols], char game_mode, size_t *mines)
{
    if (game_mode == GM_RANDOM) {
        *mines = (*mines != 0) ? *mines : calc_default_mines(rows, cols);
        return generate_random_board(rows, cols, board, *mines);
    }

    // Load board from STDIN
    printf("Board input (rows=%02zu, cols=%02zu): ", rows, cols);
    int result = load_board(rows, cols, board);
    if (result > 0) {
        *mines = (size_t) result;
    }
    stdin_cleanup();
    return result;
}

// play related
static int play_game(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines)
{
    size_t flags = mines;
    print_help();

    printf("Board: %zu rows and %zu cols "
           "(total cells: %zu, mines: %zu, available flags: %zu)\n\n",
            rows,
            cols,
            rows * cols,
            mines,
            flags);

    print_board(rows, cols, board);
    for (size_t steps = 0;; steps++) {
        int result = play_single_turn(rows, cols, board);
        printf("\n");
        print_board(rows, cols, board);
        switch (result) {
        case GAME_CONTINUE:
            continue;
        case GAME_DEFEAT:
            printf("\nYou have LOST at round %zu\n", steps);
            return GAME_DEFEAT;
        case GAME_WON:
            printf("\nCongratulation, you have WON at round: %zu\n", steps);
            return GAME_WON;
        default:
            printf("\nUnknown state - this should never happen - please contact the developer");
            return -1;
        }
    }
}

static void print_help(void)
{
    puts("Help:");
    puts("For turns you need to provide actions followed by the coordinates");
    puts("(Format: <ACTION><space><ROW_IDX><space><COL_IDX><nl>, Example: \"r 0 0\")\n");
    puts("Available actions:");
    puts("- 'f' \t Flag the cell");
    puts("- 'r' \t Reveal the cell");
    puts("- 'd' \t Debug (you do not need to provide any coordinates)");
    puts("");
}

static int play_single_turn(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    // read user input
    size_t row, col;
    char action = ACTION_UNKNOWN;
    while (true) {
        int res = prompt_turn(rows, cols, &action, &row, &col);
        if (res == -1) { // STDIN closed
            return GAME_DEFEAT;
        }
        if (res == 0) {
            break;
        }
    }
    switch (action) {
    case ACTION_FLAG:
        return do_action_flag(rows, cols, board, row, col);

    case ACTION_REVEAL:
        return do_action_reveal(rows, cols, board, row, col);

    case ACTION_UNKNOWN:
    default:
        return GAME_CONTINUE;
    }
}

static int
do_action_flag(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    printf("Toggle flag on cell at position [%zu, %zu]\n", row, col);
    if (flag_cell(rows, cols, board, row, col) == INT16_MIN) {
        fprintf(stderr, "Unable to flag the cell at position [%zu, %zu]\n", row, col);
    }
    return GAME_CONTINUE;
}

static int do_action_reveal(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    int revealed_cell = reveal_cell(rows, cols, board, row, col);
    printf("Reveal cell at position [%zu, %zu]\n", row, col);

    if (revealed_cell < 0) {
        fprintf(stderr, "Unable to reveal the cell at position [%zu, %zu]\n", row, col);
        return GAME_CONTINUE;
    }

    if (revealed_cell == 1) {
        printf("You have found mine at position [%zu, %zu].\n", row, col);
        return GAME_DEFEAT;
    }
    return is_solved(rows, cols, board) ? GAME_WON : GAME_CONTINUE;
}

// IO related functions

static int prompt_turn(size_t rows, size_t cols, char *action, size_t *row, size_t *col)
{
    printf("\nYour turn (Ex: \"r 0 0\"): ");
    char input[MAX_PROMPT_INPUT_LEN];
    if (fgets(input, MAX_PROMPT_INPUT_LEN, stdin) == NULL) {
        return -1;
    }

    strip_newline(input);
    puts("");
    // parse action
    char *in = parse_action(action, input);
    if (*action == ACTION_DEBUG) {
        *row = 0;
        *col = 0;
    }

    return parse_row_and_col(rows, cols, row, col, in);
}

static void strip_newline(char *input)
{
    input[strcspn(input, "\r\n")] = '\0';
}

char *trim_prefix_spaces(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    while (*str && isspace(*str)) {
        str++;
    }
    return str;
}

static char *parse_action(char *action, char *input)
{
    char *in = trim_prefix_spaces(input);
    char ch = (char) tolower(in[0]);
    switch (ch) {
    case ACTION_FLAG:
    case ACTION_REVEAL:
    case ACTION_DEBUG:
        *action = ch;
        break;
    case '\0':
        *action = ACTION_UNKNOWN;
        break;
    default:
        *action = ACTION_UNKNOWN;
        fprintf(stderr, "Unknown action: %s\n", input);
    }
    for (; !isspace(*in); in++)
        ;
    return in;
}

static int parse_row_and_col(size_t rows, size_t cols, size_t *row, size_t *col, char *input)
{
    char *trm_prefix = trim_prefix_spaces(input);
    char *end;
    const long input_row = strtol(trm_prefix, &end, 10);
    if (input_row < 0 || (size_t) input_row >= rows) {
        printf("Provided row is invalid \"%ld\" from input: \"%s\"\n", input_row, input);
        return 1;
    }
    trm_prefix = trim_prefix_spaces(end);
    const long input_col = strtol(trm_prefix, &end, 10);
    if (input_col < 0 || (size_t) input_col >= cols) {
        printf("Provided col is invalid \"%ld\" from input: \"%s\"\n", input_col, input);
        return 1;
    }
    *row = (input_row);
    *col = (input_col);

    return 0;
}
