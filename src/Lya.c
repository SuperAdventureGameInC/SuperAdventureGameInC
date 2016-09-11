// Compilation command:
// gcc Lya.c -o Lya.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

/* Naming conventions
  All upper case separated by underscore: #define (example: SCREEN_WIDTH)
  "g" followed by camel case: global variable (example: gNumberOfProjectiles)
  Lower case verb followed by camel case: function (example: moveProjectiles)
  Camel case: local variable (example: ProjectileType)
  Single lower case letter: local variable used as counter or loop index (example: i)
*/

//#define AUDIO 1

#ifdef AUDIO
#include "Audio.h"
#endif

#include "Map.h"
#include "Entity.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 576
#define BOUNDARY_OFFSET 100//boundary offset for player position in tenths of pixel
#define HALF_SIDE_PLAYER_COLLISION_BOX 100//in tenths of pixel, must be smaller than half side of tile
#define LINE_HEIGHT 9
#define MAX_ROW 100//Needs to be greater or equal to 18 (SCREEN_HEIGHT/32)
#define MAX_COLUMN 100//Needs to be greater or equal to 25 (SCREEN_WIDTH/32)
#define MAX_LAYER 4
#define DELAY_BETWEEN_FRAMES 20//Delay between frames in millisecond
// #define PLAYER_SPEED 60//player speed in tenths of pixels per frame = speed [pixels per second] * delay between frames [millisecond] / 100
#define MAX_PROJECTILE 1000//maximum number of projectiles
#define PROJECTILE_INFO 10//number of information stored per projectile
#define MAX_ENEMY 100//maximum number of enemies
#define ENEMY_INFO 8//number of information stored per enemy
#define NUMBER_OF_SPELLS 6//number of spells in the game
#define MAX_INSTRUCTIONS 10//number of move instructions for each enemy

struct Entity gPlayer;

//const int gPlayerSpeedDiagonal=PLAYER_SPEED*71/100;
int gTextX,gTextY;//(x,y) coordinate of upper left corner of text to render
// int gPlayerX,gPlayerY;//10 times the (x,y) coordinate of upper left corner of the player sprite
int gPlayerOrientation;//player orientation: 0 = UP or UP_LEFT, 1 = LEFT or DOWN_LEFT, 2 = DOWN or DOWN_RIGHT, 3 = RIGHT or UP_RIGHT
int gPlayerHP,gPlayerMaxHP;
int gPlayerSkipFrames;//To blink if the player get hit
int gNumberOfProjectiles;
int gProjectileList[MAX_PROJECTILE][PROJECTILE_INFO];//Each row of gProjectileList contains the following information about a projectile: PositionX*10, PositionY*10, VelocityX*10, VelocityY*10, TravelSteps, TargetX, TargetY, Direction (0 to 15), ProjectileType (0 to NUMBER_OF_SPELLS - 1 for spells, NUMBER_OF_SPELLS and above will serve for other projectiles), Caster (0=player,1=enemy)
int gNumberOfEnemies;
int gEnemyList[MAX_ENEMY][ENEMY_INFO+1+3*MAX_INSTRUCTIONS]={0};//Each row of gEnemyList contains the following information about a enemy: PositionX*10, PositionY*10, EnemyType, HP, Frame, Orientation, Cooldown, SkipFrame, current instruction index, index of the "when done" instruction, number of steps to do in current instruction, instruction 1 delta X, instruction 1 delta Y, instruction 1 number of steps, instruction 2 delta X, instruction 2 delta Y, instruction 2 number of steps, ...
const int gPlayerDirection[16]={3,3,3,0,0,0,0,1,1,1,1,2,2,2,2,3};//gPlayerDirection[ProjectileDirection] is the player direction when casting a spell in ProjectileDirection
const int gTileType[3969]={0,//gTileType[TileIndex] is the TileType associated with tile of index TileIndex. TileType are 0 = no title, 1 = overlay to render above entities and projectiles, 2 = walkable tile, 3 = obstacle to movement, 4 = obstacle to movement and projectiles
2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,2,1,3,3,3,2,2,1,3,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,1,3,3,3,0,0,1,3,1,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,3,2,2,2,2,3,3,2,2,3,2,2,2,2,2,2,2,3,3,3,3,3,3,2,2,3,3,3,3,3,3,3,1,3,2,2,2,3,3,3,2,3,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,3,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,3,3,3,3,1,3,3,3,3,3,3,3,3,3,1,3,3,2,2,2,3,3,3,2,2,3,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,3,3,2,3,2,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,2,3,3,3,3,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,2,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,2,2,2,2,2,2,3,3,3,3,2,2,3,3,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,2,2,2,2,3,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,4,4,3,2,2,2,2,2,2,2,2,1,1,4,4,2,2,2,2,2,2,2,2,3,3,2,4,4,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,4,4,3,4,4,4,4,2,2,2,2,1,1,1,1,1,2,2,2,2,2,2,2,4,4,2,3,3,3,3,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,2,3,3,4,3,2,4,4,2,2,2,2,2,2,2,2,2,2,2,4,4,1,1,1,2,2,2,2,2,2,2,4,4,2,2,2,2,2,2,2,2,2,4,1,1,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,4,4,2,2,2,1,1,1,1,3,2,2,3,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,1,1,1,1,3,2,2,3,2,2,2,2,2,2,2,2,2,2,4,4,
2,2,2,2,2,2,2,4,4,2,2,2,4,1,1,1,2,2,2,2,1,1,4,4,4,2,2,2,2,2,2,2,3,2,2,2,2,2,2,2,2,1,1,4,1,1,1,1,2,2,2,2,1,1,1,2,2,1,1,1,2,1,2,2,
3,3,3,2,2,2,2,3,2,2,2,2,2,1,1,1,3,3,3,3,1,1,1,1,1,1,1,4,4,2,4,4,3,2,3,3,2,3,3,2,2,3,3,1,3,3,3,3,3,2,2,3,1,1,1,1,1,1,1,4,1,4,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,3,3,3,3,4,4,4,4,4,1,1,4,4,2,4,4,2,2,2,2,2,2,3,1,1,3,1,1,1,1,1,1,3,3,3,2,4,4,4,1,1,4,4,1,4,1,4,4,
3,2,2,2,2,2,2,3,0,3,3,3,2,4,4,4,3,3,3,3,4,4,4,4,4,4,4,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,3,3,3,2,3,3,2,4,4,4,4,4,4,4,4,3,4,4,4,

2,2,2,2,2,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,1,1,4,4,4,1,1,1,2,2,1,2,2,1,1,2,2,2,2,
2,2,2,2,2,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,4,1,4,4,4,1,4,1,2,1,4,1,2,4,4,2,2,2,2,
2,2,2,2,2,3,3,3,2,2,2,2,3,2,2,2,4,4,4,4,4,4,4,4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,1,1,4,4,4,1,1,2,1,4,4,4,1,4,4,2,2,2,2,
2,2,2,3,3,3,3,3,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,1,1,1,1,1,4,1,1,1,1,1,4,1,1,4,2,4,1,4,4,2,2,2,2,
2,2,2,3,3,3,3,3,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,4,4,4,4,1,1,2,1,1,4,4,2,2,2,2,
2,2,2,3,3,3,3,3,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,4,1,4,4,4,1,1,2,1,1,1,1,2,2,2,2,
4,4,4,4,2,2,3,3,2,2,2,2,3,3,2,2,4,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,1,2,1,1,1,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,3,3,2,2,4,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,1,4,4,4,1,1,1,2,2,1,1,1,1,1,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,4,1,4,4,4,1,4,1,2,1,4,1,2,4,4,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,1,1,4,4,4,1,1,2,1,4,4,4,1,4,4,2,2,2,2,
1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,1,4,1,1,1,1,1,4,1,1,4,2,4,1,4,4,2,2,2,2,
1,4,4,4,4,4,4,4,4,1,1,1,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,4,4,4,4,1,4,4,4,4,1,1,2,2,2,4,4,2,2,2,2,
1,4,4,4,4,4,4,4,4,1,4,4,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,4,4,4,1,4,1,4,4,4,1,1,2,2,2,1,1,2,2,2,2,
2,4,4,4,4,4,4,4,4,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,4,2,4,4,4,4,4,4,4,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,1,1,2,2,2,2,2,2,2,2,2,
2,4,4,4,4,4,4,4,4,2,1,1,1,4,4,1,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,4,4,0,4,4,0,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,

3,3,3,3,3,3,3,2,4,4,4,4,2,4,4,4,4,2,3,3,3,1,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,2,3,3,2,3,3,3,4,4,4,4,4,4,4,4,4,2,3,3,3,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,2,3,3,3,2,2,4,4,4,4,4,4,4,2,3,1,1,1,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,3,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,2,2,3,2,2,3,3,2,2,2,2,3,3,2,2,2,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,3,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,3,3,3,2,3,2,2,4,2,4,2,4,1,3,2,2,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,3,2,3,2,4,4,4,4,4,2,2,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,4,4,2,4,4,4,4,4,4,2,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,4,3,3,4,4,2,2,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,3,4,2,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,3,3,2,2,4,4,2,2,3,2,3,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,3,2,2,2,3,2,2,2,3,3,3,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,3,3,2,2,2,2,2,2,2,2,3,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,2,3,3,3,2,4,2,3,3,3,3,2,2,2,3,3,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,3,3,3,2,3,3,3,3,3,2,2,2,3,3,4,4,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,1,1,1,1,1,1,2,2,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,3,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,3,2,2,2,2,2,2,2,2,2,2,2,2,1,1,3,3,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
4,4,4,4,4,4,2,2,4,2,3,2,2,2,2,3,3,3,3,2,3,3,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,2,1,1,1,1,1,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
4,4,4,4,4,4,4,4,4,2,3,2,2,2,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
4,4,4,4,4,4,4,4,4,3,3,4,4,3,3,3,3,3,3,3,3,2,3,3,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,4,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
4,4,4,2,2,2,2,2,4,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,2,2,2,2,2,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,
4,4,4,4,4,4,2,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,2,2,4,2,4,4,1,1,4,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,4,2,2,4,4,3,3,3,3,3,3,3,3,2,2,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
3,3,4,2,4,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,2,2,2,2,2,2,2,2,2,2,2,2,4,1,2,2,2,2,2,2,2,2,2,2,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
3,3,4,2,4,3,3,3,2,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,4,1,2,2,2,2,2,2,4,3,4,2,4,1,1,4,2,2,2,2,2,2,2,2,2,2,2,
2,3,4,3,4,3,3,3,4,4,3,3,3,3,3,2,3,4,4,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
3,3,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,2,2,2,2,2,2,2,2,2,3,4,4,4,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,1,2,2,2,2,2,2,2,3,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
int gMapMovementObstacle[MAX_ROW][MAX_COLUMN];//gMapMovementObstacle[i][j] = 1 if location(i,j) is a movement obstacle, = 0 otherwise
int gMapProjectileObstacle[MAX_ROW][MAX_COLUMN];//gMapProjectileObstacle[i][j] = 1 if location(i,j) is a projectile obstacle, = 0 otherwise
int gHasSpell[NUMBER_OF_SPELLS]={0};//Spells available to the player
int gSpellStock[NUMBER_OF_SPELLS]={0};//Number of spells the player has in stock for each spell
int gMaxSpellStock[NUMBER_OF_SPELLS]={200,400,200,200,200,200};//Maximum stock for each spell
int gProjectileSpeed[NUMBER_OF_SPELLS+1]={120,180,100,180,150,170,100};//speed in tenths of pixels per frame
int gProjectileCooldown[NUMBER_OF_SPELLS+1]={8,5,12,15,10,60,20};//Projectile cooldown in frames
SDL_Window *gWindow=NULL;//The window we'll be rendering to
SDL_Renderer *gRenderer=NULL;//The window renderer
SDL_Texture *gTextureTiles=NULL;//Tile sheet texture
//SDL_Texture *gTexturePlayer=NULL;//Player sheet texture
SDL_Texture *gTextureEnemy=NULL;//Enemy sheet texture
SDL_Texture *gTextureProjectiles=NULL;//Projectiles sheet texture
SDL_Texture *gTextureLife=NULL;//Life meter texture
SDL_Texture *gTextureTitle=NULL;
SDL_Texture *gTextureGameOver=NULL;
SDL_Texture *gTextureFont=NULL;//Font sheet texture

#ifdef AUDIO
struct Sound *gSoundFire = NULL;
struct Sound *gSoundIce = NULL;
#endif

enum EnumSpellType {
  eFire,
  ePlasma,
  eIce,
  ePurple,
  eElectric,
  eGreen,
  eNumSpellTypes
};

int initializeSDL(void){//Starts up SDL and creates window
  if(SDL_Init(SDL_INIT_VIDEO)<0){//Initialize SDL
    printf("SDL could not initialize! SDL Error: %s\n",SDL_GetError());
    return 1;
  }
  else{//Set texture filtering to linear
    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1")){
      printf("Warning: Linear texture filtering not enabled!");
    }
    gWindow=SDL_CreateWindow("Wizard apprentice Lya",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);//Create window
    if(gWindow==NULL){
      printf("Window could not be created! SDL Error: %s\n",SDL_GetError());
      return 1;
    }
    else{//Create renderer for window
      gRenderer=SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
      if(gRenderer==NULL){
        printf("Renderer could not be created! SDL Error: %s\n",SDL_GetError());
        return 1;
      }
      else{//Initialize renderer color
        SDL_SetRenderDrawColor(gRenderer,0,0,0,255);
        int imgFlags=IMG_INIT_PNG;//Initialize PNG loading
        if( !( IMG_Init(imgFlags) & imgFlags ) ){
          printf("SDL_image could not initialize! SDL_image Error: %s\n",IMG_GetError());
          return 1;
        }
      }
    }
  }

  return 0;
}

