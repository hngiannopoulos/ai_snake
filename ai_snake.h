/**
 *    AI_SNAKE.C
 *    
 *    Library that plays snake.
 *    This library is designed to be very portable 
 *    to a number of diffrent displays though user 
 *    configurable structs that contain function pointers 
 *    that the user refreneces
 *    
 *    Written By: Hristos Giannopoulos
 */

#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include <stdlib.h>

#define WRAP(Z, D)  ( ((Z) < 0) ? 0 : ((Z) > D) ? D : Z)
#define PACK(X, Y)  ((((X) & 0x00FF) << 8) | ((Y) & 0x00FF))  
#define GET_X( Z )  (((Z) >> 8) & 0x00FF)
#define GET_Y( Z )  ((Z) & 0x00FF)

#define MAX_LENGTH 75

#define SNAKE_COLOR     1
#define APPLE_COLOR     2

#define turnRight       4 
#define turnLeft        5
#define noTurn          6 
#define endOfSnake      0xFFFF

typedef enum Buttons {
    UP_B = 1,
    DOWN_B,
    LEFT_B,
    RIGHT_B,
    START_B,
    STOP_B,
    OK_B
} button;

typedef enum directions {
    NOT_MOVING = 0,
    MOVING_UP,
    MOVING_DOWN,
    MOVING_LEFT,
    MOVING_RIGHT
} snake_directions_t;

typedef enum states {
    RESET = 1,
    LOOKING_FOR_DIRECTION,
    DEAD
} snake_states_t;

typedef int (*plot_point)(void*, uint8_t, uint8_t, uint32_t);

typedef int (*clear_disp)(void* );


typedef int (*push_disp)(void* );

typedef struct {
    plot_point      plot_function;      /* function to plot a point */
    clear_disp      clear_function;     /* function to clear display */
    push_disp       push_function;      /* function to draw buffer */
    void *          cookie;             /* this gets passed to each funct */
    uint16_t        board_x;            /* board y dimension */
    uint16_t        board_y;            /* board x dimension */
    uint32_t        color_depth;        /* Bits i.e. 2-Bicolor, 24-RGB */
    uint8_t         single_player;      /* PLACEHOLDER - NOT USED*/
} game_struct_t;

typedef struct {
    uint16_t array[MAX_LENGTH];   /* array of coordinates of snake body */
    snake_directions_t direction;   /* direction the snake is moving*/
    uint16_t length;                /* length of the snake */
    uint16_t apple_pos;             /* masked x,y coordinates of apple */
    snake_states_t state;           /* State, can be written and read*/
} snake_struct_t;

void snakeInit( game_struct_t* gameStruct, snake_struct_t* snakeStruct );
void runFrame();

/*void drawSnake();
byte snakeMove(byte turning);
         
byte pointToBCD(char x, char y);

uint16_t makeApple(uint8_t x_max, y_max);
//globals


byte snakeDirection;
byte snakeLength;
byte apple;
byte snakeState;
struct CRGB * disp;*/

#endif
