#include <ctype.h>
#include "minesweeper.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNUSED(A) (void) (A)
const int ASCII_OFFSET = 48;
const int WRONG_FLAG_OFFSET = 10;
const int HIDDEN_OFFSET = 20;


/* ************************************************************** *
 *                         HELP FUNCTIONS                         *
 * ************************************************************** */
bool is_wrong_flag(uint16_t cell)
{
    return toupper(cell) == 'W' 
    || (cell >= '0' - WRONG_FLAG_OFFSET 
    && cell <= '8' - WRONG_FLAG_OFFSET);
}

bool is_flag(uint16_t cell)
{
    return toupper(cell) == 'F' || is_wrong_flag(cell);
}

bool is_mine(uint16_t cell)
{
    return toupper(cell) == 'M' || toupper(cell) == 'F' || cell == 'N';
}

bool is_number(uint16_t cell)
{
    return cell >= '0' && cell <= '8';
}

bool is_dot(uint16_t cell)
{
    return cell == '.';
}

bool is_revealed(uint16_t cell)
{
    return is_dot(cell) || is_number(cell);
}

bool is_hidden(uint16_t cell)
{
    return toupper(cell) == 'X' 
    || (cell >= '0' - HIDDEN_OFFSET && cell <= '8' - HIDDEN_OFFSET);
}

int get_number(uint16_t cell)
{
    if (is_mine(cell)){
        return 0;
    }

    return cell;
}

/* ************************************************************** *
 *                         INPUT FUNCTIONS                        *
 * ************************************************************** */

bool set_cell(uint16_t *cell, char val)
{
    // if cell pointer is null
    if (cell == NULL)
    {
        return false;
    }
    *cell = 0;

    switch(toupper(val))
    {
        case 'X':
        case 'M':
        case 'F':
        case 'W':
        case '.':
            break;
        default:
            if (val >= '0' && val <= '8')
            {
                break;
            }

            return false;
    }

    *cell = (uint16_t) val;

    return true;
}

bool contains_mine(size_t rows,size_t cols,uint16_t board[rows][cols])
{
    for(size_t i = 0; i < rows; i++)
    {
        for(size_t j = 0; j < cols; j++)
        {
            if (is_mine(board[i][j]))
            {
                return true;
            }
        }
    }

    return false;
}

int load_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{

    for(size_t i = 0; i < rows; i++)
    {
        for(size_t j = 0; j < cols;)
        {
            int input = getchar();

            if (set_cell(&board[i][j],  (char) toupper((char)input)))
            {
                j++;
            }
        }
    }

    // The postprocess function should be called at the end of the load_board function
    return postprocess(rows, cols, board);
}

int get_mine_count(size_t rows, size_t cols,
                 size_t i, size_t j,  uint16_t board[rows][cols])
{
    int mine_count = 0;

    for(int y = 1; y > -2; y--)
    {
        if ((int)i - y < 0 || i - y >= rows)
        {
            continue;
        }
        for (int x = 1; x > -2; x--)
        {
            if ((int)j - x >= 0 && j - x < cols && !(y == 0 && x == 0) 
            && (is_mine(board[i - y][j - x])))
            {
                mine_count++;
            }
        }

    }

    return mine_count;
}

int postprocess(size_t rows, size_t cols, uint16_t board[rows][cols])
{

    //dimension less than min_size or more than max_size
    if (rows < MIN_SIZE || cols < MIN_SIZE || rows > MAX_SIZE || cols > MAX_SIZE){
        return -1;
    }

    //mine in the corner
    if (is_mine(board[0][0]) || is_mine(board[rows - 1][0]) ||
        is_mine(board[0][cols - 1]) || is_mine(board[rows - 1][cols - 1])){
        return -1;
    }

    //no mine in the field
    if (!contains_mine(rows, cols, board)){
        return -1;
    }

    int all_mines = 0;

    for(size_t i = 0; i < rows; i++){
        for(size_t j = 0; j < cols; j++){

            int mine_count = get_mine_count(rows, cols, i, j, board);

            if (is_number(board[i][j])) {
              char number = board[i][j];
              char mine_count_char = mine_count + ASCII_OFFSET;
              if (number != mine_count_char){
                return -1;
              }
            }

            if (is_dot(board[i][j])){
                board[i][j] = mine_count + ASCII_OFFSET;
            }
            else if (is_hidden(board[i][j])){
                board[i][j] = mine_count + ASCII_OFFSET - HIDDEN_OFFSET;
            }
            else if (is_wrong_flag(board[i][j])){
                board[i][j] = mine_count + ASCII_OFFSET - WRONG_FLAG_OFFSET;
            }

            if (is_mine(board[i][j])){
                all_mines++;
            }
        }
    }

    return all_mines;
}

/* ************************************************************** *
 *                        OUTPUT FUNCTIONS                        *
 * ************************************************************** */
void print_edges(size_t cols)
{
    printf("   ");

    for (size_t i = 0; i < cols; i++) {
        printf("+---");
    }

    printf("+\n");
}

void print_header(size_t cols){

    printf("   ");

    for (size_t i = 0; i < cols; i++) {
        if (i < 10) {
            printf("  %ld ", i);
        }
        else{
            printf(" %ld ", i);
        }
    }

    printf("\n");
}

void print_content(size_t row, size_t rows, size_t cols, uint16_t board[rows][cols]){

    for (size_t i = 0; i < cols; i++) {
        int cell = board[row][i];
        if (cell == 'N'){
            printf("| M ");
        }
        else if (is_flag(cell)){
            printf("|_F_");
        }
        else if (is_mine(cell) || is_hidden(cell)){
            printf("|XXX");
        }
        else if (is_revealed(cell)) {
            cell == '0' ? printf("|   ") : printf("| %lc ", cell);
        }
    }

    printf("|\n");
}

