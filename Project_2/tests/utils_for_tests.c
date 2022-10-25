#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "utils_for_tests.h"
#include "../minesweeper.h"

int32_t fill_board(size_t rows, size_t cols, uint16_t board[rows][cols], const char *input) {
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            char val = input[row * cols + col];
            board[row][col] = 0;
            if (!set_cell(&(board[row][col]), val)) {
                return -1;
            }
        }
    }
    return postprocess(rows, cols, board);
}

void fill_board_value(size_t rows, size_t cols, uint16_t board[rows][cols], const char value){
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            set_cell(&board[row][col], value);
        }
    }
}

void fill_board_zeros(size_t rows, size_t cols, uint16_t board[rows][cols]){
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            board[row][col] = 0;
        }
    }
}

void set_mine(uint16_t* cell) {
    set_cell(cell, 'M');
}

void set_flag(uint16_t* cell) {
    set_cell(cell, 'F');
}

void set_revealed(uint16_t* cell) {
    set_cell(cell, '.');
}