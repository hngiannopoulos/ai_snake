/*
  AI_SNAKE Weight Test Program
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ai_snake.h"

#define RUN_MAX 10000
#define WEIGHT_RANGE 10

game_struct_t gameStruct;
snake_struct_t snakeStruct;

int ht1632c_plot(void* cookie, uint8_t X, uint8_t Y, uint32_t color);
int ht1632c_clear(void* cookie); 
int ht1632c_push(void * cookie);

#if 0
void printStruct(snake_struct_t * ss){
    Serial.print("Apple "); Serial.print(GET_X(ss->apple_pos)); Serial.print(" "); Serial.println(GET_Y(ss->apple_pos));
    for(uint8_t i = 0; i < (ss->length + 1); i++){
        Serial.print(GET_X(ss->array[i]), HEX);
        Serial.println(GET_Y(ss->array[i]), HEX);
    }
}
#endif

int main(){
    unsigned int win_count   = 0;
    unsigned int loose_count = 0;
    unsigned int apple_count = 0;
    /* Setup the Game Strcut */
    memset(&gameStruct, 0x00, sizeof(game_struct_t));

    memset(&snakeStruct, 0x00, sizeof(snake_struct_t));


    gameStruct.plot_function  = ht1632c_plot;
    gameStruct.clear_function = ht1632c_clear;
    gameStruct.push_function  = ht1632c_push;
    gameStruct.board_x        = 31;
    gameStruct.board_y        = 15;
    gameStruct.color_depth    = 2;
    
    snakeStruct.cookie         = NULL;
    gameStruct.manhattan_weight    = 1;
    gameStruct.turn_weight         = 1;
    gameStruct.no_turn_weight      = 1;
    gameStruct.look_ahead_distance = 1;
    gameStruct.look_ahead_weight   = 1;


    snakeInit(&gameStruct, &snakeStruct);
    
    snakeStruct.snake_color = 1;
    snakeStruct.apple_color = 2;

    /* Write the header for the .csv file */
    printf("manhattan_weight, turn_weight, no_turn_weight, look_ahead_distance, \
        look_ahead_weight, win_count, loose_count, apple_count \n");
    for(unsigned int manhattan_weight = 1; manhattan_weight < WEIGHT_RANGE; manhattan_weight++){
        for(unsigned int turn_weight = 1; turn_weight < WEIGHT_RANGE; turn_weight++){
            for(unsigned int no_turn_weight = 1; no_turn_weight < WEIGHT_RANGE; no_turn_weight++){
                for(unsigned int look_ahead_distance = 1; look_ahead_distance < WEIGHT_RANGE; look_ahead_distance++){
                    for(unsigned int look_ahead_weight = 1; look_ahead_weight < WEIGHT_RANGE; look_ahead_weight++){

                        win_count   = 0;
                        loose_count = 0;

                        apple_count = 0;

                        gameStruct.manhattan_weight    = manhattan_weight;
                        gameStruct.turn_weight         = turn_weight;
                        gameStruct.no_turn_weight      = no_turn_weight;
                        gameStruct.look_ahead_distance = look_ahead_distance;
                        gameStruct.look_ahead_weight   = look_ahead_weight;

                        /* Try the parameters RUN_MAX Times */
                        for(uint32_t frame = 0; frame < RUN_MAX; frame++){
                            runFrame(&snakeStruct);
                            if(snakeStruct.state == WON){
                                win_count++;
                                apple_count += snakeStruct.apple_count;
                                //sprintf("WON: %d Apples Retrieved", (int)snakeStruct.apple_count);
                            }
                            else if(snakeStruct.state == DEAD){
                                loose_count++;
                                apple_count += snakeStruct.apple_count;
                                //sprintf("LOST: %d Apples Retreived", (int)snakeStruct.apple_count);
                            }
                        }

                        if(apple_count != 0){
                            printf("%u,  %u, %u, %u, %u, %u, %u, %u \n", 
                                manhattan_weight,
                                turn_weight,
                                no_turn_weight,
                                look_ahead_distance,
                                look_ahead_weight,
                                win_count,
                                loose_count,
                                apple_count
                                );
                        }
                    }
                }
            }
        }
    }

}

int ht1632c_plot(void* cookie, uint8_t x, uint8_t y, uint32_t color){
    return 1;
}

int ht1632c_clear(void* cookie){
    return 1;
} 
int ht1632c_push(void * cookie){
    return 1;
}

int snake_print(void * cookie, char *str, const char *format, ...){
    va_list argptr;
    va_start(argptr, format);

    printf(format, argptr);

    va_end(argptr);
    return 1;
}
