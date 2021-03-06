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
#define PACK(X, Y)  ( ( ((X) << 8)  & 0xFF00) | ((Y) & 0x00FF))  
#define GET_X( Z )  (((Z) >> 8) & 0x00FF)
#define GET_Y( Z )  ((Z) & 0x00FF)
#define W_C(X) ((X) != 0) ? (X) : 0xFFFF
#define WEIGHT_CHECK( X, Y, Z) ( W_C(X) < W_C(Y) ) \
                             && ( W_C(X) < W_C(Z) )

#define STATISTICS_ON

#ifdef STATISTICS_ON                             
#define MAX_LENGTH 300
#else
#define MAX_LENGTH 120
#endif
#define WRAP_OFF


/*The maximum length of the print buffer*/
/* For the diagnostic messages */
#define SPRINTF_BUFF_LEN 80

#define TURN_RIGHT       1 
#define TURN_LEFT        3
#define NO_TURN          0 
#define TURN_ERROR       4

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
    MOVING_UP    = 0,
    MOVING_RIGHT = 1,
    MOVING_DOWN  = 2,
    MOVING_LEFT  = 3
} snake_directions_t;

typedef enum states {
    RESET = 1,
    LOOKING_FOR_DIRECTION,
    DEAD, 
    WON
} snake_states_t;

typedef int (*plot_point)(void*, uint8_t, uint8_t, uint32_t);

typedef int (*clear_disp)(void* );

typedef int (*push_disp)(void* );

typedef int (*print_funct)(void *, const char *, ...);

typedef struct {
    plot_point      plot_function;      /* function to plot a point */
    clear_disp      clear_function;     /* function to clear display */
    push_disp       push_function;      /* function to draw buffer */
    print_funct     print_function;     /* Function that prints message*/
    void *          cookie;             /* this gets passed to each funct */
    uint8_t        board_x;             /* board y dimension */
    uint8_t        board_y;             /* board x dimension */
    uint32_t        color_depth;        /* Bits i.e. 2-Bicolor, 24-RGB */
    uint8_t         single_player;      /* PLACEHOLDER - NOT USED*/ 

    uint8_t         manhattan_weight;
    uint8_t         turn_weight;
    uint8_t         no_turn_weight;
    uint8_t         look_ahead_distance;
    uint8_t         look_ahead_weight;
} game_struct_t;

typedef struct {
    uint16_t array[MAX_LENGTH+1];   /* array of coordinates of snake body */
    snake_directions_t direction;   /* direction the snake is moving*/
    uint16_t length;                /* length of the snake */
    uint16_t apple_pos;             /* masked x,y coordinates of apple */
    uint32_t apple_color;
    uint32_t snake_color;
    snake_states_t state;           /* State, can be written and read*/
#ifdef STATISTICS_ON
    uint32_t    apple_count;   
#endif 
} snake_struct_t;

void snakeInit( game_struct_t* gameStruct, snake_struct_t* snakeStruct );
void drawSnake();
void runFrame();

#endif
