#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h> //library for function strncat() - add char to string
#define INT_BITS 64

void print_error_message(char *message)
{
    fprintf(stderr, "%s\n", message);
}

/**
 * this function returns true when input is a command
 */
bool isCommand(int ch)
{
    switch(ch)
    {
        case 'P':
        case 'N':
        case ';':
        case '=':
        case 'O':
        case 'T':
        case 'X':
        case 'M':
        case 'R':
            return true;
        default:
            return false;
    }
}

/**
 * function which returns true when input is an operator
 */
bool isOperator(int ch)
{
    switch(ch)
    {
        case '+':
        case '-':
        case '*':
        case '%':
        case '/':
        case '<':
        case '>':
        case 'm':
        case 'l':
        case 'r':
            return true;
        default:
            return false;
    }
}

/**
 * method for reading notes
 */
void getNote()
{
    int ch;
    while (((ch = getchar()) != '\n') && ch != EOF) {

    }
}

/**
 * method using getchar function, ignoring spaces and ends of lines,
 * if not any of choices, reports syntax error
 * @return next command or syntax error if else
 */
int getNextCommand ()
{
    bool hasCommand = false;
    int command;
    int ch;
    while ((!hasCommand) && ((ch = getchar()) != EOF))
    {
        if (isspace(ch)) //ignore spaces and ends of lines
        {
            continue;
        }
        command = ch;
        hasCommand = true;
    }
    if (ch == EOF)
    {
        command = ch;
    }
    if (hasCommand && !(isCommand(command)) && !(isOperator(command)))
    {
        print_error_message("Syntax error");
        return -2;

    }
    return command;
}

/**
 * simple method for printing accumulator value
 * @param result is an integer that is supposed to be printed
 */
void printAccumulator(uint64_t result)
{
    printf("# %lu\n", result);
}

void binary(uint64_t result){
    if (result == 0){
        printf("# 0\n");
        return;
    }
    uint64_t number = result;
    uint64_t size = 0;
    while (number != 0){
        number >>= 1;
        size++;
    }
    printf("# ");
    while (size != 0){
        size--;
        printf("%lu", (result >> size) & 1);
    }
    printf("\n");
}
/** this method is used when command is supposed to convert input into different numerical system
* @param result is an integer in decimal
* @param command is supposed to tell us in which numerical system
* is result supposed to be printed
**/
void printAccumulatorInNumeral(uint64_t result, int command)
{
    switch (command)
    {
        case 'T':
            binary(result);
            break;
        case 'O':
            printf("# %lo\n", result);
            break;
        case 'X':
            printf("# %lX\n", result);
            break;
        default:
            break;
    }

}

/**
 * numsToStringToInt function deals with an argument which continues after a command or
 * an operator. If it is a number, it simply converts it into integer
 * @param nextCommand is stored in this function because it needs to be actual
 * @param numeral is integer - stores in which base converted int should be printed
 * @param accValue stores value of accumulator
 * @param memoryVal stores value of memory
 * @return
 */
