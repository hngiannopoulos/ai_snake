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
            runAI();
        break;
    }
}

void runAI(){
    uint8_t left_weight = TURN_WEIGHT;
    //uint16_t left_head;

    uint8_t right_weight = TURN_WEIGHT;
    //uint16_t right_head;

    uint8_t straight_weight = NO_TURN_WEIGHT;
    //uint16_t straight_head;

    uint16_t new_head;
    snake_directions_t new_direction;

    uint16_t old_head = sns->array[sns->length - 1 ];

    //Serial.println("\n ROUND: ");
    /* Test going straight */
    new_head = moveHead(old_head, NO_TURN, &new_direction);
    straight_weight += manhattanDistance(new_head, sns->apple_pos);
    straight_weight = straight_weight * checkCollision(new_head);

    straight_weight = W_C(straight_weight);
    // Serial.print("SW: "); Serial.println(straight_weight);

    /* Test Turning Right */
    new_head = moveHead(old_head, TURN_RIGHT, &new_direction);
    right_weight += manhattanDistance(new_head, sns->apple_pos);
    right_weight = right_weight * checkCollision(new_head);

    right_weight = W_C(right_weight);
    // Serial.print("RW: "); Serial.println(right_weight);

    /* Test Turning Left */
    new_head = moveHead(old_head, TURN_LEFT, &new_direction);
    left_weight += manhattanDistance(new_head, sns->apple_pos);
    left_weight = left_weight * checkCollision(new_head);

    left_weight = W_C(left_weight);
    // Serial.print("LW: "); Serial.println(left_weight);

    /*If There are no moves to make*/
    if(left_weight == 0xFF && right_weight == 0xFF && straight_weight == 0xFF){
        sns->state = DEAD;
        // Serial.println("Snake Dies");
    }

    /* check to see if you can move straight */
    else if(straight_weight <= left_weight && straight_weight <= right_weight) {
        // snakeMove(NO_TURN);
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
                        SNAKE_COLOR
                        );
    }

    /* Plot the Apple */
    apple = sns->apple_pos;
    gs->plot_function(
                    gs->cookie, 
                    GET_X(apple), 
                    GET_Y(apple), 
                    APPLE_COLOR
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
        if( GET_X(point) >= gs->board_x)
            return 0; 
        if( GET_Y(point) >= gs->board_y)
            return 0; 

    }
    return 1;
}

uint16_t moveHead(uint16_t head, uint8_t turning, snake_directions_t *final_dir){
    uint8_t X = GET_X(head);
    uint8_t Y = GET_Y(head);
    switch(sns->direction){
        case(MOVING_UP):   
                if(turning == NO_TURN){
                    Y++;
                    *final_dir = MOVING_UP;
                }
                else if(turning ==TURN_RIGHT){
                    X--;
                    *final_dir = MOVING_RIGHT;
                }
                else if(turning == TURN_LEFT){
                    X++;
                    *final_dir = MOVING_LEFT;
                }
        break;

        case(MOVING_DOWN):   
                if(turning == NO_TURN){
                    Y--;
                    *final_dir = MOVING_DOWN;
                }
                if(turning ==TURN_RIGHT){
                    X++;
                    *final_dir = MOVING_LEFT;   
                }
                if(turning == TURN_LEFT){
                    X--;
                    *final_dir   = MOVING_RIGHT;
                }
        break;

        case(MOVING_LEFT):   
                if(turning == NO_TURN){
                    X++;
                    *final_dir = MOVING_LEFT;
                }
                if(turning ==TURN_RIGHT){
                    Y++;
                    *final_dir = MOVING_UP;
                }
                if(turning == TURN_LEFT){
                    Y--;
                    *final_dir = MOVING_DOWN;
                }
        break;

        case(MOVING_RIGHT):
                if(turning == NO_TURN){
                    X--;
                    *final_dir = MOVING_RIGHT;
                }
                if(turning ==TURN_RIGHT){
                    Y--;
                    *final_dir = MOVING_DOWN;

                }
                if(turning == TURN_LEFT){
                    Y++;
                    *final_dir = MOVING_UP;
                }
        break;

        default:
        /* IF you're not moving do nothing */
        break;
    }  /* END: Switch(direction) */
    return PACK(X, Y);
}

uint8_t manhattanDistance(uint16_t current, uint16_t target ){
    uint8_t distance;
    uint8_t currentX = GET_X(current);
    uint8_t currentY = GET_Y(current);
    uint8_t targetY  = GET_Y(target);
    uint8_t targetX  = GET_X(target);

    distance = (abs(currentX - targetX) + abs(currentY - targetY));
    distance = MANHATTAN_WEIGHT * distance;
    return distance;
}


