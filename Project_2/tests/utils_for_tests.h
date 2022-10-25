#include <stddef.h>
#include <stdint.h>
#include "../minesweeper.h"

#ifndef HW02_UTILS_FOR_TESTS_H
#define HW02_UTILS_FOR_TESTS_H

int32_t fill_board(size_t rows, size_t cols, uint16_t board[rows][cols], const char *input);
void fill_board_value(size_t rows, size_t cols, uint16_t board[rows][cols], const char value);
void fill_board_zeros(size_t rows, size_t cols, uint16_t board[rows][cols]);

void set_mine(uint16_t* cell);
void set_flag(uint16_t* cell);
void set_revealed(uint16_t* cell);

#endif //HW02_UTILS_FOR_TESTS_H
