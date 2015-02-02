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
// static snake_struct_t*  sns;

void _snake_reset(snake_struct_t* sns);
void _run_ai(snake_struct_t* sns);
/*void drawSnake();*/

uint8_t _snake_move(snake_struct_t * sns, uint8_t turning);
uint8_t _check_collision(snake_struct_t * sns, uint16_t point);
uint8_t _manhattan_dist(uint16_t current, uint16_t target );
uint8_t _lowest_weight(uint16_t straight, uint16_t right, uint16_t left );
uint8_t _check_optimal_path(snake_struct_t* sns,
                            uint16_t head, 
                            snake_directions_t start_dir, 
                            uint16_t apple);

uint16_t _look_ahead(snake_struct_t * sns, uint16_t head, snake_directions_t dir, uint8_t turning);
void _make_apple(snake_struct_t * sns);
uint16_t _move_head(uint16_t head, 
                    uint8_t turning, 
                    snake_directions_t start_dir,
                    snake_directions_t *finish_dir);

snake_directions_t _turn_direction(uint8_t turning, 
                            snake_directions_t direction);

void snakeInit( game_struct_t* gs, snake_struct_t* sns ){
    /* assign inputs to static global pointers */
    sns->gs = gs;

    _snake_reset(sns);
}

/* TODO: Make runFrame take in a snakeStruct */
void runFrame(snake_struct_t* sns){

    switch(sns->state){
        case RESET:
            _snake_reset(sns);
        break;

        case DEAD:
            /* TODO: Add Death Animation here */
            sns->state = RESET;
        break;

        case LOOKING_FOR_DIRECTION:
            if(sns->length >= MAX_LENGTH - 1 ){
                sns->state = WON;
            }
            _run_ai(sns);
        break;

        case WON:
            sns->state = RESET;
        break;
    }
}

void drawSnake(snake_struct_t * sns){

    uint16_t temp_pixel; 
    uint16_t apple;

    /* Clear the Display */
    sns->gs->clear_function(sns->cookie);
    
    /* Plot the snake's Body */
    for(uint8_t i = 0; i < (sns->length); i++){
        temp_pixel = sns->array[i];
        sns->gs->plot_function(
                        sns->cookie, 
                        GET_X(temp_pixel), 
                        GET_Y(temp_pixel), 
                        sns->snake_color
                        );
    }

    /* Plot the Apple */
    apple = sns->apple_pos;
    sns->gs->plot_function(
                    sns->cookie, 
                    GET_X(apple), 
                    GET_Y(apple), 
                    sns->apple_color
                    );

}

void _snake_reset(snake_struct_t* sns){
    /* create an initial snake of length 3 */
    memset(sns->array, 0xFFFF, MAX_LENGTH * sizeof(uint16_t));

    sns->array[0]    = PACK(5, 5);
    sns->array[1]    = PACK(5, 6);
    sns->array[2]    = PACK(5, 7);
    sns->length = 3;

    sns->direction   = MOVING_UP;
    sns->state       = LOOKING_FOR_DIRECTION;

    /* create a new apple */
    _make_apple(sns);
#ifdef STATISTICS_ON
    sns->apple_count = 0;
#endif
}

/* todo, make snake take in snakeStruct */ 
void _run_ai(snake_struct_t* sns){
    uint16_t left_weight = sns->gs->turn_weight;

    uint16_t right_weight = sns->gs->turn_weight;

    uint16_t straight_weight = gs->no_turn_weight;

    uint16_t new_head;
    uint8_t best_move;

    snake_directions_t current_direction = sns->direction;
    snake_directions_t new_direction;

    uint16_t old_head = sns->array[sns->length - 1 ];

    /* Test going straight */
    new_head = _move_head(old_head, 
                            NO_TURN, 
                            current_direction, 
                            &new_direction);

    straight_weight += _manhattan_dist(new_head, sns->apple_pos);
    straight_weight += _look_ahead(sns, new_head, new_direction, NO_TURN);
    straight_weight = straight_weight * _check_collision(sns, new_head);


    /* Test Turning Right */
    new_head = _move_head(old_head, 
                            TURN_RIGHT, 
                            current_direction, 
                            &new_direction);

    right_weight += _manhattan_dist(new_head, sns->apple_pos);
    right_weight += _look_ahead(sns, new_head, new_direction, TURN_RIGHT);
    right_weight = right_weight * _check_collision(sns, new_head);

    /* Test Turning Left */
    new_head = _move_head(old_head, 
                            TURN_LEFT, 
                            current_direction, 
                            &new_direction);

    left_weight += _manhattan_dist(new_head, sns->apple_pos);
    left_weight += _look_ahead(sns, new_head, new_direction, TURN_LEFT);
    left_weight = left_weight * _check_collision(sns, new_head);

    /* Determing the move with the lowest weight */
    best_move = _lowest_weight(straight_weight,
                                right_weight,
                                left_weight
                                );

    if(best_move == TURN_ERROR){
        sns->state = DEAD;
    }
    else {
        _snake_move(sns, best_move);
    }
}

void _make_apple(snake_struct_t* sns){

    uint8_t x_max   = sns->gs->board_x - 1;
    uint8_t y_max   = sns->gs->board_y - 1;
    uint16_t length = sns->length;
    uint16_t* array = sns->array;

    uint8_t appleOk = 0;
    uint16_t tempApple;

    /* TODO: Use _check_collision instead of this */
    while(!appleOk){
        appleOk = 1;
        tempApple = PACK( (rand() % x_max) , (rand() % y_max));
        for(uint16_t i = 0; i < length; i++){
                if(tempApple == array[i])
                        appleOk = 0;    //if you hit overlap run again
        }
    }
    sns->apple_pos = tempApple;
}

