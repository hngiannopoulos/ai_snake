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

/*#include "Arduino.h"*/

static game_struct_t*   gs;
static snake_struct_t*  sns;

void snakeReset();
void drawSnake();

uint8_t snakeMove(uint8_t turning);
uint8_t checkCollision(uint16_t point);
uint8_t determineQuadrant(uint16_t packed_head, uint16_t packed_apple);

uint16_t makeApple();

void snakeInit( game_struct_t* gameStruct, snake_struct_t* snakeStruct ){
    /* assign inputs to static global pointers */
    gs = gameStruct;
    sns = snakeStruct;

    snakeReset();
    
}

void snakeReset(){
    /* create an initial snake of length 3 */
    memset(sns->array, 0xFF, MAX_LENGTH * sizeof(uint16_t));

    sns->array[0]    = WRAP(5, 5);
    sns->array[1]    = WRAP(5, 6);
    sns->array[2]    = WRAP(5, 7);
    sns->length = 3;

    sns->direction   = MOVING_UP;
    sns->state       = LOOKING_FOR_DIRECTION;

    /* create a new apple */
    sns->apple_pos = makeApple();

}

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

void runFrame(){

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
                    gameState = snakeMove(noTurn);
                else if((quadrant == 2))
                    gameState = snakeMove(turnLeft);   
                else if(quadrant == 3)
                    gameState = snakeMove(turnRight); 
                else if(quadrant == 5)
                    gameState = snakeMove(turnRight); 
                else if(quadrant == 7)
                    gameState = snakeMove(turnLeft);
                else if((quadrant == 6) || (quadrant == 4))
                    gameState = snakeMove(noTurn);
        }

        else if(snakeDirection == MOVING_DOWN){
             if((quadrant == 2) || (quadrant == 3)) 
                    gameState = snakeMove(noTurn);
                else if((quadrant == 0))
                    gameState = snakeMove(turnRight);   
                else if(quadrant == 1)
                    gameState = snakeMove(turnLeft); 
                else if(quadrant == 5)
                    gameState = snakeMove(turnLeft); 
                else if(quadrant == 7)
                    gameState = snakeMove(turnRight);
                else if((quadrant == 6) || (quadrant == 4))
                    gameState = snakeMove(noTurn);
        }

        else if(snakeDirection == MOVING_LEFT){
            if((quadrant == 0) || (quadrant == 1)) 
                gameState = snakeMove(turnRight);
            else if((quadrant == 2) || (quadrant == 3))
                gameState = snakeMove(turnLeft);   
            else if(quadrant == 6)
                gameState = snakeMove(turnLeft); 
            else if(quadrant == 4)
                gameState = snakeMove(turnRight); 
            else
                gameState = snakeMove(noTurn);    

        }

        /* snakeDirection == MOVING_RIGHT */
        else{ 
            if((quadrant == 0) || (quadrant == 1)) 
                gameState = snakeMove(turnLeft);
            else if((quadrant == 2) || (quadrant == 3))
                gameState = snakeMove(turnRight);   
            else if(quadrant == 6)
                gameState = snakeMove(turnRight); 
            else if(quadrant == 4)
                gameState = snakeMove(turnLeft); 
            else 
                gameState = snakeMove(noTurn);    

        }

    /* Deal with a won or lost game */
    if((gameState == 1))
        sns->state = DEAD;
    else
        sns->state = LOOKING_FOR_DIRECTION;

    drawSnake();

    }   /* END: LOOKING_FOR_DIRECTION */
}       /* END : runFrame() */

//
//INPUTS: MOVING: UP, DOWN, LEFT, RIGHT
//            TURNING: turnRight turnLeft noTurn
//
//RETURNS 1 if the snake hits itself
//            0 if the snake is ok
uint8_t snakeMove(uint8_t turning){                
    uint16_t head              = sns->array[sns->length - 1];
    /*uint8_t findMove = 1;*/
    uint8_t X, Y;
    snake_directions_t tempDir = sns->direction;
    uint8_t moveOk = 0;
    uint8_t returnVal = 0;
    //some sort of fucking table 
    //modifies snake head 
    while(moveOk < 5){
        X = GET_X(head);
        Y = GET_Y(head);
        switch(sns->direction){
            case(MOVING_UP):   
                    if(turning == noTurn){
                        Y++;
                        tempDir = MOVING_UP;
                    }
                    else if(turning ==turnRight){
                        X--;
                        tempDir = MOVING_RIGHT;
                    }
                    else if(turning == turnLeft){
                        X++;
                        tempDir = MOVING_LEFT;
                    }
            break;

            case(MOVING_DOWN):   
                    if(turning == noTurn){
                        Y--;
                        tempDir = MOVING_DOWN;
                    }
                    if(turning ==turnRight){
                        X++;
                        tempDir = MOVING_LEFT;   
                    }
                    if(turning == turnLeft){
                        X--;
                        tempDir   = MOVING_RIGHT;
                    }
            break;

            case(MOVING_LEFT):   
                    if(turning == noTurn){
                        X++;
                        tempDir = MOVING_LEFT;
                    }
                    if(turning ==turnRight){
                        Y++;
                        tempDir = MOVING_UP;
                    }
                    if(turning == turnLeft){
                        Y--;
                        tempDir = MOVING_DOWN;
                    }
            break;

            case(MOVING_RIGHT):
                    if(turning == noTurn){
                        X--;
                        tempDir = MOVING_RIGHT;
                    }
                    if(turning ==turnRight){
                        Y--;
                        tempDir = MOVING_DOWN;

                    }
                    if(turning == turnLeft){
                        Y++;
                        tempDir = MOVING_UP;
                    }
            break;
            
            default:
            /* if you get here theres a lot of problems, keep the head */
            break;

        }  /* END: Switch(sns->direction) */
        

        /* If you're going to hit yourself, try turning in another direction */
        if(checkCollision(PACK(X, Y)) == 1){        
            moveOk++;    //add 1 to the try count

            /* If you can't turn, try to go straight */
            if((turning != noTurn) && (moveOk < 2))   
                turning = noTurn;

            /* Otherwise try to turn in another direction */
            else                         
                turning = (moveOk % 2) ? turnLeft : turnRight;
         
            returnVal = 1; 
        }
        
        else{
            moveOk = 10;      //
            sns->direction = tempDir;
        }

    }   /* END: while(moveOK < 5) */

    //assign new head with modified X and Y values 
    sns->array[sns->length] = PACK(X, Y);

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


uint8_t checkCollision(uint16_t point){
    /* Returns 1 for a collision */
    for(uint16_t i = 0; i < (sns->length); i++){
        if(point == sns->array[i])
            return 1;
    }
    return 0;
}













