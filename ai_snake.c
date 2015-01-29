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

#include "ai_snake.h"
#include <string.h>

// #include "Arduino.h"

static game_struct_t*   gs;
static snake_struct_t*  sns;

void snakeReset();
void runAI();
/*void drawSnake();*/

uint8_t snakeMove(uint8_t turning);
uint8_t checkCollision(uint16_t point);
uint8_t manhattanDistance(uint16_t current, uint16_t target );
uint8_t determineQuadrant(uint16_t packed_head, uint16_t packed_apple);

uint16_t lookAhead(uint8_t turning);
uint16_t makeApple();
uint16_t moveHead(uint16_t head, uint8_t turning, snake_directions_t *final_dir);

void snakeInit( game_struct_t* gameStruct, snake_struct_t* snakeStruct ){
    /* assign inputs to static global pointers */
    gs = gameStruct;
    sns = snakeStruct;

    snakeReset();
}

void snakeReset(){
    /* create an initial snake of length 3 */
    memset(sns->array, 0xFFFF, MAX_LENGTH * sizeof(uint16_t));

    sns->array[0]    = PACK(5, 5);
    sns->array[1]    = PACK(5, 6);
    sns->array[2]    = PACK(5, 7);
    sns->length = 3;

    sns->direction   = MOVING_UP;
    sns->state       = LOOKING_FOR_DIRECTION;

    /* create a new apple */
    sns->apple_pos = makeApple();
#ifdef STATISTICS_ON
        sns->apple_count = 0;
#endif
}

void runFrame(){

    switch(sns->state){
        case RESET:
            snakeReset();
        break;

        case DEAD:
            /* TODO: Add Death Animation here */
            sns->state = RESET;
        break;

        case LOOKING_FOR_DIRECTION:
            if(sns->length >= MAX_LENGTH - 1 ){
                sns->state = WON;
            }
            runAI();
        break;

        case WON:
            sns->state = RESET;
        break;
    }
}

void runAI(){
    uint16_t left_weight = gs->turn_weight;

    uint16_t right_weight = gs->turn_weight;

    uint16_t straight_weight = gs->no_turn_weight;

    uint16_t new_head;
    snake_directions_t new_direction;

    uint16_t old_head = sns->array[sns->length - 1 ];

    //Serial.println("\n ROUND: ");
    /* Test going straight */
    new_head = moveHead(old_head, NO_TURN, &new_direction);
    straight_weight += manhattanDistance(new_head, sns->apple_pos);
    straight_weight += lookAhead(NO_TURN);

    straight_weight = straight_weight * checkCollision(new_head);
    straight_weight = W_C(straight_weight);
    // Serial.print("SW: "); Serial.println(straight_weight);

    /* Test Turning Right */
    new_head = moveHead(old_head, TURN_RIGHT, &new_direction);
    right_weight += manhattanDistance(new_head, sns->apple_pos);
    right_weight += lookAhead(TURN_RIGHT);

    right_weight = right_weight * checkCollision(new_head);
    right_weight = W_C(right_weight);
    // Serial.print("RW: "); Serial.println(right_weight);

    /* Test Turning Left */
    new_head = moveHead(old_head, TURN_LEFT, &new_direction);
    left_weight += manhattanDistance(new_head, sns->apple_pos);
    left_weight += lookAhead(TURN_LEFT);

    left_weight = left_weight * checkCollision(new_head);
    left_weight = W_C(left_weight);
    // Serial.print("LW: "); Serial.println(left_weight);

    /*If There are no moves to make*/
    if(left_weight == 0xFFFF && right_weight == 0xFFFF && straight_weight == 0xFFFF){
        sns->state = DEAD;
        // Serial.println("Snake Dies");
    }

    /* check to see if you can move straight */
    else if(straight_weight <= left_weight && straight_weight <= right_weight) {
        snakeMove(NO_TURN);
        // Serial.println("Choose Straight");
    }

    /* Check to see if you can turn left */
    else if(left_weight <= straight_weight && left_weight <= right_weight){
        snakeMove(TURN_LEFT);
        // Serial.println("Choose LEFT");
    }

    else {
        snakeMove(TURN_RIGHT);
        // Serial.println("Choose RIGHT");
    }
    /* TODO: Finish Here */


}

uint16_t makeApple(){

    uint8_t x_max   = gs->board_x - 1;
    uint8_t y_max   = gs->board_y - 1;
    uint16_t length = sns->length;
    uint16_t* array = sns->array;

    uint8_t appleOk = 0;
    uint16_t tempApple;

    while(!appleOk){
        appleOk = 1;
        tempApple = PACK( (rand() % x_max) , (rand() % y_max));
        for(uint16_t i = 0; i < length; i++){
                if(tempApple == array[i])
                        appleOk = 0;    //if you hit overlap run again
        }
    }
    return tempApple;
}

void drawSnake(){

    uint16_t temp_pixel; 
    uint16_t apple;

    /* Clear the Display */
    gs->clear_function(gs->cookie);
    
    /* Plot the snake's Body */
    for(uint8_t i = 0; i < (sns->length); i++){
        temp_pixel = sns->array[i];
        gs->plot_function(
                        gs->cookie, 
                        GET_X(temp_pixel), 
                        GET_Y(temp_pixel), 
                        sns->snake_color
                        );
    }

    /* Plot the Apple */
    apple = sns->apple_pos;
    gs->plot_function(
                    gs->cookie, 
                    GET_X(apple), 
                    GET_Y(apple), 
                    sns->apple_color
                    );

}