int loadTiles(void){
  SDL_Surface *loadedSurface=IMG_Load("res/Tiles.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","Tiles.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureTiles=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureTiles==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","Tiles.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureTiles==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadEnemy(void){
  SDL_Surface *loadedSurface=IMG_Load("res/SkeletonArcher.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","SkeletonArcher.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureEnemy=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureEnemy==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","SkeletonArcher.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureEnemy==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadProjectiles(void){
  SDL_Surface *loadedSurface=IMG_Load("res/Projectiles.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","Projectiles.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureProjectiles=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureProjectiles==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","Projectiles.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureProjectiles==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadLife(void){
  SDL_Surface *loadedSurface=IMG_Load("res/Life.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","Life.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureLife=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureLife==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","Life.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureLife==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadTitle(void){
  SDL_Surface *loadedSurface=IMG_Load("res/Title.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","Title.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureTitle=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureTitle==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","Title.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureTitle==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadGameOver(void){
  SDL_Surface *loadedSurface=IMG_Load("res/GameOver.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","GameOver.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTextureGameOver=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureGameOver==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","GameOver.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureGameOver==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

int loadFont(void){
  SDL_Surface *loadedSurface=SDL_LoadBMP("res/font.bmp");
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL Error: %s\n","font.bmp",SDL_GetError());
    return 1;
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,0,0));//Set the transparent pixel to black
    gTextureFont=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTextureFont==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","font.bmp",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTextureFont==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

#ifdef AUDIO
int loadAudio(struct AudioCtx *ctx){
  gSoundFire = loadSound(ctx, "res/fire.opus");
  if(!gSoundFire){
    puts("Could not load fire sound file");
    return 1;
  }

  gSoundIce = loadSound(ctx, "res/ice.opus");
  if(!gSoundFire){
    puts("Could not load fire sound file");
    return 1;
  }

  return 0;  
}
#endif

void closeSDL(void){//Frees media and shuts down SDL
  SDL_DestroyTexture(gTextureTiles);gTextureTiles=NULL;//Free loaded tile
//  SDL_DestroyTexture(gTexturePlayer);gTexturePlayer=NULL;//Free loaded player
  SDL_DestroyTexture(gTextureEnemy);gTextureEnemy=NULL;//Free loaded enemy
  SDL_DestroyTexture(gTextureProjectiles);gTextureProjectiles=NULL;//Free loaded projectiles
  SDL_DestroyTexture(gTextureLife);gTextureLife=NULL;//Free loaded life
  SDL_DestroyTexture(gTextureTitle);gTextureTitle=NULL;//Free loaded title
  SDL_DestroyTexture(gTextureGameOver);gTextureGameOver=NULL;//Free loaded GameOver
  SDL_DestroyTexture(gTextureFont);gTextureFont=NULL;//Free loaded font
  
  //Destroy window
  SDL_DestroyRenderer(gRenderer);gRenderer=NULL;
  SDL_DestroyWindow(gWindow);gWindow=NULL;

  IMG_Quit();SDL_Quit();//Quit SDL subsystems
}


int renderDigit(int X,int Y,int Digit){
  /*
  Renders a single digit using (X,Y) as upper left corner.
  Return int value:
    0 on success
    1 if the input has more than one digit
    
  */
  
  SDL_Rect SrcRect;//source rectangle
  SDL_Rect DstRect;//destination rectangle
  
  if(Digit<0||Digit>9)return 1;
  
  SrcRect.x=8*Digit;
  SrcRect.y=0;
  SrcRect.w=8;
  SrcRect.h=8;
  DstRect.x=X;
  DstRect.y=Y;
  DstRect.w=8;
  DstRect.h=8;
  
  SDL_RenderCopy(gRenderer,gTextureFont,&SrcRect,&DstRect);//Render character to screen
  return 0;
}

int renderCharacter(int X,int Y,char C){
  /*
  Renders a charter using (X,Y) as upper left corner.
  Return int value:
    0 on success
    1 if the character is not in the font
    2 if the character is a new line
    
    Possible modification: For variable character width, we should make the function return the width of the character it just printed. It would return 0 in case of failure.
    
  */
  
  SDL_Rect SrcRect;//source rectangle
  SDL_Rect DstRect;//destination rectangle
  SrcRect.w=8;
  SrcRect.h=8;
  DstRect.x=X;
  DstRect.y=Y;
  DstRect.w=8;
  DstRect.h=8;
  
  switch(C){
    case'0':SrcRect.x=  0;SrcRect.y= 0;break;
    case'1':SrcRect.x=  8;SrcRect.y= 0;break;
    case'2':SrcRect.x= 16;SrcRect.y= 0;break;
    case'3':SrcRect.x= 24;SrcRect.y= 0;break;
    case'4':SrcRect.x= 32;SrcRect.y= 0;break;
    case'5':SrcRect.x= 40;SrcRect.y= 0;break;
    case'6':SrcRect.x= 48;SrcRect.y= 0;break;
    case'7':SrcRect.x= 56;SrcRect.y= 0;break;
    case'8':SrcRect.x= 64;SrcRect.y= 0;break;
    case'9':SrcRect.x= 72;SrcRect.y= 0;break;
    case'A':SrcRect.x= 80;SrcRect.y= 0;break;
    case'B':SrcRect.x= 88;SrcRect.y= 0;break;
    case'C':SrcRect.x= 96;SrcRect.y= 0;break;
    case'D':SrcRect.x=104;SrcRect.y= 0;break;
    case'E':SrcRect.x=112;SrcRect.y= 0;break;
    case'F':SrcRect.x=120;SrcRect.y= 0;break;
    case'G':SrcRect.x=  0;SrcRect.y= 8;break;
    case'H':SrcRect.x=  8;SrcRect.y= 8;break;
    case'I':SrcRect.x= 16;SrcRect.y= 8;break;
    case'J':SrcRect.x= 24;SrcRect.y= 8;break;
    case'K':SrcRect.x= 32;SrcRect.y= 8;break;
    case'L':SrcRect.x= 40;SrcRect.y= 8;break;
    case'M':SrcRect.x= 48;SrcRect.y= 8;break;
    case'N':SrcRect.x= 56;SrcRect.y= 8;break;
    case'O':SrcRect.x= 64;SrcRect.y= 8;break;
    case'P':SrcRect.x= 72;SrcRect.y= 8;break;
    case'Q':SrcRect.x= 80;SrcRect.y= 8;break;
    case'R':SrcRect.x= 88;SrcRect.y= 8;break;
    case'S':SrcRect.x= 96;SrcRect.y= 8;break;
    case'T':SrcRect.x=104;SrcRect.y= 8;break;
    case'U':SrcRect.x=112;SrcRect.y= 8;break;
    case'V':SrcRect.x=120;SrcRect.y= 8;break;
    case'W':SrcRect.x=  0;SrcRect.y=16;break;
    case'X':SrcRect.x=  8;SrcRect.y=16;break;
    case'Y':SrcRect.x= 16;SrcRect.y=16;break;
    case'Z':SrcRect.x= 24;SrcRect.y=16;break;
    case'a':SrcRect.x= 32;SrcRect.y=16;break;
    case'b':SrcRect.x= 40;SrcRect.y=16;break;
    case'c':SrcRect.x= 48;SrcRect.y=16;break;
    case'd':SrcRect.x= 56;SrcRect.y=16;break;
    case'e':SrcRect.x= 64;SrcRect.y=16;break;
    case'f':SrcRect.x= 72;SrcRect.y=16;break;
    case'g':SrcRect.x= 80;SrcRect.y=16;break;
    case'h':SrcRect.x= 88;SrcRect.y=16;break;
    case'i':SrcRect.x= 96;SrcRect.y=16;break;
    case'j':SrcRect.x=104;SrcRect.y=16;break;
    case'k':SrcRect.x=112;SrcRect.y=16;break;
    case'l':SrcRect.x=120;SrcRect.y=16;break;
    case'm':SrcRect.x=  0;SrcRect.y=24;break;
    case'n':SrcRect.x=  8;SrcRect.y=24;break;
    case'o':SrcRect.x= 16;SrcRect.y=24;break;
    case'p':SrcRect.x= 24;SrcRect.y=24;break;
    case'q':SrcRect.x= 32;SrcRect.y=24;break;
    case'r':SrcRect.x= 40;SrcRect.y=24;break;
    case's':SrcRect.x= 48;SrcRect.y=24;break;
    case't':SrcRect.x= 56;SrcRect.y=24;break;
    case'u':SrcRect.x= 64;SrcRect.y=24;break;
    case'v':SrcRect.x= 72;SrcRect.y=24;break;
    case'w':SrcRect.x= 80;SrcRect.y=24;break;
    case'x':SrcRect.x= 88;SrcRect.y=24;break;
    case'y':SrcRect.x= 96;SrcRect.y=24;break;
    case'z':SrcRect.x=104;SrcRect.y=24;break;
    case'.':SrcRect.x=112;SrcRect.y=24;break;
    case':':SrcRect.x=120;SrcRect.y=24;break;
    case',':SrcRect.x=  0;SrcRect.y=32;break;
    case';':SrcRect.x=  8;SrcRect.y=32;break;
    case'|':SrcRect.x= 16;SrcRect.y=32;break;
    case'/':SrcRect.x= 24;SrcRect.y=32;break;
    case'\\':SrcRect.x=32;SrcRect.y=32;break;
    case'\'':SrcRect.x=40;SrcRect.y=32;break;
    case'\"':SrcRect.x=48;SrcRect.y=32;break;
    case'?':SrcRect.x= 56;SrcRect.y=32;break;
    case'!':SrcRect.x= 64;SrcRect.y=32;break;
    case'+':SrcRect.x= 72;SrcRect.y=32;break;
    case'-':SrcRect.x= 80;SrcRect.y=32;break;
    case'*':SrcRect.x= 88;SrcRect.y=32;break;
    case'=':SrcRect.x= 96;SrcRect.y=32;break;
    case'(':SrcRect.x=104;SrcRect.y=32;break;
    case')':SrcRect.x=112;SrcRect.y=32;break;
    case'_':SrcRect.x=120;SrcRect.y=32;break;
    case'<':SrcRect.x=  0;SrcRect.y=40;break;
    case'>':SrcRect.x=  8;SrcRect.y=40;break;
    case'[':SrcRect.x= 16;SrcRect.y=40;break;
    case']':SrcRect.x= 24;SrcRect.y=40;break;
    case'#':SrcRect.x= 32;SrcRect.y=40;break;
    case'$':SrcRect.x= 40;SrcRect.y=40;break;
    case'^':SrcRect.x= 48;SrcRect.y=40;break;
    //case'²':SrcRect.x= 56;SrcRect.y=40;break;//triggers a warning and doesn't work
    //case'ù':SrcRect.x= 64;SrcRect.y=40;break;//triggers a warning and doesn't work
    //case'µ':SrcRect.x= 72;SrcRect.y=40;break;//triggers a warning and doesn't work
    case' ':return 0;//space don't need rendering
    case'\n':return 2;
    default:return 1;
  }
  
  SDL_RenderCopy(gRenderer,gTextureFont,&SrcRect,&DstRect);//Render character to screen
  return 0;
}

int renderText(int X0,char String[]){
  /*
  Renders a text using (gTextX,gTextY) as upper left corner for the first line. Each new line starts at X0.
  */
  
  int i=0,j=0,NonPrintableCharacterToTreat;
  while(1){
    j=i;
    while(String[j]!=' '&&String[j]!=0&&String[j]!='\n')j++;//j points to the character after the current word
    if(gTextX+8*(j-i)>SCREEN_WIDTH){//If the current word is too long to fit on the current line, we go to the next line.
      gTextX=X0;
      gTextY+=LINE_HEIGHT;
    }
    while(i<j){//Print the current word
      renderCharacter(gTextX,gTextY,String[i++]);
      gTextX+=8;
    }
    
    NonPrintableCharacterToTreat=1;
    while(NonPrintableCharacterToTreat){//Handle spaces, new lines and the null character
      switch(String[i]){
        case' ':gTextX+=8;i++;break;
        case'\n':gTextY+=LINE_HEIGHT;gTextX=X0;i++;break;
        case 0:return 0;//Exit if the last character is a null character
        default:NonPrintableCharacterToTreat=0;
      }
    }
  }
}


void renderTile(int Row,int Column,int ScreenX,int ScreenY,int RenderType,Uint16 Map[MAX_LAYER][MAX_ROW][MAX_COLUMN]){
//Renders the tile at (Row,Column).
  int Layer;
  
  SDL_Rect SourceRectangle;
  SourceRectangle.w=32;
  SourceRectangle.h=32;
  SDL_Rect DestinationRectangle;
  DestinationRectangle.x=32*Column-ScreenX;
  DestinationRectangle.y=32*Row-ScreenY;
  DestinationRectangle.w=32;
  DestinationRectangle.h=32;
  
  for(Layer=0;Layer<MAX_LAYER;Layer++){
    if(RenderType<gTileType[Map[Layer][Row][Column]]&&gTileType[Map[Layer][Row][Column]]<(2+3*RenderType)){
      SourceRectangle.x=32*((Map[Layer][Row][Column]-1)%64);
      SourceRectangle.y=32*((Map[Layer][Row][Column]-1)/64);
      SDL_RenderCopy(gRenderer,gTextureTiles,&SourceRectangle,&DestinationRectangle);//Render tile to screen
    }
  }
  
}

void renderMap(int ScreenX,int ScreenY,int RenderType,Uint16 Map[MAX_LAYER][MAX_ROW][MAX_COLUMN]){
  int i,j;
  int MaxI=((ScreenY+SCREEN_HEIGHT)/32+((ScreenY+SCREEN_HEIGHT)%32>0));
  int MaxJ=((ScreenX+SCREEN_WIDTH)/32+((ScreenX+SCREEN_WIDTH)%32>0));
  for(i=ScreenY/32;i<MaxI;i++){
    for(j=ScreenX/32;j<MaxJ;j++){
      renderTile(i,j,ScreenX,ScreenY,RenderType,Map);
    }
  }
}

void renderProjectiles(int ScreenX,int ScreenY){
  int i;
  SDL_Rect SrcRect;//source rectangle
  SrcRect.w=32;
  SrcRect.h=32;
  SDL_Rect DstRect;//destination rectangle
  DstRect.w=SrcRect.w;
  DstRect.h=SrcRect.h;
  
  for(i=0;i<gNumberOfProjectiles;i++){
    SrcRect.x=32*gProjectileList[i][7];//select the projectile sprite according to its Direction
    SrcRect.y=32*gProjectileList[i][8];//select the projectile sprite according to its Type
    DstRect.x=gProjectileList[i][0]/10-16-ScreenX;
    DstRect.y=gProjectileList[i][1]/10-16-ScreenY;
    SDL_RenderCopy(gRenderer,gTextureProjectiles,&SrcRect,&DstRect);//Render projectile to screen
  }
}

void renderPlayer(int ScreenX,int ScreenY,int Frame,int Orientation){
  //Renders the player in rectangle [ (gPlayerX-32,gPlayerY-54) , (gPlayerX+32,gPlayerY+10) ].
  SDL_Rect SrcRect;//source rectangle
  SrcRect.x=64*Frame;
  SrcRect.y=64*(Orientation+8);
  SrcRect.w=64;
  SrcRect.h=64;
  SDL_Rect DstRect;//destination rectangle
  DstRect.x=gPlayer.x/10-32-ScreenX;
  DstRect.y=(gPlayer.y+HALF_SIDE_PLAYER_COLLISION_BOX)/10-64-ScreenY;
  DstRect.w=64;
  DstRect.h=64;
  SDL_RenderCopy(gRenderer,getSpriteset(&gPlayer),&SrcRect,&DstRect);//Render player to screen
}

void renderEnemy(int ScreenX,int ScreenY,int Frame,int Orientation,int EnemyIndex){
  SDL_Rect SrcRect;//source rectangle
  SrcRect.x=64*Frame;
  SrcRect.y=64*(Orientation+8);
  SrcRect.w=64;
  SrcRect.h=64;
  SDL_Rect DstRect;//destination rectangle
  DstRect.x=gEnemyList[EnemyIndex][0]/10-32-ScreenX;
  DstRect.y=(gEnemyList[EnemyIndex][1]+HALF_SIDE_PLAYER_COLLISION_BOX)/10-64-ScreenY;
  DstRect.w=64;
  DstRect.h=64;
  SDL_RenderCopy(gRenderer,gTextureEnemy,&SrcRect,&DstRect);//Render enemy to screen
}


void renderSpellInterface(int SpellTypeLeft,int SpellTypeRight){
  int i;
  int OffsetY=20;
  
  {//Render the two active spells
    SDL_Rect SrcRect;//source rectangle
    SrcRect.x=0;
    SrcRect.w=32;
    SrcRect.h=32;
    SDL_Rect DstRect;//destination rectangle
    DstRect.y=OffsetY-6;
    DstRect.w=SrcRect.w;
    DstRect.h=SrcRect.h;
    
    if(SpellTypeLeft<NUMBER_OF_SPELLS){
      SrcRect.y=32*SpellTypeLeft;
      DstRect.x=SCREEN_WIDTH-98;
      SDL_RenderCopy(gRenderer,gTextureProjectiles,&SrcRect,&DstRect);//Render Left spell
    }
    if(SpellTypeRight<NUMBER_OF_SPELLS){
      SrcRect.y=32*SpellTypeRight;
      DstRect.x=SCREEN_WIDTH-73;
      SDL_RenderCopy(gRenderer,gTextureProjectiles,&SrcRect,&DstRect);//Render Right spell
    }
    
    SDL_SetRenderDrawColor(gRenderer,255,255,255,255);//set the rendering color to white
    SDL_Rect OutlineRect={SCREEN_WIDTH-92,OffsetY,20,20};
    SDL_RenderDrawRect(gRenderer,&OutlineRect);
    OutlineRect.x=SCREEN_WIDTH-67;
    SDL_RenderDrawRect(gRenderer,&OutlineRect);
    SDL_SetRenderDrawColor(gRenderer,0,0,0,255);//set back the rendering color to black
  }
  
  {//render spell stock info
    SDL_Rect SrcRect;//source rectangle
    SrcRect.x=512;
    SrcRect.w=8;
    SrcRect.h=8;
    SDL_Rect DstRect;//destination rectangle
    DstRect.x=SCREEN_WIDTH-42;
    DstRect.y=OffsetY;
    DstRect.w=SrcRect.w;
    DstRect.h=SrcRect.h;
    
    for(i=0;i<NUMBER_OF_SPELLS;i++){
      if(gHasSpell[i]){
        SrcRect.y=16+32*i;
        SDL_RenderCopy(gRenderer,gTextureProjectiles,&SrcRect,&DstRect);//Render projectile to screen
        renderDigit(DstRect.x+9,DstRect.y,gSpellStock[i]/100);
        renderDigit(DstRect.x+17,DstRect.y,(gSpellStock[i]/10)%10);
        renderDigit(DstRect.x+25,DstRect.y,gSpellStock[i]%10);
        DstRect.y+=LINE_HEIGHT;
      }
    }
  }
}

void renderHP(int HP,int MaxHP){
//renders HP in as heart counter: 1 heart = 2HP, MaxHP must be an even number from 2 to 20
  
  SDL_Rect SrcRect;//source rectangle
  SrcRect.x=0;
  SrcRect.y=0;
  SrcRect.w=6*MaxHP;
  SrcRect.h=11;
  SDL_Rect DstRect;//destination rectangle
  DstRect.x=SCREEN_WIDTH-128;
  DstRect.y=5;
  DstRect.w=SrcRect.w;
  DstRect.h=SrcRect.h;
  SDL_RenderCopy(gRenderer,gTextureLife,&SrcRect,&DstRect);//Render empty hearts
  
  SrcRect.y=SrcRect.h;
  SrcRect.w=6*HP;
  SrcRect.h-=2;
  DstRect.y++;
  DstRect.w=SrcRect.w;
  DstRect.h=SrcRect.h;
  SDL_RenderCopy(gRenderer,gTextureLife,&SrcRect,&DstRect);//Render heart content
}


int moveRight(int Speed){
//Moves right and corrects gPlayerX for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderRow,RowOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayer.x;
  gPlayer.x+=Speed;
  
  if(gPlayer.x>MAX_COLUMN*320-BOUNDARY_OFFSET)gPlayer.x=MAX_COLUMN*320-BOUNDARY_OFFSET;
  DestinationRow=gPlayer.y/320;
  DestinationColumn=gPlayer.x/320;
  RemainderRow=gPlayer.y%320;
  RowOffsetOfAdjacentTileToCheck=(RemainderRow>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderRow<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile above, is 0 if we don't have to check a second adjacent tile, 1 if we have to check the tile below
  if(DestinationColumn<MAX_COLUMN && ( gMapMovementObstacle[DestinationRow][DestinationColumn+1] || gMapMovementObstacle[DestinationRow+RowOffsetOfAdjacentTileToCheck][DestinationColumn+1]) ){
    Boundary=320*(DestinationColumn+1)-BOUNDARY_OFFSET;
    if(gPlayer.x>Boundary)gPlayer.x=Boundary;
  }
  
  return gPlayer.x!=InitialCoordinate;
}

int moveLeft(int Speed){
//Moves left and corrects gPlayerX for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderRow,RowOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayer.x;
  gPlayer.x-=Speed;
  
  if(gPlayer.x<BOUNDARY_OFFSET)gPlayer.x=BOUNDARY_OFFSET;
  DestinationRow=gPlayer.y/320;
  DestinationColumn=gPlayer.x/320;
  RemainderRow=gPlayer.y%320;
  RowOffsetOfAdjacentTileToCheck=(RemainderRow>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderRow<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile above, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile below
  if(DestinationColumn>0 && ( gMapMovementObstacle[DestinationRow][DestinationColumn-1] || gMapMovementObstacle[DestinationRow+RowOffsetOfAdjacentTileToCheck][DestinationColumn-1]) ){
    Boundary=320*DestinationColumn+BOUNDARY_OFFSET;
    if(gPlayer.x<Boundary)gPlayer.x=Boundary;
  }
  
  return gPlayer.x!=InitialCoordinate;
}

int moveDown(int Speed){
//Moves down and corrects gPlayerY for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderColumn,ColumnOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayer.y;
  gPlayer.y+=Speed;
  
  if(gPlayer.y>MAX_ROW*320-BOUNDARY_OFFSET)gPlayer.y=MAX_ROW*320-BOUNDARY_OFFSET;
  DestinationRow=gPlayer.y/320;
  DestinationColumn=gPlayer.x/320;
  RemainderColumn=gPlayer.x%320;
  ColumnOffsetOfAdjacentTileToCheck=(RemainderColumn>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderColumn<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile to the left, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile to the right
  if(DestinationRow<MAX_ROW && ( gMapMovementObstacle[DestinationRow+1][DestinationColumn] || gMapMovementObstacle[DestinationRow+1][DestinationColumn+ColumnOffsetOfAdjacentTileToCheck]) ){
    Boundary=320*(DestinationRow+1)-BOUNDARY_OFFSET;
    if(gPlayer.y>Boundary)gPlayer.y=Boundary;
  }
  
  return gPlayer.y!=InitialCoordinate;
}

int moveUp(int Speed){
//Moves up and corrects gPlayerY for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderColumn,ColumnOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayer.y;
  gPlayer.y-=Speed;
  
  if(gPlayer.y<BOUNDARY_OFFSET)gPlayer.y=BOUNDARY_OFFSET;
  DestinationRow=gPlayer.y/320;
  DestinationColumn=gPlayer.x/320;
  RemainderColumn=gPlayer.x%320;
  ColumnOffsetOfAdjacentTileToCheck=(RemainderColumn>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderColumn<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile to the left, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile to the right
  if(DestinationRow>0 && ( gMapMovementObstacle[DestinationRow-1][DestinationColumn] || gMapMovementObstacle[DestinationRow-1][DestinationColumn+ColumnOffsetOfAdjacentTileToCheck]) ){
    Boundary=320*(DestinationRow)+BOUNDARY_OFFSET;
    if(gPlayer.y<Boundary)gPlayer.y=Boundary;
  }
  
  return gPlayer.y!=InitialCoordinate;
}

int movePlayer(void){
/*moves the player
  returned int:
   0 = no move
   1 = successful move
  Adapt gPlayerOrientation:
   0 = Orientation UP or UP_LEFT
   1 = Orientation LEFT or DOWN_LEFT
   2 = Orientation DOWN or DOWN_RIGHT
   3 = Orientation RIGHT or UP_RIGHT
*/
  const Uint8 *KeyboardState=SDL_GetKeyboardState(NULL);
  int MoveResult1,MoveResult2;
  
  if(KeyboardState[SDL_SCANCODE_UP]&&KeyboardState[SDL_SCANCODE_LEFT]){
    MoveResult1=moveUp(getDiagonalSpeed(&gPlayer));
    MoveResult2=moveLeft(getDiagonalSpeed(&gPlayer));
    if(MoveResult1){gPlayerOrientation=0;return 1;}
    if(MoveResult2){gPlayerOrientation=1;return 1;}
    gPlayerOrientation=0;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]&&KeyboardState[SDL_SCANCODE_LEFT]){
    MoveResult1=moveDown(getDiagonalSpeed(&gPlayer));
    MoveResult2=moveLeft(getDiagonalSpeed(&gPlayer));
    if(MoveResult2){gPlayerOrientation=1;return 1;}
    if(MoveResult1){gPlayerOrientation=2;return 1;}
    gPlayerOrientation=1;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]&&KeyboardState[SDL_SCANCODE_RIGHT]){
    MoveResult1=moveDown(getDiagonalSpeed(&gPlayer));
    MoveResult2=moveRight(getDiagonalSpeed(&gPlayer));
    if(MoveResult1){gPlayerOrientation=2;return 1;}
    if(MoveResult2){gPlayerOrientation=3;return 1;}
    gPlayerOrientation=2;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_UP]&&KeyboardState[SDL_SCANCODE_RIGHT]){
    MoveResult1=moveUp(getDiagonalSpeed(&gPlayer));
    MoveResult2=moveRight(getDiagonalSpeed(&gPlayer));
    if(MoveResult2){gPlayerOrientation=3;return 1;}
    if(MoveResult1){gPlayerOrientation=0;return 1;}
    gPlayerOrientation=3;return 0;
  }
  
  if(KeyboardState[SDL_SCANCODE_UP]){
    gPlayerOrientation=0;
    return moveUp(getSpeed(&gPlayer));
  }
  if(KeyboardState[SDL_SCANCODE_LEFT]){
    gPlayerOrientation=1;
    return moveLeft(getSpeed(&gPlayer));
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]){
    gPlayerOrientation=2;
    return moveDown(getSpeed(&gPlayer));
  }
  if(KeyboardState[SDL_SCANCODE_RIGHT]){
    gPlayerOrientation=3;
    return moveRight(getSpeed(&gPlayer));
  }
  
  return 0;
}

void moveEnemies(){
  
  int i;
  for(i=0;i<gNumberOfEnemies;i++){
    if(gEnemyList[i][ENEMY_INFO]){//Check if the enemy has any move instructions
      gEnemyList[i][0]+=gEnemyList[i][gEnemyList[i][ENEMY_INFO]];//move along X
      gEnemyList[i][1]+=gEnemyList[i][gEnemyList[i][ENEMY_INFO]+1];//move along Y
      if(--gEnemyList[i][ENEMY_INFO+2]<=0)//All steps from the current instruction are done
        if(gEnemyList[i][ENEMY_INFO]==ENEMY_INFO+3*MAX_INSTRUCTIONS){//If we reached the last instruction, go to the "when-done" instruction
          if(gEnemyList[i][ENEMY_INFO]=gEnemyList[i][ENEMY_INFO+1])
            gEnemyList[i][ENEMY_INFO+2]=gEnemyList[i][gEnemyList[i][ENEMY_INFO]+2];
        }
        else{
          gEnemyList[i][ENEMY_INFO]+=3;//Go to next instruction
          if(!(gEnemyList[i][ENEMY_INFO+2]=gEnemyList[i][gEnemyList[i][ENEMY_INFO]+2])){//If the next instruction has 0 steps, go to the "when-done" instruction
            if(gEnemyList[i][ENEMY_INFO]=gEnemyList[i][ENEMY_INFO+1])
              gEnemyList[i][ENEMY_INFO+2]=gEnemyList[i][gEnemyList[i][ENEMY_INFO]+2];
          }
        }
    }
  }
  
}

void centreScreenOnPlayer(int*ScreenX,int*ScreenY){
  if(gPlayer.x/10<=SCREEN_WIDTH/2)*ScreenX=0;
  else if(gPlayer.x/10>=MAX_COLUMN*32-SCREEN_WIDTH/2)*ScreenX=MAX_COLUMN*32-SCREEN_WIDTH;
  else *ScreenX=gPlayer.x/10-SCREEN_WIDTH/2;
  
  if(gPlayer.y/10<=SCREEN_HEIGHT/2)*ScreenY=0;
  else if(gPlayer.y/10>=MAX_ROW*32-SCREEN_HEIGHT/2)*ScreenY=MAX_ROW*32-SCREEN_HEIGHT;
  else *ScreenY=gPlayer.y/10-SCREEN_HEIGHT/2;
}

void applyProjectileImpact(int TargetType,int ProjectileIndex,Uint16 Map[MAX_LAYER][MAX_ROW][MAX_COLUMN]){
/*Damage enemy or modify the tiles depending on the spell, removes the projectile from gProjectileList when done.
  TargetType:
   0 = empty tile (ground, building, ... )
   1 = the player
   2 = enemy index 0
   3 = enemy index 1
   ...
*/
  int Layer,i,j,TargetRow,TargetColumn;
  
  if(TargetType==0){
    TargetColumn=gProjectileList[ProjectileIndex][0]/320;
    TargetRow=gProjectileList[ProjectileIndex][1]/320;
    switch(gProjectileList[ProjectileIndex][8]){//ProjectileType
      case 0://fire spell
        switch(Map[0][TargetRow][TargetColumn]){//Tile modifications
          case 12:case 74:case 76:case 138:case 203:case 266:case 706:case 735:case 1612:case 2056:case 2061:case 2066:case 2130:case 2194:case 2248:case 2249:case 2250:case 2251:case 2252:case 2253:case 2254:case 2255:case 2256:case 2257: case 2258:case 2316:case 2378:case 2433:case 2434:case 3536:case 3589:
            Map[0][TargetRow][TargetColumn]=75;break;//If the tile index was any for the above, it turns into tile 75 (=burned tile).
          case 645:Map[0][TargetRow][TargetColumn]=646;break;
          case 646:case 734:Map[0][TargetRow][TargetColumn]=706;break;
          case 731:Map[0][TargetRow][TargetColumn]=641;break;
          case 733:Map[0][TargetRow][TargetColumn]=643;break;
          case 795:Map[0][TargetRow][TargetColumn]=833;break;
          case 796:Map[0][TargetRow][TargetColumn]=834;break;
          case 797:Map[0][TargetRow][TargetColumn]=835;break;
          case 799:Map[0][TargetRow][TargetColumn]=644;break;
          case 1164:Map[0][TargetRow][TargetColumn]=1612;break;
          case 1225:Map[0][TargetRow][TargetColumn]=2817;break;
          case 1227:Map[0][TargetRow][TargetColumn]=2819;break;
          case 1289:Map[0][TargetRow][TargetColumn]=2945;break;
          case 1290:Map[0][TargetRow][TargetColumn]=2946;break;
          case 1291:Map[0][TargetRow][TargetColumn]=2947;break;
          case 1353:Map[0][TargetRow][TargetColumn]=1030;break;
          case 1354:Map[0][TargetRow][TargetColumn]=1031;break;
          case 1355:Map[0][TargetRow][TargetColumn]=1032;break;
          case 1417:Map[0][TargetRow][TargetColumn]=1094;break;
          case 1418:Map[0][TargetRow][TargetColumn]=1095;break;
          case 1419:Map[0][TargetRow][TargetColumn]=1096;break;
          case 1481:Map[0][TargetRow][TargetColumn]=1158;break;
          case 1482:Map[0][TargetRow][TargetColumn]=1159;break;
          case 1483:Map[0][TargetRow][TargetColumn]=1160;break;
          case 1478:Map[0][TargetRow][TargetColumn]=1220;break;
          case 1479:Map[0][TargetRow][TargetColumn]=1221;break;
          case 1480:Map[0][TargetRow][TargetColumn]=1222;break;
          case 1542:Map[0][TargetRow][TargetColumn]=1284;break;
          case 1543:Map[0][TargetRow][TargetColumn]=1285;break;
          case 1544:Map[0][TargetRow][TargetColumn]=1286;break;
          case 1606:Map[0][TargetRow][TargetColumn]=1348;break;
          case 1607:Map[0][TargetRow][TargetColumn]=1349;break;
          case 1608:Map[0][TargetRow][TargetColumn]=1350;break;
          case 1228:Map[0][TargetRow][TargetColumn]=1223;break;
          case 1229:Map[0][TargetRow][TargetColumn]=1224;break;
          case 1292:Map[0][TargetRow][TargetColumn]=1287;break;
          case 1293:Map[0][TargetRow][TargetColumn]=1288;break;
          case 1545:Map[0][TargetRow][TargetColumn]=1351;break;
          case 1546:Map[0][TargetRow][TargetColumn]=1352;break;
          case 1609:Map[0][TargetRow][TargetColumn]=1415;break;
          case 1610:Map[0][TargetRow][TargetColumn]=1416;break;
        }
        break;
      case 1://standard damage spell, doesn't modify terrain
        break;
      case 2://ice spell
        switch(Map[0][TargetRow][TargetColumn]){//Tile modifications
          case 75:Map[0][TargetRow][TargetColumn]=735;break;
          case 641:Map[0][TargetRow][TargetColumn]=731;break;
          case 643:Map[0][TargetRow][TargetColumn]=733;break;
          case 644:case 2885:case 2949:case 3095:Map[0][TargetRow][TargetColumn]=799;break;
          case 706:Map[0][TargetRow][TargetColumn]=734;break;
          case 833:Map[0][TargetRow][TargetColumn]=795;break;
          case 834:Map[0][TargetRow][TargetColumn]=796;break;
          case 835:Map[0][TargetRow][TargetColumn]=797;break;
          case 1612:case 2056:case 2061:case 2066:case 2130:case 2194:case 2248:case 2249:case 2250:case 2251:case 2252:case 2253:case 2254:case 2255:case 2256:case 2257: case 2258:case 2316:case 2378:case 2433:case 2434:case 3536:case 3589:
            Map[0][TargetRow][TargetColumn]=1164;break;
          case 2817:Map[0][TargetRow][TargetColumn]=1225;break;
          case 2819:Map[0][TargetRow][TargetColumn]=1227;break;
          case 2945:Map[0][TargetRow][TargetColumn]=1289;break;
          case 2946:Map[0][TargetRow][TargetColumn]=1290;break;
          case 2947:Map[0][TargetRow][TargetColumn]=1291;break;
          case 1030:Map[0][TargetRow][TargetColumn]=1353;break;
          case 1031:Map[0][TargetRow][TargetColumn]=1354;break;
          case 1032:Map[0][TargetRow][TargetColumn]=1355;break;
          case 1094:Map[0][TargetRow][TargetColumn]=1417;break;
          case 1095:Map[0][TargetRow][TargetColumn]=1418;break;
          case 1096:Map[0][TargetRow][TargetColumn]=1419;break;
          case 1158:Map[0][TargetRow][TargetColumn]=1481;break;
          case 1159:Map[0][TargetRow][TargetColumn]=1482;break;
          case 1160:Map[0][TargetRow][TargetColumn]=1483;break;
          case 1220:Map[0][TargetRow][TargetColumn]=1478;break;
          case 1221:Map[0][TargetRow][TargetColumn]=1479;break;
          case 1222:Map[0][TargetRow][TargetColumn]=1480;break;
          case 1284:Map[0][TargetRow][TargetColumn]=1542;break;
          case 1285:Map[0][TargetRow][TargetColumn]=1543;break;
          case 1286:Map[0][TargetRow][TargetColumn]=1544;break;
          case 1348:Map[0][TargetRow][TargetColumn]=1606;break;
          case 1349:Map[0][TargetRow][TargetColumn]=1607;break;
          case 1350:Map[0][TargetRow][TargetColumn]=1608;break;
          case 1223:Map[0][TargetRow][TargetColumn]=1228;break;
          case 1224:Map[0][TargetRow][TargetColumn]=1229;break;
          case 1287:Map[0][TargetRow][TargetColumn]=1292;break;
          case 1288:Map[0][TargetRow][TargetColumn]=1293;break;
          case 1351:Map[0][TargetRow][TargetColumn]=1545;break;
          case 1352:Map[0][TargetRow][TargetColumn]=1546;break;
          case 1415:Map[0][TargetRow][TargetColumn]=1609;break;
          case 1416:Map[0][TargetRow][TargetColumn]=1610;break;
        }
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        break;
    }
    for(Layer=2;!Map[--Layer][TargetRow][TargetColumn];);
    gMapMovementObstacle[TargetRow][TargetColumn]=(gTileType[Map[Layer][TargetRow][TargetColumn]]>2);
    gMapProjectileObstacle[TargetRow][TargetColumn]=(gTileType[Map[Layer][TargetRow][TargetColumn]]==4);
  }
  else if(TargetType==1){
    gPlayerHP--;
    gPlayerSkipFrames=2;//makes the player blink when hit
  }
  else{
    gEnemyList[TargetType-2][7]=2;//makes the enemy blink when hit
    if(gProjectileList[ProjectileIndex][8]==3)//Reduce enemy HP according to the ProjectileType
      gEnemyList[TargetType-2][3]-=2;
    else
      gEnemyList[TargetType-2][3]--;
    if(gEnemyList[TargetType-2][3]<=0){//Remove enemy it if HP<=0
      gNumberOfEnemies--;
      for(i=TargetType-2;i<gNumberOfEnemies;i++)
        for(j=0;j<ENEMY_INFO+1+3*MAX_INSTRUCTIONS;j++)gEnemyList[i][j]=gEnemyList[i+1][j];
    }
  }
  
  {//Remove projectile
    gNumberOfProjectiles--;
    for(i=ProjectileIndex;i<gNumberOfProjectiles;i++)
      for(j=0;j<PROJECTILE_INFO;j++)gProjectileList[i][j]=gProjectileList[i+1][j];
  }
}

void moveProjectiles(Uint16 Map[MAX_LAYER][MAX_ROW][MAX_COLUMN]){
//Moves all projectiles and remove the ones that reach their target
  int i,j,DestinationRow,DestinationColumn,DeltaX,DeltaY,HasHit;
  for(i=0;i<gNumberOfProjectiles;i++){
    if(gProjectileList[i][4]){//the projectile has some steps left to travel
      gProjectileList[i][0]+=gProjectileList[i][2];
      gProjectileList[i][1]+=gProjectileList[i][3];
    }
    else{//this projectile reached its target
      gProjectileList[i][0]=gProjectileList[i][5]*10;
      gProjectileList[i][1]=gProjectileList[i][6]*10;
    }
    gProjectileList[i][4]--;
    
    HasHit=0;
    if(gProjectileList[i][9]==0)//Checks if a player projectile hits an enemy
      for(j=0;j<gNumberOfEnemies;j++){
        DeltaX=gProjectileList[i][0]-gEnemyList[j][0];
        DeltaY=gProjectileList[i][1]-gEnemyList[j][1];
        if(-190<DeltaX&&DeltaX<200&&-460<DeltaY&&DeltaY<140){//gEnemyList[j][1] is 16 pixels larger than hit box centre
          applyProjectileImpact(j+2,i--,Map);
          HasHit=1;
          break;
        }
      }
    if(HasHit==0){
      DeltaX=gProjectileList[i][0]-gPlayer.x;
      DeltaY=gProjectileList[i][1]-gPlayer.y;
      DestinationColumn=gProjectileList[i][0]/320;
      DestinationRow=gProjectileList[i][1]/320;
      if(gProjectileList[i][9]&&-190<DeltaX&&DeltaX<200&&-460<DeltaY&&DeltaY<140)//Checks if an enemy projectile hits the player
        applyProjectileImpact(1,i--,Map);
      else if(gProjectileList[i][4]==-1||gMapProjectileObstacle[DestinationRow][DestinationColumn])//Checks if the projectile reached its target or encountered an obstacle
        applyProjectileImpact(0,i--,Map);
    }
  }
}

void computeProjectileVelocityAndSteps(int ProjectileIndex){
//compute the velocity components and the number of steps for the projectile given by ProjectileIndex
  int DeltaX=10*gProjectileList[ProjectileIndex][5]-gProjectileList[ProjectileIndex][0];
  int DeltaY=10*gProjectileList[ProjectileIndex][6]-gProjectileList[ProjectileIndex][1];
  float DistanceSquare;
  DistanceSquare=(float)DeltaX*(float)DeltaX+(float)DeltaY*(float)DeltaY;
  
  gProjectileList[ProjectileIndex][4]=((int)(sqrt(DistanceSquare)))/gProjectileSpeed[gProjectileList[ProjectileIndex][8]];//Steps
  if(gProjectileList[ProjectileIndex][4]){//This if prevents dividing by zero
    gProjectileList[ProjectileIndex][2]=DeltaX/gProjectileList[ProjectileIndex][4];//Velocity X
    gProjectileList[ProjectileIndex][3]=DeltaY/gProjectileList[ProjectileIndex][4];//Velocity Y
  }
}

int computeProjectileDirection(int ProjectileIndex){
  /* computes the direction of the projectile
    returned int:
     0 = direction in [-11.25°, 11.25°[
     1 = direction in [ 11.25°, 33.75°[
     2 = direction in [ 33.75°, 56.25°[
     3 = direction in [ 56.25°, 78.75°[
     4 = direction in [ 78.75°,101.25°[
     5 = direction in [101.25°,123.75°[
     6 = direction in [123.75°,146.25°[
     7 = direction in [146.25°,168.75°[
     8 = direction in [168.75°,191.25°[
     9 = direction in [191.25°,213.75°[
    10 = direction in [213.75°,236.25°[
    11 = direction in [236.25°,258.75°[
    12 = direction in [258.75°,281.25°[
    13 = direction in [281.25°,303.75°[
    14 = direction in [303.75°,326.25°[
    15 = direction in [326.25°,348.75°[
  */
  long DeltaX,DeltaY;//in tenth of pixel
  DeltaX=10*(long)gProjectileList[ProjectileIndex][5]-(long)gProjectileList[ProjectileIndex][0];
  DeltaY=(long)gProjectileList[ProjectileIndex][1]-10*(long)gProjectileList[ProjectileIndex][6];
  
  if(5*DeltaY>-DeltaX){
    if(DeltaY<5*DeltaX){
      if(3*DeltaY<2*DeltaX){
        if(5*DeltaY<DeltaX)return 0;else return 1;
      }
      else{
        if(2*DeltaY<3*DeltaX)return 2;else return 3;
      }
    }
    else{
      if(2*DeltaY>-3*DeltaX){
        if(DeltaY>-5*DeltaX)return 4;else return 5;
      }
      else{
        if(3*DeltaY>-2*DeltaX)return 6;else return 7;
      }
    }
  }
  else{
    if(DeltaY>5*DeltaX){
      if(3*DeltaY>2*DeltaX){
        if(5*DeltaY>DeltaX)return 8;else return 9;
      }
      else{
        if(2*DeltaY>3*DeltaX)return 10;else return 11;
      }
    }
    else{
      if(2*DeltaY<-3*DeltaX){
        if(DeltaY<-5*DeltaX)return 12;else return 13;
      }
      else{
        if(3*DeltaY<-2*DeltaX)return 14;else return 15;
      }
    }
  }
  
}

void makeEnemyShoot(int EnemyIndex){
  gProjectileList[gNumberOfProjectiles][0]=gEnemyList[EnemyIndex][0];
  gProjectileList[gNumberOfProjectiles][1]=gEnemyList[EnemyIndex][1];
  gProjectileList[gNumberOfProjectiles][5]=gPlayer.x/10;
  gProjectileList[gNumberOfProjectiles][6]=gPlayer.y/10;
  gProjectileList[gNumberOfProjectiles][8]=6;
  gProjectileList[gNumberOfProjectiles][9]=1;
  computeProjectileVelocityAndSteps(gNumberOfProjectiles);
  gProjectileList[gNumberOfProjectiles][4]=9858/gProjectileSpeed[gProjectileList[gNumberOfProjectiles][8]];
  gProjectileList[gNumberOfProjectiles][7]=computeProjectileDirection(gNumberOfProjectiles);
  gNumberOfProjectiles++;
}


int displayTitleScreen(void){
/*Function that displays the title screen
  Return int value:
    0 if the player requests to quit: "I want to be a warrior"
    1 if the player starts a new game: "I want to learn magic"
    2 if the player loads a saved game: "I know some spells"
*/
  SDL_Event Event;
  int Choice=1;
  int Y[3]={478,362,420};//Y[Choice] is the Y coordinate of the upper side of the selection rectangle
  SDL_Rect SelectionRectangle={365,Y[Choice],410,50};
  SDL_Rect ScreenRectangle={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
  SDL_SetRenderDrawColor(gRenderer,255,255,255,255);//set the rendering color to white
  
  while(1){
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer,gTextureTitle,&ScreenRectangle,&ScreenRectangle);
    SDL_RenderDrawRect(gRenderer,&SelectionRectangle);//render selection rectangle
    SDL_RenderPresent(gRenderer);
    while(SDL_PollEvent(&Event)){
      switch(Event.type){
        case SDL_QUIT:
          return 0;
        case SDL_KEYDOWN:
          switch(Event.key.keysym.sym){//Select action based on key press
            case SDLK_UP: //SDL_Keycode for detailed list
              if(Choice==0)Choice=2;else Choice--;
              SelectionRectangle.y=Y[Choice];
              break;
            case SDLK_DOWN:
              if(Choice==2)Choice=0;else Choice++;
              SelectionRectangle.y=Y[Choice];
              break;
            case SDLK_RETURN:case SDLK_RETURN2:case SDLK_KP_ENTER:
              return Choice;
          }
      }
    }
    SDL_Delay(DELAY_BETWEEN_FRAMES);
  }
}

int displayGameOverScreen(void){
/*Function that displays the game over screen
  Return int value:
    0 if the player requests to quit
    1 if the player press any key
*/
  SDL_Event Event;
  SDL_Rect ScreenRectangle={0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
  
  while(1){
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer,gTextureGameOver,&ScreenRectangle,&ScreenRectangle);
    SDL_RenderPresent(gRenderer);
    while(SDL_PollEvent(&Event)){
      switch(Event.type){
        case SDL_QUIT:
          return 0;
        case SDL_KEYDOWN:case SDL_MOUSEBUTTONDOWN:
          return 1;
      }
    }
    SDL_Delay(DELAY_BETWEEN_FRAMES);
  }
}

int play(void){
/*Function that plays the game
  Return int value:
    0 if the player requests to quit
    1 to return to title screen
*/
  Uint32 LastUpdate,ElapsedTime;//LastUpdate and ElapsedTime in millisecond
  Uint32 Hello=5000;//Duration in milliseconds of a Hello test message
  Uint16 Map[MAX_LAYER][MAX_ROW][MAX_COLUMN]={//Map[layer][i][j] is the index of the tile to display at (row=i, column=j).
  706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,479,415,414,266,12,706,706,706,1281,1283,706,706,706,706,706,706,706,1089,706,706,706,706,1026,706,10,267,75,202,140,706,706,706,706,706,706,706,706,694,694,694,694,694,706,706,644,644,644,644,769,706,706,115,116,179,243,181,706,458,203,395,266,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,203,415,415,266,11,12,706,1345,1347,706,1089,706,706,1025,706,706,706,706,706,706,706,1025,10,267,75,202,140,706,1025,1025,1025,706,706,706,706,694,706,706,694,706,706,706,771,644,644,644,3095,833,705,178,179,180,180,180,245,706,706,458,203,395,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,414,415,414,266,12,706,706,706,706,706,706,706,706,706,1026,706,706,706,1154,706,74,75,202,140,706,706,706,706,1025,706,706,706,706,706,706,706,694,706,706,707,835,644,644,644,644,2885,705,242,243,180,307,308,309,706,706,706,394,395,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,203,479,415,266,11,12,706,706,706,1154,706,706,706,706,706,706,1153,706,10,267,202,140,1089,706,706,706,706,1025,706,1025,706,706,706,706,706,706,706,706,707,644,644,644,644,644,2949,705,306,703,704,309,371,373,706,706,706,394,395,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,203,479,414,266,11,12,706,706,706,706,1025,706,706,1025,706,706,74,268,76,706,706,706,706,706,706,1025,1025,1025,706,706,1025,706,706,706,706,770,771,644,641,643,644,644,641,706,370,767,768,373,706,706,706,706,706,458,203,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,414,415,414,266,11,12,1154,706,706,706,706,706,706,10,267,202,140,706,706,706,706,706,706,706,1025,1025,706,706,706,706,706,706,707,834,835,641,706,771,644,644,705,706,706,706,706,706,706,706,706,706,706,706,458,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,203,413,415,414,266,12,706,1025,706,1089,706,706,74,75,76,706,706,706,706,706,706,706,706,706,1089,1025,706,706,706,706,706,771,2885,644,705,707,835,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,203,415,413,266,12,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,770,770,771,835,2949,644,769,771,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1030,1031,1032,706,138,139,203,413,266,11,12,706,1090,706,74,75,76,706,706,1154,706,706,706,706,706,706,706,706,706,707,834,834,835,644,644,644,833,835,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,1094,1095,1287,1032,706,706,138,203,414,415,76,1025,706,706,74,75,76,706,706,706,706,706,706,706,706,706,706,706,706,771,644,644,644,644,644,644,644,644,644,644,705,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1158,1224,1095,1287,1032,706,706,138,203,415,266,11,11,11,267,75,76,1090,706,706,706,1154,706,706,1025,706,706,706,707,835,641,643,644,644,644,644,644,644,3095,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,694,706,706,706,1025,1089,1158,1159,1159,1160,706,1025,706,138,139,203,223,223,223,223,223,266,12,706,1217,1218,1219,1025,1025,706,706,706,706,771,644,705,771,644,644,644,644,644,644,641,642,706,706,770,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,694,694,706,706,1025,1025,1025,1025,1025,1025,706,706,706,706,706,138,139,139,139,200,223,223,76,1153,1281,1282,1283,1025,1025,706,706,706,707,835,641,707,835,644,644,644,641,771,641,706,770,770,771,834,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,694,706,706,706,706,706,1090,706,706,706,706,706,706,1025,1025,706,706,706,1026,706,138,203,223,76,706,1281,1282,1283,1025,1025,1025,1025,1025,771,644,769,771,644,644,644,641,706,835,769,771,834,834,835,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,1025,706,706,706,706,706,706,706,706,74,223,266,12,1345,1346,1347,706,706,706,706,771,835,644,833,835,644,641,642,706,771,644,833,835,641,642,642,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,706,706,706,706,706,1025,706,1154,706,706,706,1025,706,138,203,223,266,11,12,706,1025,706,706,771,835,644,644,644,644,644,769,770,771,835,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,706,706,74,223,223,223,266,11,11,12,771,835,644,644,644,3095,644,644,833,834,835,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,706,706,1154,138,203,223,223,223,223,223,707,835,644,644,644,644,644,644,644,641,642,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,1090,706,706,1025,706,706,706,706,706,706,706,138,203,223,202,139,139,771,644,644,644,644,641,642,643,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,706,706,706,706,706,138,139,140,1154,771,835,644,644,644,644,705,1026,707,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,1025,706,706,1025,706,706,706,706,706,706,706,1089,706,706,1025,706,1154,706,771,835,644,644,644,644,641,1153,1089,771,644,705,706,706,706,706,706,706,706,706,694,694,706,706,706,706,706,706,706,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,706,771,835,644,2950,644,644,644,769,770,771,835,641,706,706,706,706,706,706,706,706,694,694,694,706,706,706,706,694,706,706,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,1025,706,706,706,706,706,706,706,707,835,644,644,644,644,644,644,833,834,835,641,706,1154,706,706,706,706,706,706,694,694,694,706,706,706,694,694,694,694,694,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,1025,706,706,706,1025,706,706,706,706,706,1025,706,706,706,706,771,2885,644,644,644,644,644,644,641,642,642,706,706,706,706,706,706,706,706,706,694,694,694,706,706,694,694,706,706,694,706,706,706,706,706,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,770,770,771,835,2949,644,644,644,644,2885,641,706,74,266,706,706,706,706,706,706,706,706,706,706,694,694,694,694,694,706,706,706,694,706,706,706,706,706,1612,1549,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,770,771,834,834,835,644,644,644,644,644,644,2949,705,706,138,203,266,11,12,706,706,706,706,706,706,706,694,694,694,694,694,706,706,706,694,694,706,706,706,694,1612,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,1025,706,706,706,706,706,706,706,706,707,834,835,3095,644,644,644,644,644,644,644,644,641,706,1154,706,74,75,75,266,12,706,706,706,1026,706,706,706,706,706,694,694,694,694,694,706,706,706,706,694,694,2251,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,707,644,641,643,644,644,644,644,644,641,643,644,705,706,706,706,138,203,75,75,266,12,706,706,706,706,706,706,706,706,706,694,706,706,706,706,706,706,706,706,706,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,11,12,706,706,706,1154,706,707,644,769,771,644,644,644,644,644,705,706,642,706,706,706,706,706,706,203,268,75,266,12,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,645,706,706,706,706,1091,202,140,706,706,706,770,770,771,644,833,835,644,644,644,644,644,769,706,706,706,706,706,706,706,706,138,203,75,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,1393,1025,706,646,645,706,706,1089,1155,1157,706,706,706,771,834,834,835,644,644,644,644,644,644,644,644,833,705,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1548,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1136,1073,1138,706,706,1027,1029,706,706,706,706,706,770,771,835,2885,3095,644,644,644,644,644,644,644,644,644,644,705,706,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1136,1137,1138,1027,1028,1092,1092,1029,1153,1090,1026,771,834,835,644,2949,644,644,644,644,644,644,644,644,641,642,642,706,706,706,706,706,706,706,706,706,706,706,138,203,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1548,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1200,1201,1202,1092,1092,1092,1092,1157,770,771,834,835,2950,644,644,644,644,644,644,644,644,644,644,2885,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,266,12,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,1612,3536,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1309,1310,1092,1092,1092,1156,1157,771,834,835,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,1373,1374,1155,1156,1157,706,771,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1440,1441,706,706,706,771,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,1089,706,706,1612,1612,1612,1612,1612,1612,1612,2433,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,771,835,3095,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,705,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,771,835,644,2885,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,75,266,12,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,770,771,835,2885,644,2949,644,644,644,644,644,644,644,644,644,644,644,644,3095,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,75,76,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1548,1549,706,1547,1548,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,707,834,835,644,2949,644,644,644,644,644,644,644,644,644,644,644,644,644,2885,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,266,12,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,2761,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1548,1612,1612,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,770,771,2885,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,76,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,707,834,835,2949,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,3095,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,76,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,3536,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,694,694,706,706,706,706,706,706,771,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,266,12,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1613,706,706,706,706,706,706,706,706,694,694,706,706,706,706,706,707,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,1612,2253,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,694,706,706,706,706,706,706,707,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,643,769,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,707,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2885,705,707,833,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,266,12,706,706,706,706,706,706,706,706,1612,1612,1612,1612,2194,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2257,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,707,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,705,707,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,1026,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1613,706,706,706,706,706,706,706,694,706,706,706,706,706,771,644,3095,644,644,644,644,644,644,644,644,644,644,644,644,644,3095,641,706,707,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2256,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,707,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2885,644,705,706,706,642,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,2702,2703,1612,1612,1612,1612,1612,1612,2049,2050,2050,2050,2051,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1613,706,706,706,706,706,706,706,706,706,706,1089,707,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,266,12,706,706,706,706,706,706,1153,2766,2767,1612,1612,1612,1612,1612,1612,2113,1612,1612,2130,1612,2051,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2056,1612,1549,706,706,706,706,706,706,706,706,706,770,771,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,268,75,266,12,706,706,706,706,706,706,1612,1612,1612,1612,1612,1612,2049,2050,1612,1612,1612,1612,1612,1612,2050,2050,2051,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1548,1549,706,706,706,706,706,706,707,834,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,74,75,202,203,75,266,12,706,706,706,706,706,1612,1612,1612,1612,1612,1612,2113,1612,1612,1612,1612,1612,1612,1612,1612,2252,2115,1612,1612,1612,1612,1612,1612,1612,1612,1612,2442,1612,1612,1612,1548,1548,1548,1549,706,706,771,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,138,203,75,76,706,706,706,706,706,1612,1612,1612,1612,1612,1612,2113,2433,1612,1612,1612,1612,1612,1612,1612,1612,2115,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2255,1612,1613,706,771,835,644,644,644,644,644,644,644,644,644,644,644,644,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,202,140,706,74,75,266,706,706,706,706,706,1551,1551,1552,1612,1612,1612,2177,1612,1612,1612,2248,1612,1612,1612,2178,2438,2179,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,771,835,3095,2885,644,644,644,644,644,644,644,644,644,644,641,642,642,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,202,140,706,706,138,203,75,266,12,706,706,706,1615,1615,1615,1552,1612,1612,2241,2177,2178,1612,1612,1612,1612,2179,2242,2502,2243,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,803,835,2885,644,2949,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,74,268,75,76,706,706,706,1678,1615,1615,1615,1552,1612,2305,2241,2242,2177,2178,2178,2179,2243,2371,2566,2307,1612,1612,1612,1612,1612,1612,3589,1612,1612,1612,1612,1612,1612,1612,1612,1612,2883,644,2949,644,644,644,644,644,644,644,644,644,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,202,140,706,706,706,706,138,203,75,266,12,706,706,1612,1678,1615,1615,1615,1552,1612,2305,2306,2241,2242,2242,2243,2307,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2882,1612,1612,1612,803,2947,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,10,267,76,706,706,706,706,706,706,74,75,75,76,706,706,1612,1612,1678,1615,1615,1615,1552,2130,1612,2305,2306,2306,2307,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2883,2946,2881,1612,1612,2883,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,202,140,706,706,706,706,706,706,138,203,75,76,706,706,1612,3536,1612,1678,1679,1679,1615,1551,1552,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2883,2947,644,2945,802,803,2947,644,644,644,644,644,644,644,644,644,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,76,706,706,706,706,706,706,706,706,74,75,266,12,706,1612,1612,1612,1612,1612,1612,1678,1679,1680,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,803,2947,644,2817,2818,1612,2883,644,644,644,644,644,644,644,644,644,644,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,140,706,706,706,706,706,706,706,706,74,75,75,76,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,803,644,644,802,2254,2883,2947,644,644,644,644,644,644,644,644,644,644,769,770,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,202,140,706,706,706,1153,706,706,706,706,706,138,203,75,266,706,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2066,1612,1612,1612,1612,1612,1612,1612,1612,2883,644,644,2881,2883,2947,644,644,644,644,644,644,644,644,644,644,644,833,834,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,202,140,706,706,706,706,706,706,706,706,706,706,706,138,203,75,266,2258,1612,1612,1612,1612,1612,1612,1612,2248,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,803,2947,644,644,2945,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,76,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,75,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,1612,2052,2053,2054,1612,1612,2253,1612,1612,1612,1612,1612,1612,803,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2885,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,76,706,706,706,706,706,706,706,706,706,706,706,706,706,138,203,75,1612,1612,2056,1612,1612,1612,1612,1612,1612,2052,2053,2053,2376,2181,2119,2054,1612,1612,1612,1612,1612,2314,1612,1612,2883,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,644,705,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,2054,1612,1612,1612,2061,1612,1612,1612,1612,2180,2508,2181,2116,2053,2118,2181,2054,1612,1612,1612,1612,1612,1612,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,3095,644,644,644,641,706,706,706,706,706,706,706,706,706,706,1217,1219,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,76,706,706,706,706,706,1154,706,706,706,706,706,706,706,706,706,706,2500,2053,2053,2054,1612,2066,1612,1612,2052,2181,2116,2053,2181,2181,2246,2507,2246,1612,1612,1612,1612,1612,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,644,644,705,706,706,706,706,706,706,706,706,706,706,1345,1347,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2507,2181,2181,2181,2054,1612,1612,1612,2180,2181,2180,2500,2247,2182,2374,2182,2243,1612,1612,1612,1612,803,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,771,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2445,2181,2181,2508,2182,1612,1612,1612,2244,2181,2180,2181,2181,2246,2445,2182,2307,1612,1612,2698,2882,2883,644,644,644,644,644,644,644,644,644,644,644,644,2885,644,644,641,771,835,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,11,267,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2181,2508,2181,2117,2182,1612,2130,1612,2241,2180,2244,2245,2246,2374,2181,2246,1612,1612,1612,803,2946,2947,644,644,644,644,644,644,644,644,644,644,644,644,2949,644,641,771,835,644,641,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,2181,2247,2181,2181,2181,2054,1612,1612,2305,2180,2381,2055,2374,2181,2182,2243,1612,1612,2882,2883,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,835,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2181,2181,2376,2181,2181,2182,1612,1612,1612,2180,2181,2181,2309,2181,2246,2307,1612,2883,2946,2947,2885,644,644,644,644,644,644,644,644,644,644,644,644,644,644,705,771,644,644,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2245,2181,2500,2181,2245,2246,1612,1612,2258,2180,2311,2375,2181,2246,2243,2434,2883,2947,644,644,2949,644,644,644,644,644,644,644,644,644,644,2885,644,641,642,771,835,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,10,267,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1154,706,706,2242,2244,2245,2246,2242,2243,1612,1612,1612,2244,2245,2245,2246,2243,2307,2883,2947,644,644,644,644,3095,644,644,644,644,644,644,644,644,644,2949,644,705,771,835,2950,644,705,1027,1029,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2306,2241,2242,2243,2306,2307,1612,1612,1612,2241,2242,2242,2243,2307,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,771,835,644,644,641,706,1091,75,1029,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,2305,2306,2307,1612,1612,1612,1612,1612,2305,2371,2306,2307,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,642,771,835,644,644,641,706,706,1155,1156,1157,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,74,75,76,706,706,706,706,706,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,2760,1612,1612,2120,2258,1612,1612,1612,1612,2882,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,707,835,644,644,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2054,1612,1612,2761,1612,1612,1612,1612,1612,1612,2883,2946,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,771,2885,644,644,769,770,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,75,75,76,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2182,1612,1612,1612,1612,1612,1612,2378,1612,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,706,771,835,2949,644,644,833,834,705,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,11,267,75,75,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,706,2182,1612,1612,1612,1612,1612,1612,1612,2883,2947,3095,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,705,706,771,835,644,641,642,642,643,3095,769,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,75,202,139,139,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2246,1612,3536,1612,1612,1612,2882,2883,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,643,644,769,771,835,644,641,706,706,706,707,644,833,705,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,10,267,75,202,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2243,1612,1612,1612,1612,2883,2946,2947,644,644,644,644,644,644,644,644,644,644,644,644,644,644,644,641,706,707,644,833,835,644,641,706,706,706,706,706,642,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,138,139,139,140,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2307,1612,1612,2882,2883,2947,644,644,2950,644,644,644,644,644,644,644,644,644,644,2885,644,641,642,706,706,771,644,644,644,644,769,706,706,706,115,116,115,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,1154,706,706,706,706,706,706,706,706,706,706,706,1612,1612,803,2946,2947,2885,644,644,644,644,644,644,644,644,644,644,644,644,644,2949,641,706,706,706,707,835,2885,641,643,644,833,705,706,178,184,248,179,183,186,116,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,3379,3380,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,2818,2819,2949,644,644,644,644,644,644,2885,644,644,644,644,644,641,642,706,706,706,706,707,644,2949,705,706,643,644,705,706,242,244,694,694,179,179,184,181,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,645,706,706,3379,3379,3380,3380,706,706,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1612,1612,1612,2882,2883,644,644,644,644,644,644,644,2949,644,644,644,641,642,706,706,706,706,706,706,771,644,641,706,706,707,644,769,706,306,250,179,179,694,694,179,245,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,3379,3379,3502,3504,3380,3380,645,645,646,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,3589,2882,2883,2946,2947,644,644,644,644,644,644,644,644,644,641,642,706,706,706,706,706,1154,706,707,835,644,705,706,706,707,644,833,705,370,311,250,248,179,179,308,309,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,646,645,706,706,3381,3502,3430,3189,3504,3637,646,706,645,706,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2883,2946,2947,644,644,644,644,644,644,644,644,641,642,642,706,706,706,1153,706,706,706,706,706,707,644,641,706,706,1090,706,643,644,769,706,706,817,818,819,820,372,373,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,646,645,646,646,706,706,3502,3506,3124,3184,3122,3504,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,2947,2885,3095,644,644,644,644,644,644,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,642,706,706,706,706,706,707,644,833,705,706,706,882,309,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,645,706,706,645,706,706,3125,3189,3124,3248,3122,3123,645,706,646,645,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1154,706,706,706,706,644,2949,644,644,644,644,644,641,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,642,642,706,706,706,946,373,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,135,136,520,2422,72,73,646,645,706,645,706,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,642,643,644,2885,644,641,642,706,706,706,706,706,706,706,706,1154,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,71,521,199,137,706,706,706,706,646,706,706,706,646,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,771,644,2949,641,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,135,136,137,706,645,706,646,706,706,706,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,771,835,2885,644,705,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,645,646,1547,1549,706,706,645,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,835,644,2949,641,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1025,706,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,645,706,706,706,706,706,706,706,706,706,1025,1547,1548,1548,1612,1612,1548,1549,706,646,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,641,642,642,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,646,706,706,706,706,706,706,706,706,1547,1548,1612,1612,1612,2185,1220,1221,1222,1548,1549,706,645,706,706,706,706,1026,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1217,1218,1218,1219,706,706,706,706,1547,1548,2249,1220,1221,1221,1221,1221,1416,1285,1286,1612,1612,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,1153,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1026,706,706,706,706,706,706,706,1154,706,706,706,1027,1028,1029,706,706,1154,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,706,706,706,706,706,706,706,706,706,706,1281,1282,1282,1283,706,706,706,706,1611,1220,1221,1416,1285,1285,1285,1285,1285,1285,1415,1222,1550,1552,1549,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1089,706,706,706,1029,706,1090,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1027,1028,268,1092,1093,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,1281,1282,1282,1283,706,706,706,706,1611,1284,1285,1285,1285,1285,1285,1285,1285,1285,1285,1286,1614,1616,1613,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,706,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,284,0,0,0,221,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,629,503,0,0,0,0,0,568,759,0,0,0,0,0,113,114,0,757,758,759,0,182,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,286,287,284,0,0,221,159,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,693,0,568,567,0,568,758,759,0,0,0,0,0,0,177,0,0,697,0,0,0,246,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,1998,1999,0,0,0,286,284,0,0,0,221,160,0,0,0,0,0,1021,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,757,758,759,693,0,695,0,0,0,0,0,0,0,0,241,0,0,0,0,0,0,310,0,697,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,287,284,0,0,221,159,160,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,629,379,758,759,0,0,0,0,0,0,0,0,305,0,0,0,0,0,0,374,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1421,1422,0,0,0,0,0,0,0,0,0,0,0,0,286,287,284,0,0,221,159,2280,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,1934,1935,0,696,0,0,0,0,0,629,503,695,0,0,0,0,0,0,0,0,0,0,369,0,0,0,0,436,437,0,1021,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,0,0,0,632,0,0,0,0,286,284,0,0,0,221,159,160,0,0,1934,1935,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,757,758,759,0,0,0,0,1934,1935,0,0,0,0,433,434,831,832,437,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,287,284,0,0,0,221,160,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,829,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,287,284,0,0,221,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1021,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,286,287,284,0,221,159,160,0,1934,1935,0,0,0,1995,1996,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,632,0,0,0,0,0,0,0,829,0,0,0,894,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,284,0,0,224,0,1998,1999,0,0,0,471,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,629,630,631,0,0,0,0,0,0,0,0,0,2150,286,284,0,221,159,159,159,159,159,160,535,0,0,0,0,0,0,0,0,1934,1935,0,0,829,0,0,0,0,0,0,0,0,0,0,0,0,0,892,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,693,0,504,631,0,0,0,0,0,0,0,0,0,0,286,287,284,0,0,0,0,0,221,160,0,1934,1935,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1020,0,0,0,0,957,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,629,503,0,2347,695,0,0,0,0,0,0,0,0,1421,1422,0,0,286,287,287,287,284,0,0,224,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,1421,1422,0,0,0,0,0,0,0,1998,1999,0,0,0,0,693,0,568,758,759,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,286,284,0,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1021,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,0,0,0,0,0,0,0,757,758,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,697,222,0,221,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1020,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,284,0,221,159,160,1021,1803,1804,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,894,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,1021,0,0,222,0,0,0,221,159,159,160,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,892,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,284,0,0,0,0,0,224,842,844,651,651,652,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,284,0,285,287,287,288,906,908,773,715,716,0,0,0,0,0,0,1161,0,0,0,0,0,0,0,629,631,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,286,287,288,0,0,0,0,0,907,908,0,0,841,844,845,0,0,0,0,0,0,0,629,630,630,503,695,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,905,908,909,0,0,0,1421,1422,0,629,503,0,0,568,759,0,629,630,631,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,711,0,0,0,0,0,0,0,1485,1486,629,503,0,0,0,695,629,630,503,0,504,630,631,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,775,0,0,0,0,0,0,0,0,0,693,0,0,0,568,443,503,0,0,0,0,0,695,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1421,1422,0,0,0,0,0,1161,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,2215,839,0,0,0,0,0,0,0,0,0,693,0,0,0,504,503,0,0,568,567,0,568,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,757,567,0,0,0,0,0,568,759,693,0,504,631,0,629,630,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,693,0,0,0,0,0,504,630,503,0,0,504,630,503,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,757,758,758,567,0,0,0,0,0,568,758,758,567,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1421,1422,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1803,1804,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,757,567,0,568,758,758,759,0,0,757,758,567,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,757,758,759,0,0,0,0,0,0,0,757,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,1329,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,697,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,1071,1072,0,1074,1075,0,0,0,0,0,0,0,696,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1135,0,0,0,1139,0,208,0,1161,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1135,0,0,0,1139,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,696,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1199,0,0,0,1203,0,0,0,0,0,0,711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3178,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1263,0,0,1311,1267,0,0,0,0,0,0,775,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,953,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,1421,1422,0,0,0,0,0,0,0,0,0,1375,0,0,0,0,0,0,0,839,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,955,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,954,955,0,0,0,0,0,0,0,1998,1999,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,938,0,0,1442,0,0,0,972,901,524,903,904,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,949,950,1018,1019,0,1013,1014,1015,0,0,0,0,0,0,0,954,955,0,0,954,955,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,965,966,967,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,954,955,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,1018,1019,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,1016,1017,0,1018,1019,0,0,954,955,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,949,950,951,0,1018,1019,0,0,0,0,949,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1161,0,0,0,0,0,0,0,0,3686,3687,3688,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,1013,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,955,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,629,630,630,631,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,829,0,0,0,0,0,0,1013,1014,1015,1016,1017,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,693,1934,1935,504,631,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,629,503,1998,1999,568,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,954,955,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1995,1996,0,0,0,0,0,0,1934,1935,0,0,757,567,0,568,759,1995,1996,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,949,950,951,0,0,0,0,0,0,0,2279,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1998,1999,0,0,0,757,758,443,630,631,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,1013,1014,1016,1017,0,0,0,1021,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,629,503,568,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,3048,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,693,0,695,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,1016,1017,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,1016,1017,0,1421,1422,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,693,568,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,954,955,0,0,0,1018,1019,0,0,0,0,954,955,0,0,0,1018,1019,0,0,0,0,0,0,0,1485,1486,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,757,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,949,950,951,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,1016,1017,0,0,0,0,1016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,1013,1014,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2280,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1803,1804,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,2215,0,0,0,0,0,0,0,1013,1014,1015,0,1016,1017,0,0,0,0,828,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,892,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,954,955,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,955,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,1016,1017,1018,1019,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,1018,1019,0,0,0,0,0,0,954,955,0,0,954,955,0,0,0,0,1016,1017,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,1018,1019,0,0,1018,1019,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,1018,954,955,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,954,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,1016,1017,0,0,0,1016,1017,0,0,0,0,1016,1017,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,954,955,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,829,0,0,0,1016,1017,955,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,950,951,0,0,0,0,0,0,0,0,0,0,0,0,2344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,949,950,951,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,1016,1017,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,954,955,1016,1017,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,1016,1017,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,954,955,0,0,0,0,0,1018,1019,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,954,955,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,829,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2278,0,0,0,954,955,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,1018,1019,955,1018,1019,950,951,0,0,0,0,0,0,0,0,0,0,0,2342,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,949,950,951,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2920,2921,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,1016,1017,0,0,1013,1014,1015,0,0,0,0,0,0,1018,1019,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2984,2985,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,1016,1017,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,1018,1019,0,0,1165,0,0,0,0,1016,1017,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,547,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,954,955,0,954,955,0,0,0,0,0,0,0,1165,0,0,0,0,1016,1017,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,611,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,1018,1019,0,1018,1019,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,954,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,675,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,1016,1017,0,1021,1016,1017,0,0,0,1018,1019,0,0,0,0,1165,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,1016,1017,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,1161,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,1018,1019,954,955,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,1018,1019,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2278,0,0,1013,1014,1015,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,954,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,2343,0,0,0,954,955,0,0,0,0,0,0,0,1018,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,51,52,51,57,58,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,954,955,0,0,0,1165,0,0,0,0,0,0,1016,1017,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,113,114,0,629,631,121,122,52,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,2151,0,0,0,0,1018,1019,0,0,0,0,0,1803,1804,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,0,629,503,504,631,0,0,117,118,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,1161,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,241,0,693,0,0,504,630,631,0,182,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,3445,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,954,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,305,0,757,758,567,1016,1017,695,0,246,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3509,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,369,0,0,0,757,758,758,759,0,310,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,433,434,375,0,0,0,0,0,0,374,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,697,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,439,881,0,0,884,436,437,438,0,0,696,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,945,0,0,948,949,950,951,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,1165,0,0,0,3046,3047,0,781,0,0,272,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,696,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1009,1010,437,1012,1013,1014,1015,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3178,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,955,0,0,0,954,955,0,0,0,0,0,2214,0,0,0,0,0,0,0,0,0,0,0,647,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,949,950,951,1018,1019,949,950,951,697,0,0,0,0,0,0,0,0,829,0,0,336,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,3179,0,949,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1013,1014,1015,0,0,1013,1014,1015,0,0,0,0,1165,0,0,0,0,0,0,0,400,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1165,0,0,0,0,1016,1017,0,0,0,0,0,1013,1014,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,955,0,0,0,0,0,0,0,0,0,1804,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,697,0,0,0,0,0,954,955,0,0,0,0,0,0,0,954,955,0,0,0,0,0,970,0,775,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1018,1019,0,0,0,0,954,955,0,1018,1019,0,1165,0,0,0,0,0,839,0,0,0,0,0,0,0,0,0,0,0,0,0,1016,1017,0,0,0,0,0,0,0,0,1165,0,0,0,0,0,0,0,0,0,0,1934,1935,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,0,0,0,1018,1019,950,951,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,950,951,0,0,1165,0,0,0,0,0,0,954,955,0,0,0,0,949,950,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,1805,1806,1807,1808,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,1933,0,0,1936,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,1867,1868,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,1931,1932,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,830,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,1933,0,0,1936,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,828,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,956,0,0,0,0,893,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,956,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,1739,1740,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,830,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,828,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,1933,0,0,1936,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1739,1740,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,952,953,0,0,0,0,0,0,0,888,889,0,0,0,0,1931,1932,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,827,952,953,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,888,952,953,827,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1264,1265,1266,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,821,952,890,891,0,885,886,887,0,0,0,0,0,0,0,826,827,0,0,826,827,952,953,890,891,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,826,827,0,0,0,885,886,954,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,890,891,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,888,889,0,890,891,0,0,826,827,0,951,0,0,0,0,888,889,0,0,0,0,0,0,0,821,822,823,0,890,891,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,968,0,0,0,0,0,0,0,0,0,0,0,890,952,953,0,0,0,0,0,890,891,0,0,0,0,0,0,952,953,826,827,0,0,0,0,0,885,886,887,0,954,0,0,0,0,0,885,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,955,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,827,0,0,0,952,953,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,888,889,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,954,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,826,952,953,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1867,1868,0,0,0,0,0,1805,1806,1807,1808,0,0,1933,0,0,1936,1867,1868,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,952,953,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1931,1932,0,0,0,0,0,1869,1870,1871,1872,0,0,0,0,0,0,1931,1932,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,885,886,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,952,953,827,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,952,953,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1015,890,891,0,0,888,889,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,888,889,0,952,953,827,0,0,0,890,891,0,0,0,0,826,827,0,0,0,890,891,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,821,822,952,953,0,0,890,891,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,888,889,0,0,0,0,888,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,885,886,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,952,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,952,953,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,1015,0,885,886,887,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,1739,1740,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,826,827,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,827,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,888,889,890,891,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,821,952,953,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,890,891,0,0,0,0,0,0,826,827,0,952,953,827,0,0,0,0,888,952,953,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,954,0,0,0,0,0,0,0,890,891,0,0,890,891,0,0,821,822,952,953,0,0,949,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,890,891,827,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,826,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,890,891,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,888,889,0,0,0,888,889,0,0,0,0,888,889,952,953,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,952,953,889,0,0,0,952,953,827,0,888,889,0,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,827,0,0,0,0,0,0,952,953,0,0,0,0,890,891,822,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,891,821,822,823,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,0,885,886,887,0,0,0,0,0,0,0,0,0,0,949,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,952,953,0,0,0,888,952,953,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,952,953,0,0,0,0,826,827,888,889,952,953,952,953,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,890,891,888,889,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,826,827,0,0,0,0,0,890,891,952,953,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,826,827,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,949,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,890,891,827,890,891,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,954,821,822,949,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,888,889,0,0,885,886,887,0,0,0,0,0,0,890,891,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,826,827,0,0,0,0,0,0,0,0,0,0,889,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,888,889,952,953,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,826,827,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,952,953,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,952,953,890,891,0,890,891,0,0,0,0,0,826,827,0,0,0,0,0,952,953,0,0,0,0,826,888,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,826,827,0,0,888,889,0,0,888,889,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,890,952,953,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,952,953,0,888,952,953,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,955,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,821,822,823,890,891,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,885,886,887,0,0,890,891,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,888,889,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,826,827,0,0,0,0,0,0,0,949,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,890,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1739,1740,0,0,0,0,0,0,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3315,3316,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,826,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3315,0,0,3316,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,827,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3315,0,0,0,0,3316,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,955,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,2982,2983,0,717,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,827,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,891,821,822,823,890,891,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,954,0,885,886,887,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,588,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,952,953,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,952,953,0,0,0,0,0,0,826,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,0,0,0,0,0,1740,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,826,827,0,890,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1805,1806,1807,1808,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,823,0,0,0,0,0,890,891,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,822,952,953,0,0,0,0,0,0,0,0,826,827,0,0,0,0,821,822,0,0,1869,1870,1871,1872,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,886,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,886,887,0,0,0,0,0,0,0,0,0,890,891,0,0,0,0,885,886,0,0,1933,0,0,1936,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,949,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,889,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,888,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,951,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,887,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,822,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,889,888,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,888,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,891,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,889,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,890,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,826,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,827,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,821,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,885,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  };
  int Layer,i,j,Cooldown=0;
  int PlayerFrame=8;//frame number in player animation times 10
  int MouseX,MouseY;//Mouse location in screen coordinates
  int KnownSpells=0;
  int SpellTypeLeft=0,SpellTypeRight=2;//SpellType for left and right mouse button, 6 = no spell
  int SpellType;
  int SwitchSpell;//Switch spell flag
  int CastSpell=0;//Cast spell flag
  int ScreenX,ScreenY;//Upper left corner of the screen expressed in map coordinates
  gPlayer.x=65*320+160;gPlayer.y=91*320+160;
  centreScreenOnPlayer(&ScreenX,&ScreenY);
  gPlayerOrientation=2;
  gPlayerHP=20;gPlayerMaxHP=20;
  gPlayerSkipFrames=0;
  gNumberOfProjectiles=0;
  for(i=0;i<MAX_PROJECTILE;i++)
    for(j=0;j<PROJECTILE_INFO;j++)
      gProjectileList[i][j]=0;
  gNumberOfEnemies=0;
  for(i=0;i<MAX_ENEMY;i++)
    for(j=0;j<ENEMY_INFO+1+3*MAX_INSTRUCTIONS;j++)
      gEnemyList[i][j]=0;
  
  for(i=0;i<MAX_ROW;i++)
    for(j=0;j<MAX_COLUMN;j++){//To determine if a tile is a movement/projectile obstruction, we look for a non-null tile in layer 1 then (if needed) in layer 0. If the tile is of type 3, it obstructs movement. If it is of type 4, it obstructs movement and projectiles.
      for(Layer=2;!Map[--Layer][i][j];);
      gMapMovementObstacle[i][j]=(gTileType[Map[Layer][i][j]]>2);
      gMapProjectileObstacle[i][j]=(gTileType[Map[Layer][i][j]]==4);
    }
  
  for(i=0;i<NUMBER_OF_SPELLS;i++){
    gHasSpell[i]=0;gSpellStock[i]=0;
  }
  
  gHasSpell[0]=1;KnownSpells++;gSpellStock[0]=100;
  gHasSpell[1]=1;KnownSpells++;gSpellStock[1]=200;
  gHasSpell[2]=1;KnownSpells++;gSpellStock[2]=100;
  gHasSpell[3]=1;KnownSpells++;gSpellStock[3]=100;
  
  gEnemyList[gNumberOfEnemies][0]=74*320;//1120
  gEnemyList[gNumberOfEnemies][1]=25*320;//1120
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=2*320+160;
  gEnemyList[gNumberOfEnemies][1]=71*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=11*320+160;
  gEnemyList[gNumberOfEnemies][1]=69*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=39*320+160;
  gEnemyList[gNumberOfEnemies][1]=85*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=66*320+160;
  gEnemyList[gNumberOfEnemies][1]=54*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=98*320+160;
  gEnemyList[gNumberOfEnemies][1]=4*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=91*320+160;
  gEnemyList[gNumberOfEnemies][1]=7*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=98*320+160;
  gEnemyList[gNumberOfEnemies][1]=9*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=91*320+160;
  gEnemyList[gNumberOfEnemies][1]=51*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=65*320+160;
  gEnemyList[gNumberOfEnemies][1]=16*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=52*320+160;
  gEnemyList[gNumberOfEnemies][1]=7*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=64*320+160;
  gEnemyList[gNumberOfEnemies][1]=1*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=8*320+160;
  gEnemyList[gNumberOfEnemies][1]=12*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=23*320+160;
  gEnemyList[gNumberOfEnemies][1]=23*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=47*320+160;
  gEnemyList[gNumberOfEnemies][1]=33*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=31*320+160;
  gEnemyList[gNumberOfEnemies][1]=58*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=97*320+160;
  gEnemyList[gNumberOfEnemies][1]=39*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=54*320+160;
  gEnemyList[gNumberOfEnemies][1]=55*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=17*320+160;
  gEnemyList[gNumberOfEnemies][1]=97*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=19*320+160;
  gEnemyList[gNumberOfEnemies][1]=96*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=19*320+160;
  gEnemyList[gNumberOfEnemies][1]=98*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=22*320+160;
  gEnemyList[gNumberOfEnemies][1]=96*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=86*320+160;
  gEnemyList[gNumberOfEnemies][1]=84*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gEnemyList[gNumberOfEnemies][6]=0;
  gEnemyList[gNumberOfEnemies][7]=0;
  gEnemyList[gNumberOfEnemies][8]=11;
  gEnemyList[gNumberOfEnemies][9]=0;
  gEnemyList[gNumberOfEnemies][10]=500;
  gEnemyList[gNumberOfEnemies][11]=0;
  gEnemyList[gNumberOfEnemies][12]=0;
  gEnemyList[gNumberOfEnemies][13]=500;
  gEnemyList[gNumberOfEnemies][14]=-21;
  gEnemyList[gNumberOfEnemies][15]=21;
  gEnemyList[gNumberOfEnemies][16]=183;
  gEnemyList[gNumberOfEnemies][17]=-28;
  gEnemyList[gNumberOfEnemies][18]=-14;
  gEnemyList[gNumberOfEnemies][19]=23;
  gEnemyList[gNumberOfEnemies][20]=-30;
  gEnemyList[gNumberOfEnemies][21]=0;
  gEnemyList[gNumberOfEnemies][22]=75;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=8*320+160;
  gEnemyList[gNumberOfEnemies][1]=47*320+160;
  gEnemyList[gNumberOfEnemies][3]=5;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gEnemyList[gNumberOfEnemies][6]=0;
  gEnemyList[gNumberOfEnemies][7]=0;
  gEnemyList[gNumberOfEnemies][8]=11;
  gEnemyList[gNumberOfEnemies][9]=11;
  gEnemyList[gNumberOfEnemies][10]=64;
  gEnemyList[gNumberOfEnemies][11]=30;
  gEnemyList[gNumberOfEnemies][12]=0;
  gEnemyList[gNumberOfEnemies][13]=64;
  gEnemyList[gNumberOfEnemies][14]=21;
  gEnemyList[gNumberOfEnemies][15]=21;
  gEnemyList[gNumberOfEnemies][16]=61;
  gEnemyList[gNumberOfEnemies][17]=0;
  gEnemyList[gNumberOfEnemies][18]=30;
  gEnemyList[gNumberOfEnemies][19]=64;
  gEnemyList[gNumberOfEnemies][20]=-21;
  gEnemyList[gNumberOfEnemies][21]=21;
  gEnemyList[gNumberOfEnemies][22]=61;
  gEnemyList[gNumberOfEnemies][23]=-30;
  gEnemyList[gNumberOfEnemies][24]=0;
  gEnemyList[gNumberOfEnemies][25]=64;
  gEnemyList[gNumberOfEnemies][26]=-21;
  gEnemyList[gNumberOfEnemies][27]=-21;
  gEnemyList[gNumberOfEnemies][28]=61;
  gEnemyList[gNumberOfEnemies][29]=0;
  gEnemyList[gNumberOfEnemies][30]=-30;
  gEnemyList[gNumberOfEnemies][31]=64;
  gEnemyList[gNumberOfEnemies][32]=21;
  gEnemyList[gNumberOfEnemies][33]=-21;
  gEnemyList[gNumberOfEnemies][34]=61;
  gNumberOfEnemies++;
  
  SDL_Event Event;//Event handler
  const Uint8 *KeyboardState=SDL_GetKeyboardState(NULL);
  while(1){
    LastUpdate=SDL_GetTicks();
    
    SwitchSpell=KnownSpells&&(KeyboardState[SDL_SCANCODE_LCTRL]||KeyboardState[SDL_SCANCODE_RCTRL]);//Need at least one spell to prevent infinite switch spell loop
    while(SDL_PollEvent(&Event)){
      switch(Event.type){
        case SDL_QUIT:
          return 0;
        case SDL_MOUSEBUTTONDOWN:
          if(SwitchSpell){
            switch(Event.button.button){
              case SDL_BUTTON_LEFT:
                do{//Switch left spell
                  if(++SpellTypeLeft>5)SpellTypeLeft=0;
                }while(gHasSpell[SpellTypeLeft]==0);
                break;
              case SDL_BUTTON_RIGHT:
                do{//Switch right spell
                  if(++SpellTypeRight>5)SpellTypeRight=0;
                }while(gHasSpell[SpellTypeRight]==0);
                break;
            }
          }
          break;
      }
    }
    
    if(!SwitchSpell){//Can canst only when not changing spell
      if(SDL_GetMouseState(&MouseX,&MouseY)&SDL_BUTTON(SDL_BUTTON_LEFT)){
        if(Cooldown==0)CastSpell=gHasSpell[SpellType=SpellTypeLeft];//Will cast left spell
      }
      if(SDL_GetMouseState(&MouseX,&MouseY)&SDL_BUTTON(SDL_BUTTON_RIGHT)){
        if(Cooldown==0)CastSpell=gHasSpell[SpellType=SpellTypeRight];//Will cast right spell
      }
    }
    
    if(CastSpell){
      CastSpell=0;//Reset the CastSpell flag
      if(gSpellStock[SpellType]){
        Cooldown=gProjectileCooldown[SpellType];//Set the Cooldown counter
        gSpellStock[SpellType]--;
        gProjectileList[gNumberOfProjectiles][0]=gPlayer.x;
        gProjectileList[gNumberOfProjectiles][1]=gPlayer.y;
        gProjectileList[gNumberOfProjectiles][5]=ScreenX+MouseX;
        gProjectileList[gNumberOfProjectiles][6]=ScreenY+MouseY;
        gProjectileList[gNumberOfProjectiles][8]=SpellType;
        gProjectileList[gNumberOfProjectiles][9]=0;
        computeProjectileVelocityAndSteps(gNumberOfProjectiles);
        gProjectileList[gNumberOfProjectiles][7]=computeProjectileDirection(gNumberOfProjectiles);
        gPlayerOrientation=gPlayerDirection[gProjectileList[gNumberOfProjectiles][7]];
        gNumberOfProjectiles++;
        #ifdef AUDIO
        // Play the firing sound. This is just a test, since we only have two sounds right now.
        if(SpellType == eFire)
          playSound(gSoundFire, 0);
        else if(SpellType == eIce)
          playSound(gSoundIce, 0);
        #endif
      }
    }
    
    moveEnemies();//test move enemies
    if(movePlayer()){
      PlayerFrame+=2;if(PlayerFrame>=90)PlayerFrame=10;//cycle animation frames PlayerFrame/10 in [1,8]
    }
    else{
      PlayerFrame=8;// PlayerFrame/10 = 0 --> static frame
    }
    centreScreenOnPlayer(&ScreenX,&ScreenY);
    
    moveProjectiles(Map);
    
    SDL_RenderClear(gRenderer);//Clear screen
    renderMap(ScreenX,ScreenY,1,Map);
    
    for(i=0;i<gNumberOfEnemies;i++){
      if(gEnemyList[i][7]==0)
        renderEnemy(ScreenX,ScreenY,8,9,i);//render the enemy if it was not hit
      else
        gEnemyList[i][7]--;
      
      if(gEnemyList[i][6])//Enemy cooldown
        gEnemyList[i][6]--;
      else if(ScreenX*10-240<gEnemyList[i][0]&&gEnemyList[i][0]<(ScreenX+SCREEN_WIDTH+24)*10&&ScreenY*10-20<gEnemyList[i][1]&&gEnemyList[i][1]<(ScreenY+SCREEN_HEIGHT+48)*10){
        makeEnemyShoot(i);
        gEnemyList[i][6]=gProjectileCooldown[6];
      }
    }
    
    renderProjectiles(ScreenX,ScreenY);
    if(gPlayerSkipFrames==0)
      renderPlayer(ScreenX,ScreenY,PlayerFrame/10,gPlayerOrientation);//render the player if it was not hit
    else
      gPlayerSkipFrames--;
    
    renderMap(ScreenX,ScreenY,0,Map);
    if(Hello>0){
      gTextX=gPlayer.x/10+12-ScreenX;
      gTextY=gPlayer.y/10-45-ScreenY;
      renderText(gTextX+5,"Hi,\n\nI'm Lya and this is our tech demo.\n\nYou've read the PDF?");
      Hello-=DELAY_BETWEEN_FRAMES;
    }
    
    /*if(gHasSpell[0]==0&&Hello<=0){//Example on how to add a spell
      gHasSpell[0]=1;
      KnownSpells++;
    }
    */
    
    if(gPlayerHP>0)renderHP(gPlayerHP,gPlayerMaxHP);else return displayGameOverScreen();
    if(KnownSpells)renderSpellInterface(SpellTypeLeft,SpellTypeRight);
    SDL_RenderPresent(gRenderer);//Update screen
    if(Cooldown)Cooldown--;//lower Cooldown if it is non-zero
    
    ElapsedTime=SDL_GetTicks()-LastUpdate;
    if(ElapsedTime<DELAY_BETWEEN_FRAMES){//Enforce a constant delay between frames
      SDL_Delay(DELAY_BETWEEN_FRAMES-ElapsedTime);
    }
  }
}


int main(int argc,char*argv[]){
  
  #ifdef AUDIO
  struct AudioCtx *const audioCtx = createAudioCtx();
  if(!audioCtx){
    printf("Failed to initialize audio!\n");
  }
  else
  #endif
  if(initializeSDL()){//Start up SDL and create window
    printf("Failed to initialize!\n");
  }
  else{
    if(loadEntityTypes(gRenderer)||loadTiles()||loadEnemy()||loadProjectiles()||loadLife()||loadFont()||loadTitle()||loadGameOver()
      #ifdef AUDIO
      ||loadAudio(audioCtx)
      #endif
      ){//Load tiles, player, enemy, projectiles, life, title, game over and font
      printf("Failed to load media!\n");
    }
    else{
      int MainLoopIsRunning=1;//Main loop running flag
      struct Map *map =
          loadMap("res/field_test.json", gRenderer, NULL);
      while(MainLoopIsRunning){//While application is running
        MainLoopIsRunning=displayTitleScreen();
        if(MainLoopIsRunning){
            MainLoopIsRunning=play();
        }
      }
    }
  }
  closeSDL();//Free resources and close SDL
  return EXIT_SUCCESS;
}