long long numsToStringToInt(int *nextCommand, int *numeral, uint64_t accValue, uint64_t memoryVal)
{
    char str[150] = {'\0'};
    char chr;
    *numeral = 10;
    bool finished = false;
    while (!finished)
    {
        chr = (char) getchar();

        if (isspace(chr))
        {
            continue;
        }

        else if ((chr == 'T') || (chr == 'X') || (chr == 'O'))
        {
            switch (chr) {
                case 'T':
                    *numeral = 2;
                    break;
                case 'X':
                    *numeral = 16;
                    break;
                case 'O':
                    *numeral = 8;
                    break;
                default:
                    break;
            }
            printAccumulatorInNumeral(accValue, chr);
        }
        else if (chr == 'm')
        {
            *nextCommand = getNextCommand();
            return (long long) memoryVal;
        }
        else if (!isCommand(chr) && !(isOperator(chr)) && ((chr >= 'g' && chr <= 'z') || (chr >= 'G' && chr <= 'Z')))
        {
            print_error_message("Syntax error");
            return -2;
        }

        else if ((chr == EOF) || isCommand(chr) || isOperator(chr))
        {
            *nextCommand = (int) chr;
            finished = true;
        }
        else
        {
            strncat(str,&chr, 1);
        }
    }
    char *ptr = NULL;
    uint64_t converted;
    converted = strtoul(str, &ptr, *numeral);
    if (!(str[0] == '0') || !(str[1] == '\0')) {
        if (converted == 0) {
            // conversion failed
            *numeral = -1;
        }
    }
    return (long long) converted;

}
int processOperator(int command, uint64_t *accumulator, uint64_t *memory, int numeral, int nextCommand){
    uint64_t number;
    switch (command) {
        case '+':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator += number;
            printAccumulator(*accumulator);
            break;
        case '-':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);

            if ((number > *accumulator) || (numeral == -1)) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator -= number;
            printAccumulator(*accumulator);

            break;
        case '*':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator *= number;
            printAccumulator(*accumulator);
            break;
        case '/':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            if (number == 0) {
                print_error_message("Division by zero");
                return -2;
            }
            *accumulator /= number;
            printAccumulator(*accumulator);

            break;
        case '%':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            } else if (number == 0) {
                print_error_message("Division by zero");
                return -2;
            } else {
                *accumulator %= number;
                printAccumulator(*accumulator);
            }
            break;
        case '<':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator = *accumulator << number;
            printAccumulator(*accumulator);
            break;
        case '>':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator = *accumulator >> number;
            printAccumulator(*accumulator);
            break;
        case 'l':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator = (*accumulator << number) | (*accumulator >> (INT_BITS - number));
            printAccumulator(*accumulator);
            break;
        case 'r':
            number = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
            if (numeral == -1) {
                print_error_message("Out of range");
                return -2;
            }
            *accumulator = (*accumulator >> number) | (*accumulator << (INT_BITS - number));
            printAccumulator(*accumulator);
            break;
        default:
            break;
    }
    return nextCommand;
}
/**
 * this is the most important function which is used for processing commands and
 * operators, here is basically implemented what should calculator do after each
 * command or operator
 * @return next command and print accumulator value
 */
int processCommand(int command, uint64_t *accumulator, uint64_t *memory)
{

    int nextCommand = EOF;
    int numeral = 10;
    long long argument;
    if (isCommand(command))
    {
        switch (command)
        {
            case 'P':
                argument = numsToStringToInt(&nextCommand, &numeral, *accumulator, *memory);
                if ((argument < 0) || (numeral == -1))
                {
                    print_error_message("Out of range");
                    return -2;
                }
                *accumulator = argument;
                printAccumulator(*accumulator);
                break;
            case ';':
                getNote();
                nextCommand = getNextCommand();
                break;
            case 'N':
                *accumulator = 0;
                printAccumulator(*accumulator);
                nextCommand = getNextCommand();
                break;
            case '=':
                printAccumulator(*accumulator);
                nextCommand = getNextCommand();
                break;
            case 'T':
            case 'O':
            case 'X':
                printAccumulatorInNumeral(*accumulator, command);
                nextCommand = getNextCommand();
                break;
            case 'M':
                *memory = *accumulator;
                nextCommand = getNextCommand();
                break;
            case 'R':
                *memory = 0;
                nextCommand = getNextCommand();
                break;
            default:
                break;
        }
    }
    else if (isOperator(command))
    {
        nextCommand = processOperator(command, accumulator, memory, numeral, nextCommand);
    }
    else if (command != EOF)
    {
        print_error_message("Syntax error");
        return -2;
    }

    return nextCommand;
}


bool calculate(void)
{
    uint64_t memory = 0;
    uint64_t accumulator = 0;
    //get first command
    int command = getNextCommand();

    do
    {
        if (command < 0)
        {
            return false;
        }
        command = processCommand(command, &accumulator, &memory);
    }
    while (command != EOF);

    if (command == -2)
    {
        return false;
    }
    return true;
}


int main(void)
{
    if (!calculate()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