int print_board(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    //HEADER
    print_header(cols);

    for(size_t u = 0; u < rows; u++) {

        print_edges(cols);

        //print row number
        if (u < 10) {
            printf(" %ld ", u);
        }
        else{
            printf("%ld ", u);
        }

        print_content(u, rows, cols, board);
    }

    print_edges(cols);

    return 0;
}

char show_cell(uint16_t cell)
{
    if (cell == 'N'){
        return 'M';
    }
    if (is_flag(cell)){
        return 'F';
    }
    if (is_hidden(cell) || is_mine(cell)){
        return 'X';
    }
    if (is_number(cell)){
        return (char)cell;
    }
    if (is_revealed(cell)){
        return ' ';
    }
    return (char) cell;

}

/* ************************************************************** *
 *                    GAME MECHANIC FUNCTIONS                     *
 * ************************************************************** */

int reveal_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    // 1. zavolaj reveal_single
    // ak je return == 0  a hodnota cell je == 0 tak flood_fill
    //pokus o odkrytí políčka mimo hranice herního plán

    uint16_t* cell = &board[row][col];

    int return_type = reveal_single(cell);

    if (return_type == 0 && *cell == '0'){
        reveal_floodfill(rows, cols, board, row, col);
    }

    return return_type;
}

int reveal_single(uint16_t *cell)
{
    // IS NULL
    //pokus o odkrytí již odkrytého políčka
    //pokus o odkrytí políčka označeného vlajkou (je jedno či je to wrong flaga)

    if (cell == NULL || is_revealed(*cell) || (is_flag(*cell) || is_wrong_flag(*cell)))
    {
        return -1;
    }

    //V těchto případech vrací hodnotu -1, jinak vrací 0 a při odkrytí a výbuchu miny vrací 1.

    if (is_hidden(*cell))
    {
        *cell += HIDDEN_OFFSET;
        return 0;
    }
    if (is_wrong_flag(*cell))
    {
        *cell += WRONG_FLAG_OFFSET;
        return 0;
    }

    if (is_mine(*cell))
    {
        *cell = 'N'; // just for me :)
        return 1;
    }

    return -1;
}

void reveal_floodfill(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    for(int y = 1; y > -2; y--)
    {
        if ((int)row - y >= 0 && (int)row - y < (int)rows)
        {
            for (int x = 1; x > -2; x--)
            {
                if ((int)col - x >= 0 && (int)col - x < (int)cols && !(y == 0 && x == 0))
                {
                    uint16_t *cell = &board[row - y][col - x];

                    if (reveal_cell(rows, cols, board, row - y, col - x) == 0
                        && board[row - y][col - x] == '0')
                    {
                        reveal_floodfill(rows, cols, board, row - y, col - x);
                    }

                    // reveal wrong flags :)
                    if (is_wrong_flag(*cell))
                    {
                        *cell += WRONG_FLAG_OFFSET;
                        if (*cell == '0'){
                            reveal_floodfill(rows, cols, board, row - y, col - x);
                        }
                    }
                }
            }
        }
    }
}

int flag_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col)
{
    int mines = 0;

    if (is_flag(board[row][col])){
        if (is_wrong_flag(board[row][col]))
        {
            board[row][col] = get_mine_count(rows, cols, row, col, board) + ASCII_OFFSET - HIDDEN_OFFSET;
        }
        else
        {
            board[row][col] = 'M';
        }
    }
    else
    {
        if (is_revealed(board[row][col]))
        {

        }
        else if (is_mine(board[row][col]))
        {
            board[row][col] = 'F';
        }
        else
        {
            board[row][col] = 'W';
        }
    }

    for(size_t i = 0; i < rows; i++)
    {
        for(size_t j = 0; j < cols; j++)
        {
            if (is_flag(board[i][j]))
            {
                mines--;
            }

            if (is_mine(board[i][j]))
            {
                mines++;
            }
        }
    }

    return mines;
}

bool is_solved(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    int hidden = 0;

    for(size_t i = 0; i < rows; i++)
    {
        for(size_t j = 0; j < cols; j++)
        {
            uint16_t cell = board[i][j];

            if (is_hidden(cell) || is_wrong_flag(cell))
            {
                hidden++;
            }
        }
    }

    return hidden == 0;
}

/* ************************************************************** *
 *                         BONUS FUNCTIONS                        *
 * ************************************************************** */

int generate_random_board(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines)
{
    // Use current time as seed for random generator
    srand(time(0));

    // fill the board with 'X'
    for (int i = 0; i < (int) rows; i++) {
        for (int j = 0; j < (int) cols; j++) {
            board[i][j] = 'X';
        }
    }

    size_t counter = 0;
    while (counter != mines) {
        int row = rand() % (int) rows;
        int col = rand() % (int) cols;
        if ((row == 0 && (col == 0 || col == (int) cols - 1))
            || (row == (int) rows - 1 && (col == 0 || col == (int) cols - 1))) {
            continue;
        }
        if (board[row][col] == 'M') {
            continue;
        }

        board[row][col] = 'M';
        counter++;
    }

    // The postprocess function should be called at the end of generate random board function
    return postprocess(rows, cols, board);
}

int find_mines(size_t rows, size_t cols, uint16_t board[rows][cols])
{
    // TODO: Implement me
    UNUSED(rows);
    UNUSED(cols);
    UNUSED(board);
    return -1;
}