//////////////////
//OLD FUNCTIONS //
//////////////////


void runFrameOld(){

    uint16_t head              = sns->array[sns->length - 1];
    
    snake_states_t snake_state = sns->state;
    uint8_t gameState          = 0; //1 - if game was lost; 0 - otherwise 
    snake_directions_t snakeDirection = sns->direction;

    if (snake_state == RESET || snake_state == DEAD){
        snakeReset();
    }
     
    /* Basic Pathfinding Algorithm */
    else if (snake_state == LOOKING_FOR_DIRECTION){
        uint8_t quadrant = determineQuadrant(head, sns->apple_pos);

        if(snakeDirection == MOVING_UP){
                if((quadrant == 0) || (quadrant == 1)) 
                    gameState = snakeMove(NO_TURN);
                else if((quadrant == 2))
                    gameState = snakeMove(TURN_LEFT);   
                else if(quadrant == 3)
                    gameState = snakeMove(TURN_RIGHT); 
                else if(quadrant == 5)
                    gameState = snakeMove(TURN_RIGHT); 
                else if(quadrant == 7)
                    gameState = snakeMove(TURN_LEFT);
                else if((quadrant == 6) || (quadrant == 4))
                    gameState = snakeMove(NO_TURN);
        }

        else if(snakeDirection == MOVING_DOWN){
             if((quadrant == 2) || (quadrant == 3)) 
                    gameState = snakeMove(NO_TURN);
                else if((quadrant == 0))
                    gameState = snakeMove(TURN_RIGHT);   
                else if(quadrant == 1)
                    gameState = snakeMove(TURN_LEFT); 
                else if(quadrant == 5)
                    gameState = snakeMove(TURN_LEFT); 
                else if(quadrant == 7)
                    gameState = snakeMove(TURN_RIGHT);
                else if((quadrant == 6) || (quadrant == 4))
                    gameState = snakeMove(NO_TURN);
        }

        else if(snakeDirection == MOVING_LEFT){
            if((quadrant == 0) || (quadrant == 1)) 
                gameState = snakeMove(TURN_RIGHT);
            else if((quadrant == 2) || (quadrant == 3))
                gameState = snakeMove(TURN_LEFT);   
            else if(quadrant == 6)
                gameState = snakeMove(TURN_LEFT); 
            else if(quadrant == 4)
                gameState = snakeMove(TURN_RIGHT); 
            else
                gameState = snakeMove(NO_TURN);    

        }

        /* snakeDirection == MOVING_RIGHT */
        else{ 
            if((quadrant == 0) || (quadrant == 1)) 
                gameState = snakeMove(TURN_LEFT);
            else if((quadrant == 2) || (quadrant == 3))
                gameState = snakeMove(TURN_RIGHT);   
            else if(quadrant == 6)
                gameState = snakeMove(TURN_RIGHT); 
            else if(quadrant == 4)
                gameState = snakeMove(TURN_LEFT); 
            else 
                gameState = snakeMove(NO_TURN);    

        }

    /* Deal with a won or lost game */
    if((gameState == 1))
        sns->state = DEAD;
    else
        sns->state = LOOKING_FOR_DIRECTION;

    drawSnake();

    }   /* END: LOOKING_FOR_DIRECTION */
}       /* END : runFrame() */

/* 0 | 1   on lines 4(V), 5
 * 2 | 3   relative to the apple location
 * 
 * */
uint8_t determineQuadrant(uint16_t packed_head, uint16_t packed_apple){                      
    uint8_t quadrant = 0;
    uint8_t headX    = GET_X(packed_head);
    uint8_t headY    = GET_Y(packed_head);
    uint8_t appleX   = GET_X(packed_apple);
    uint8_t appleY   = GET_Y(packed_apple);

    /* Check to see if you actually hit the apple */
    if((headX == appleX) && (headY == appleY))                     
        return 8;

    /* Check to see if you've hit a vertical */
    /* Then determine if you are above or below the apple */
    if(headX == appleX) 
        return (appleY > headY) ? 4 : 6;

    /* Check to see if you've hit a horizontal */
    /* Then determine if you are to the left or right of the apple */
    if(headY == appleY)                    
        return (appleX > headX) ? 7 : 5;

    /* Determine regular quadrant */
    /* If You're bellow the apple... */
    if(headY > appleY)          
        quadrant += 2;  
    /* If you're to the right of the apple */        
    if(headX > appleX)          
        quadrant += 1;  

    return quadrant;
}