//
//INPUTS: MOVING: UP, DOWN, LEFT, RIGHT
//            TURNING: TURN_RIGHT TURN_LEFT NO_TURN
//

uint8_t snakeMove(uint8_t turning){
    uint16_t new_head;
    snake_directions_t newDir;

    uint16_t head     = sns->array[sns->length - 1];
    uint8_t returnVal = 0;

    new_head = moveHead(head, turning, &newDir);

    /* Update the Head and Direction in the snake Sttruct */
    sns->array[sns->length] = new_head;
    sns->direction          = newDir;

    /* If you didn't hit the apple */
    if(sns->array[sns->length] != sns->apple_pos){
        /* Move the snake down */
        /* TODO: Create a rolling array to hold the snake body */              
        for(uint8_t i = 0; i < (sns->length + 1) ; i++)
            sns->array[i] = sns->array[i+1];
    }

    /* If you've hit the apple */
    /* Increment the length of the snake and make new apple */ 
    else{                                                              
        sns->length++;                                                
        sns->apple_pos = makeApple();  
#ifdef STATISTICS_ON
        sns->apple_count++;
#endif                                     
    }
    return returnVal;
}

/**
 * [checkCollision description]
 * @param  point [description]
 * @return       0 if there is a collision 1 otherwise
 */
uint8_t checkCollision(uint16_t point){
    /* Returns 1 for a collision */
    for(uint16_t i = 0; i < (sns->length); i++){
        if(point == sns->array[i])
            return 0;
        if( GET_X(point) > gs->board_x)
            return 0; 
        if( GET_Y(point) > gs->board_y)
            return 0; 

    }
    return 1;
}

uint16_t moveHead(uint16_t head, uint8_t turning, snake_directions_t *final_dir){
    uint8_t X             = GET_X(head);
    uint8_t Y             = GET_Y(head);
    uint8_t old_direction = sns->direction;
    uint8_t new_direction = (old_direction + turning) % 4;

    switch(new_direction){
        case(MOVING_UP):  
            Y++;  
        break;

        case(MOVING_DOWN):   
            Y--;
        break;

        case(MOVING_LEFT):   
            X--;
        break;

        case(MOVING_RIGHT):
            X++;
        break;

        default:
        /* IF you're not moving do nothing */
        break;
    }  /* END: Switch(direction) */
    *final_dir = (snake_directions_t) new_direction;
    return PACK(X, Y);
}

uint8_t manhattanDistance(uint16_t current, uint16_t target ){
    uint8_t distance;
    uint8_t currentX = GET_X(current);
    uint8_t currentY = GET_Y(current);
    uint8_t targetY  = GET_Y(target);
    uint8_t targetX  = GET_X(target);

    distance = (abs(currentX - targetX) + abs(currentY - targetY));
    distance = gs->manhattan_weight * distance;
    return distance;
}

uint16_t lookAhead(uint8_t turning){
    uint8_t X1, Y1;     /* X1 and Y1 are above */
    uint8_t X2, Y2;     /* X2 and Y2 are bellow*/
    uint16_t clear_count = 0;
    uint16_t temp_point1, temp_point2;

    /* TODO: Optimize this code */
    uint8_t direction = (sns->direction + turning) % 4;
    for(uint8_t i = 0; i <= LOOK_AHEAD_DISTANCE; i++){
        for(uint8_t j = 0; j <= (LOOK_AHEAD_DISTANCE - i); j++){
            X1 = GET_X(sns->array[sns->length]);
            Y1 = GET_Y(sns->array[sns->length]);
            X2 = X1;
            Y2 = X1;

            /*Figure out where points should be*/
            switch(direction){
                case MOVING_RIGHT:
                    X1 += i;
                    X2 += i;

                    Y1 += j;
                    Y2 -= j;
                break;

                case MOVING_LEFT:
                    X1 -= i;
                    X2 -= i;
                  
                    Y1 += j;
                    Y2 -= j;

                break;

                case MOVING_UP:
                    Y1 += i;
                    Y2 += i;
                    
                    X1 += j;
                    X2 -= j;

                break;

                case MOVING_DOWN:
                    Y1 -= i;
                    Y2 -= i;
                    
                    X1 += j;
                    X2 -= j;
                break;

                default:
                    /* You Should never get here */
                break;

            } /* END : switch(Direction) */

            /* Check to see if there is a collison at point 1 */
            temp_point1  = PACK(X1, Y1);
            clear_count += (checkCollision(temp_point1)) ? 0 : gs->look_ahead_weight;

            /* Check to see if there is a collison on point 2 */
            temp_point2 = PACK(X2, Y2);
            clear_count += (checkCollision(temp_point2)) ? 0 : gs->look_ahead_weight;

            /*There higher clear count is, the fewer moves can be */
            /* Made from the point */

        }   /* END: for(j) */
    }       /* END; for(i) */
    return clear_count;
}           /* END: lookAhead(uint8_t turning) */
