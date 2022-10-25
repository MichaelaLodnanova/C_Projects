/*
 * DO NOT MODIFY THIS FILE
 */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef HW02_MINESWEEPER_H
#define HW02_MINESWEEPER_H

//
// GLOBAL CONSTANTS
//

#define MAX_SIZE 99
#define MIN_SIZE 3

// Game states
#define GAME_CONTINUE 0
#define GAME_WON 1
#define GAME_DEFEAT 2

// Load and set related functions

/**
 * @brief Set the cell value (representation) based on the input character
 * Example:
 *
 * set_cell(&board[0][1], 'M');
 * would set the board cell at row=0; column=1 as MINE
 *
 * Possible values:
 *  |X  | Unrevealed cell (that is not a mine or flag)
 *  |M  | Mine
 *  |F  | Cell with mine, but correctly flagged (Flag)
 *  |W  | Cell without mine, but incorrectly flagged (Wrong flag)
 *  |.  | Revealed cell but without information
 *          of number of adj. mines (you need to compute them)
 *  |0-8| Already calculated number of mines
 *
 * @note You should be calling this function in the {load_board}
 *  and {generate_random_board} functions
 *
 * @param cell the single cell (it has to be pointer)
 * @param val The character representation of the cell (see the possible values)
 * @return true if set_cell was successfully done, otherwise false
 */
bool set_cell(uint16_t *cell, char val);

/**
 * @brief Function to process the board after it has been initialized
 *
 * This functions is checking whether the board is valid and computes number
 * of adjacent mines.
 *
 * This function is expected to be called at the end of
 * the `load_board` or `generate_random_board` functions
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @return number of mines if everything is correct, otherwise -1 (any error)
 */
int postprocess(size_t rows, size_t cols, uint16_t board[rows][cols]);

/**
 * @brief Load the board from the STDIN and sets corresponding cells to the correct values
 *
 * if the invalid character was provided, you should ignore it
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @return true if load was successful (you have filled whole board) otherwise false
 */
int load_board(size_t rows, size_t cols, uint16_t board[rows][cols]);

/**
 * @brief BONUS: generate the random board for provided board size
 *
 * You should be using rand(3) function to generate the board
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @param mines Number of mines
 * @return Number of mines or -1 if any error
 */
int generate_random_board(size_t rows, size_t cols, uint16_t board[rows][cols], size_t mines);

// Print/Output related functions
/**
 * @brief Get a cell character representation
 *
 * The function should be called in the print_board
 *
 * @param cell Single cell representation from the board
 * @return
 */
char show_cell(uint16_t cell);

/**
 * @brief Print out whole board nicely according to spec.
 * Example:
 *
 *     0   1   2
 *   +---+---+---+
 *  0|XXX|XXX|XXX|
 *   +---+---+---+
 *  1|XXX|XXX|XXX|
 *   +---+---+---+
 *  2|XXX|XXX|XXX|
 *   +---+---+---+
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @return
 */
int print_board(size_t rows, size_t cols, uint16_t board[rows][cols]);

//
// Checks
//

/**
 * @brief Check whether the provided cell is mine
 * @param cell Single cell representation from the board
 * @return true if the cell is mine otherwise false
 */
bool is_mine(uint16_t cell);

/**
 * @brief Check whether the provided cell is flag
 * @param cell Single cell representation from the board
 * @return true if the cell is flag otherwise false
 */
bool is_flag(uint16_t cell);

/**
 * @brief Check whether the provided cell is revealed
 * @param cell Single cell representation from the board
 * @return true if the cell is revealed otherwise false
 */
bool is_revealed(uint16_t cell);

/**
 * @brief Get a number of adjacent mines (0 when the cell is mine)
 * @param cell Single cell representation from the board
 * @return number of adjacent mines or 0 when the cell is mine
 */
int get_number(uint16_t cell);

//
// Game
//

/**
 * @brief Reveal the cell/s
 * By default all mines will be revealed by the flood-fill method (see the assignment)
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @param row starting row
 * @param col starting colum
 * @return
 */
int reveal_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col);

/**
 * @brief Reveals a single cell
 * @param cell Single cell representation from the board (has to be pointer)
 * @return 1 if mine; 0 if correctly revealed; -1 for any other case (null, already revealed, flagged ...)
 */
int reveal_single(uint16_t *cell);

/**
 * @brief Reveals the all mines using the flood-fill method (see the assignment)
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @param row starting row
 * @param col starting colum
 */
void reveal_floodfill(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col);

/**
 * @brief Flags the cell at the row, col position
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @param row selected row
 * @param col selected colum
 * @return number of flags left (it can be negative number)
 */
int flag_cell(size_t rows, size_t cols, uint16_t board[rows][cols], size_t row, size_t col);

/**
 * @brief Function to check whether the game is solved
 *
 * It means that the only unrevealed cells are the mines
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @return true if the game is solved; false otherwise
 */
bool is_solved(size_t rows, size_t cols, uint16_t board[rows][cols]);

/**
 * @brief Find and Flag mines
 *
 * @param rows Number of rows
 * @param cols Number of cols
 * @param board 2D array that represents the mines board
 * @return number of mines found or -1 if any error
 */
int find_mines(size_t rows, size_t cols, uint16_t board[rows][cols]);

#endif //HW02_MINESWEEPER_H