uint8_t _snake_move(snake_struct_t* sns, uint8_t turning){
    uint16_t new_head;

    uint16_t head     = sns->array[sns->length - 1];
    uint8_t returnVal = 0;

    new_head = _move_head(head, turning, sns->direction, &sns->direction);

    /* Update the Head and Direction in the snake Sttruct */
    sns->array[sns->length] = new_head;

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
        _make_apple(sns);  
#ifdef STATISTICS_ON
        sns->apple_count++;
#endif                                     
    }
    return returnVal;
}

/**
 * [_check_collision description]
 * @param  point [description]
 * @return       0 if there is a collision 1 otherwise
 */
uint8_t _check_collision(snake_struct_t* sns, uint16_t point){
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

uint16_t _move_head(uint16_t head, 
                    uint8_t turning, 
                    snake_directions_t start_dir,
                    snake_directions_t *finish_dir)
{

    uint8_t X             = GET_X(head);
    uint8_t Y             = GET_Y(head);

    *finish_dir = _turn_direction(turning, start_dir);

    switch(*finish_dir){
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
    return PACK(X, Y);
}

uint8_t _manhattan_dist(uint16_t current, uint16_t target ){
    uint8_t distance;
    uint8_t currentX = GET_X(current);
    uint8_t currentY = GET_Y(current);
    uint8_t targetY  = GET_Y(target);
    uint8_t targetX  = GET_X(target);

    distance = (abs(currentX - targetX) + abs(currentY - targetY));
    distance = gs->manhattan_weight * distance;
    return distance;
}

uint16_t _look_ahead(snake_struct_t* sns, uint16_t head, snake_directions_t dir, uint8_t turning){
    uint8_t X1, Y1;     /* X1 and Y1 are above */
    uint8_t X2, Y2;     /* X2 and Y2 are bellow*/
    uint16_t temp_point1, temp_point2;
    
    uint8_t distance     = gs->look_ahead_distance;
    uint16_t clear_count = 0;
    snake_directions_t direction = _turn_direction(turning, dir);

    /* TODO: Optimize this code */
    for(uint8_t i = 0; i <= distance; i++){
        for(uint8_t j = 0; j <= (distance - i); j++){
            X1 = GET_X(head);
            Y1 = GET_Y(head);
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

            /* Check to see if there is a collision at point 1 */
            temp_point1  = PACK(X1, Y1);
            clear_count += _check_optimal_path(sns, head, direction, temp_point1);
            gs->print_function(NULL, "OUT of A\r\n");

            /* Check to see if there is a collision on point 2 */
            temp_point2 = PACK(X2, Y2);
            clear_count += _check_optimal_path(sns, head, direction, temp_point2);
            gs->print_function(NULL, "OUT of B\r\n");

        }   /* END: for(j) */
    }       /* END; for(i) */
    return clear_count;
}           /* END: _look_ahead(uint8_t turning) */


/* Returns gs->look_ahead_weight if the optimal path is not reachable */
uint8_t _check_optimal_path(snake_struct_t * sns,
                            uint16_t head, 
                            snake_directions_t start_dir, 
                            uint16_t apple)
{
    uint16_t straight_weight;
    uint16_t left_weight;
    uint16_t right_weight;
    uint16_t temp_head;
    uint8_t best_move;
    snake_directions_t temp_dir;

    /* keep running until you hit the apple */
    while(_manhattan_dist(head, apple)){

        /* Check Straight */
        temp_head = _move_head(head, NO_TURN, start_dir, &temp_dir );
        straight_weight = _manhattan_dist(temp_head, apple);

        /* Check Turn Right */
        temp_head = _move_head(head, TURN_RIGHT, start_dir, &temp_dir);
        right_weight = _manhattan_dist(temp_head, apple);

        /* Check Turn Left */
        temp_head = _move_head(head, TURN_LEFT, start_dir, &temp_dir);
        left_weight = _manhattan_dist(temp_head, apple);

        /* Process Weights and determine best direction */ 
        best_move = _lowest_weight(straight_weight,
                                right_weight,
                                left_weight);

        head = _move_head(head, best_move,  start_dir, &start_dir);
        
        /* If there was a collision return 0 */
        if(!_check_collision(sns, head)){
            gs->print_function(NULL, "NO Optimal Path\r\n");
            return gs->look_ahead_weight;
        }
    }
    gs->print_function(NULL, "Found Optimal Path\r\n");
    return 0;
}

/* Any weight of 0 is invalid */
uint8_t _lowest_weight(uint16_t straight, uint16_t right, uint16_t left ){
    
    straight = W_C(straight);
    right    = W_C(right);
    left     = W_C(left);

    /*If There are no moves to make*/
    if(straight == 0xFFFF && right == 0xFFFF && left == 0xFFFF){
        return TURN_ERROR;
    }

    /* check to see if you can move straight */
    else if(straight <= left && straight <= right) {
        return NO_TURN;
    }

    /* Check to see if you can turn left */
    else if(left <= straight && left <= right){
        return TURN_LEFT;
    }

    else {
        return TURN_RIGHT;
    }
}

snake_directions_t _turn_direction(uint8_t turning, 
                            snake_directions_t direction)
{
    return (snake_directions_t)((direction + turning) % 4);
}