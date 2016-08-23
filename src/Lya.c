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
#define MAX_ROW 27//Needs to be greater or equal to 18 (SCREEN_HEIGHT/32)
#define MAX_COLUMN 31//Needs to be greater or equal to 25 (SCREEN_WIDTH/32)
#define DELAY_BETWEEN_FRAMES 20//Delay between frames in millisecond
#define PLAYER_SPEED 60//player speed in tenths of pixels per frame = speed [pixels per second] * delay between frames [millisecond] / 100
#define MAX_PROJECTILE 1000//maximum number of projectiles
#define PROJECTILE_INFO 10//number of information stored per projectile
#define MAX_ENEMY 100//maximum number of enemies
#define ENEMY_INFO 8//number of information stored per enemy
#define NUMBER_OF_SPELLS 6//number of spells in the game

const int gPlayerSpeedDiagonal=PLAYER_SPEED*71/100;
Uint16 gMap[MAX_ROW][MAX_COLUMN]={//gMap[i][j] is the index of the tile to display at (row=i, column=j).
  705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,644,705,645,644,705,705,705,705,705,705,705,705,705,643,643,643,
  705,644,644,705,705,705,705,705,705, 73, 74, 75,705,645,644,705,644,644,705,644,705,705,705,705,705,705,705,705,642,643,643,
  705,644,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,644,644,645,644,705,705,705,645,705,705,705,705,705,705,643,643,
  705,705,705,705,705,644,644,644,644,137,202,265, 11,705,705,705,644,705,645,705,705,705,705,705,705,705,705,705,705,643,643,
  705,705,705,705,705,644,705,705,644,705, 73, 74, 75,705,705,705,705,705,705,705,644,705,644,705,705,705,705,705,705,643,643,
  705,705,705,705,705,644,705,705,644,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,705,705,705,705,705,705,643,643,
  705,705,705,705,705,644,705,705,644,705,137,202,265, 11,705, 14, 15,705,705,705,705,705,705,705,705,705,705,705,705,643,643,
  644,705,705,705,705,644,644,644,644,705,705, 73, 74, 75,705, 78, 79,705,705,644,705,705,705,705,705,705,705,705,705,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,705,705,705,705,770,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,644,705,644,705,705,705,705,705,705,705,705,705,705,705,834,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,705,705,705,705,643,643,643,
  705,705,645,705,705,705,705,705,705,705,645, 73, 74, 75,644,705,644,705,705,705,705,705,645,705,705,705,705,705,643,643,643,
  705,705,705,705,705,644,705,705,705,705,705, 73, 74, 75,705,705,705,705,644,705,705,644,705,705,705,705,705,705,643,643,643,
  705,705,705,705,705,645,705,705,644,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,705,705,705,705,643,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,644,705,705,705,705,705,705,643,643,643,
  705,705,705,645,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,705,705,705,770,643,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,645,705,705,705,644,645,705,705,705,705,834,643,643,643,
  705,644,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,644,705,705,705,705,705,643,643,643,643,
  644,705,705,705,705,705,705,644,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,644,705,705,769,769,770,643,643,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,705,705,833,833,834,643,643,643,643,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,705,705,705,705,769,770,643,643,643,643,640,641,641,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,705,769,769,769,770,833,834,643,643,643,640,705,705,705,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,705,705,770,833,833,833,834,643,643,643,643,643,704,705,705,705,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,769,770,834,643,643,643,643,643,643,643,643,643,704,705,705,705,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,705,833,834,643,643,643,643,643,643,643,643,643,643,704,705,705,705,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,770,643,643,643,643,643,643,643,643,643,643,643,643,704,705,705,705,
  705,705,705,705,705,705,705,705,705,705,705, 73, 74, 75,834,643,643,643,643,643,643,643,643,643,643,643,643,704,705,705,705};
int gTextX,gTextY;//(x,y) coordinate of upper left corner of text to render
int gPlayerX,gPlayerY;//10 times the (x,y) coordinate of upper left corner of the player sprite
int gPlayerOrientation;//player orientation: 0 = UP or UP_LEFT, 1 = LEFT or DOWN_LEFT, 2 = DOWN or DOWN_RIGHT, 3 = RIGHT or UP_RIGHT
int gPlayerHP,gPlayerMaxHP;
int gPlayerSkipFrames;//To blink if the player get hit
int gNumberOfProjectiles;
int gProjectileList[MAX_PROJECTILE][PROJECTILE_INFO];//Each row of gProjectileList contains the following information about a projectile: PositionX*10, PositionY*10, VelocityX*10, VelocityY*10, TravelSteps, TargetX, TargetY, Direction (0 to 15), ProjectileType (0 to NUMBER_OF_SPELLS - 1 for spells, NUMBER_OF_SPELLS and above will serve for other projectiles), Caster (0=player,1=enemy)
int gNumberOfEnemies;
int gEnemyList[MAX_ENEMY][ENEMY_INFO];//Each row of gEnemyList contains the following information about a enemy: PositionX*10, PositionY*10, EnemyType, HP, Frame, Orientation, Cooldown, SkipFrame
const int gPlayerDirection[16]={3,3,3,0,0,0,0,1,1,1,1,2,2,2,2,3};//gPlayerDirection[ProjectileDirection] is the player direction when casting a spell in ProjectileDirection
int gMovementObstacle[1024]={//gMovementObstacle[TileIndex] = 1 if TileIndex correspond to a movement obstacle, = 0 if not
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int gProjectileObstacle[1024]={//gProjectileObstacle[TileIndex] = 1 if TileIndex correspond to a projectile obstacle, = 0 if not
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int gHasSpell[NUMBER_OF_SPELLS]={0};//Spells available to the player
int gSpellStock[NUMBER_OF_SPELLS]={0};//Number of spells the player has in stock for each spell
int gMaxSpellStock[NUMBER_OF_SPELLS]={200,400,200,200,200,200};//Maximum stock for each spell
int gProjectileSpeed[NUMBER_OF_SPELLS+1]={120,180,100,180,150,170,100};//speed in tenths of pixels per frame
int gProjectileCooldown[NUMBER_OF_SPELLS+1]={8,5,12,15,10,60,20};//Projectile cooldown in frames
SDL_Window *gWindow=NULL;//The window we'll be rendering to
SDL_Renderer *gRenderer=NULL;//The window renderer
SDL_Texture *gTextureTiles=NULL;//Tile sheet texture
SDL_Texture *gTexturePlayer=NULL;//Player sheet texture
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
      gRenderer=SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED);
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

int loadPlayer(void){
  SDL_Surface *loadedSurface=IMG_Load("res/Player.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image %s! SDL_image Error: %s\n","Player.png",IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    gTexturePlayer=SDL_CreateTextureFromSurface(gRenderer,loadedSurface);
    if(gTexturePlayer==NULL){
      printf("Unable to create texture from %s! SDL Error: %s\n","Player.png",SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  if(gTexturePlayer==NULL){
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
  SDL_DestroyTexture(gTexturePlayer);gTexturePlayer=NULL;//Free loaded player
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


void renderTile(int Row,int Column,int ScreenX,int ScreenY){
//Renders the tile at (Row,Column). The necessary information is read from gMap.
  SDL_Rect SourceRectangle;
  SourceRectangle.x=32*(gMap[Row][Column]%64);
  SourceRectangle.y=32*(gMap[Row][Column]/64);
  SourceRectangle.w=32;
  SourceRectangle.h=32;
  SDL_Rect DestinationRectangle;
  DestinationRectangle.x=32*Column-ScreenX;
  DestinationRectangle.y=32*Row-ScreenY;
  DestinationRectangle.w=32;
  DestinationRectangle.h=32;
  SDL_RenderCopy(gRenderer,gTextureTiles,&SourceRectangle,&DestinationRectangle);//Render tile to screen
}

void renderMap(int ScreenX,int ScreenY){
  int i,j;
  int MaxI=((ScreenY+SCREEN_HEIGHT)/32+((ScreenY+SCREEN_HEIGHT)%32>0));
  int MaxJ=((ScreenX+SCREEN_WIDTH)/32+((ScreenX+SCREEN_WIDTH)%32>0));
  for(i=ScreenY/32;i<MaxI;i++){
    for(j=ScreenX/32;j<MaxJ;j++){
      renderTile(i,j,ScreenX,ScreenY);
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
  
  SDL_RenderCopy(gRenderer,gTexturePlayer,&SrcRect,&DstRect);
  
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
  DstRect.x=gPlayerX/10-32-ScreenX;
  DstRect.y=(gPlayerY+HALF_SIDE_PLAYER_COLLISION_BOX)/10-64-ScreenY;
  DstRect.w=64;
  DstRect.h=64;
  SDL_RenderCopy(gRenderer,gTexturePlayer,&SrcRect,&DstRect);//Render player to screen
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
  int InitialCoordinate=gPlayerX;
  gPlayerX+=Speed;
  
  if(gPlayerX>MAX_COLUMN*320-BOUNDARY_OFFSET)gPlayerX=MAX_COLUMN*320-BOUNDARY_OFFSET;
  DestinationRow=gPlayerY/320;
  DestinationColumn=gPlayerX/320;
  RemainderRow=gPlayerY%320;
  RowOffsetOfAdjacentTileToCheck=(RemainderRow>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderRow<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile above, is 0 if we don't have to check a second adjacent tile, 1 if we have to check the tile below
  if(DestinationColumn<MAX_COLUMN && ( gMovementObstacle[gMap[DestinationRow][DestinationColumn+1]] || gMovementObstacle[gMap[DestinationRow+RowOffsetOfAdjacentTileToCheck][DestinationColumn+1]]) ){
    Boundary=320*(DestinationColumn+1)-BOUNDARY_OFFSET;
    if(gPlayerX>Boundary)gPlayerX=Boundary;
  }
  
  return gPlayerX!=InitialCoordinate;
}

int moveLeft(int Speed){
//Moves left and corrects gPlayerX for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderRow,RowOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayerX;
  gPlayerX-=Speed;
  
  if(gPlayerX<BOUNDARY_OFFSET)gPlayerX=BOUNDARY_OFFSET;
  DestinationRow=gPlayerY/320;
  DestinationColumn=gPlayerX/320;
  RemainderRow=gPlayerY%320;
  RowOffsetOfAdjacentTileToCheck=(RemainderRow>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderRow<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile above, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile below
  if(DestinationColumn>0 && ( gMovementObstacle[gMap[DestinationRow][DestinationColumn-1]] || gMovementObstacle[gMap[DestinationRow+RowOffsetOfAdjacentTileToCheck][DestinationColumn-1]]) ){
    Boundary=320*DestinationColumn+BOUNDARY_OFFSET;
    if(gPlayerX<Boundary)gPlayerX=Boundary;
  }
  
  return gPlayerX!=InitialCoordinate;
}

int moveDown(int Speed){
//Moves down and corrects gPlayerY for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderColumn,ColumnOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayerY;
  gPlayerY+=Speed;
  
  if(gPlayerY>MAX_ROW*320-BOUNDARY_OFFSET)gPlayerY=MAX_ROW*320-BOUNDARY_OFFSET;
  DestinationRow=gPlayerY/320;
  DestinationColumn=gPlayerX/320;
  RemainderColumn=gPlayerX%320;
  ColumnOffsetOfAdjacentTileToCheck=(RemainderColumn>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderColumn<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile to the left, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile to the right
  if(DestinationRow<MAX_ROW && ( gMovementObstacle[gMap[DestinationRow+1][DestinationColumn]] || gMovementObstacle[gMap[DestinationRow+1][DestinationColumn+ColumnOffsetOfAdjacentTileToCheck]]) ){
    Boundary=320*(DestinationRow+1)-BOUNDARY_OFFSET;
    if(gPlayerY>Boundary)gPlayerY=Boundary;
  }
  
  return gPlayerY!=InitialCoordinate;
}

int moveUp(int Speed){
//Moves up and corrects gPlayerY for collision if needed. Returns 1 if the coordinate changed, 0 otherwise.
  int DestinationRow,DestinationColumn,RemainderColumn,ColumnOffsetOfAdjacentTileToCheck,Boundary;
  int InitialCoordinate=gPlayerY;
  gPlayerY-=Speed;
  
  if(gPlayerY<BOUNDARY_OFFSET)gPlayerY=BOUNDARY_OFFSET;
  DestinationRow=gPlayerY/320;
  DestinationColumn=gPlayerX/320;
  RemainderColumn=gPlayerX%320;
  ColumnOffsetOfAdjacentTileToCheck=(RemainderColumn>320-HALF_SIDE_PLAYER_COLLISION_BOX)-(RemainderColumn<HALF_SIDE_PLAYER_COLLISION_BOX);//-1 if we have to check the tile to the left, is 0 if we don't have to check an adjacent tile, 1 if we have to check the tile to the right
  if(DestinationRow>0 && ( gMovementObstacle[gMap[DestinationRow-1][DestinationColumn]] || gMovementObstacle[gMap[DestinationRow-1][DestinationColumn+ColumnOffsetOfAdjacentTileToCheck]]) ){
    Boundary=320*(DestinationRow)+BOUNDARY_OFFSET;
    if(gPlayerY<Boundary)gPlayerY=Boundary;
  }
  
  return gPlayerY!=InitialCoordinate;
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
    MoveResult1=moveUp(gPlayerSpeedDiagonal);
    MoveResult2=moveLeft(gPlayerSpeedDiagonal);
    if(MoveResult1){gPlayerOrientation=0;return 1;}
    if(MoveResult2){gPlayerOrientation=1;return 1;}
    gPlayerOrientation=0;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]&&KeyboardState[SDL_SCANCODE_LEFT]){
    MoveResult1=moveDown(gPlayerSpeedDiagonal);
    MoveResult2=moveLeft(gPlayerSpeedDiagonal);
    if(MoveResult2){gPlayerOrientation=1;return 1;}
    if(MoveResult1){gPlayerOrientation=2;return 1;}
    gPlayerOrientation=1;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]&&KeyboardState[SDL_SCANCODE_RIGHT]){
    MoveResult1=moveDown(gPlayerSpeedDiagonal);
    MoveResult2=moveRight(gPlayerSpeedDiagonal);
    if(MoveResult1){gPlayerOrientation=2;return 1;}
    if(MoveResult2){gPlayerOrientation=3;return 1;}
    gPlayerOrientation=2;return 0;
  }
  if(KeyboardState[SDL_SCANCODE_UP]&&KeyboardState[SDL_SCANCODE_RIGHT]){
    MoveResult1=moveUp(gPlayerSpeedDiagonal);
    MoveResult2=moveRight(gPlayerSpeedDiagonal);
    if(MoveResult2){gPlayerOrientation=3;return 1;}
    if(MoveResult1){gPlayerOrientation=0;return 1;}
    gPlayerOrientation=3;return 0;
  }
  
  if(KeyboardState[SDL_SCANCODE_UP]){
    gPlayerOrientation=0;
    return moveUp(PLAYER_SPEED);
  }
  if(KeyboardState[SDL_SCANCODE_LEFT]){
    gPlayerOrientation=1;
    return moveLeft(PLAYER_SPEED);
  }
  if(KeyboardState[SDL_SCANCODE_DOWN]){
    gPlayerOrientation=2;
    return moveDown(PLAYER_SPEED);
  }
  if(KeyboardState[SDL_SCANCODE_RIGHT]){
    gPlayerOrientation=3;
    return moveRight(PLAYER_SPEED);
  }
  
  return 0;
}

void centreScreenOnPlayer(int*ScreenX,int*ScreenY){
  if(gPlayerX/10<=SCREEN_WIDTH/2)*ScreenX=0;
  else if(gPlayerX/10>=MAX_COLUMN*32-SCREEN_WIDTH/2)*ScreenX=MAX_COLUMN*32-SCREEN_WIDTH;
  else *ScreenX=gPlayerX/10-SCREEN_WIDTH/2;
  
  if(gPlayerY/10<=SCREEN_HEIGHT/2)*ScreenY=0;
  else if(gPlayerY/10>=MAX_ROW*32-SCREEN_HEIGHT/2)*ScreenY=MAX_ROW*32-SCREEN_HEIGHT;
  else *ScreenY=gPlayerY/10-SCREEN_HEIGHT/2;
}

void applyProjectileImpact(int TargetType,int ProjectileIndex){
/*Damage enemy or modify the tiles depending on the spell, removes the projectile from gProjectileList when done.
  TargetType:
   0 = empty tile (ground, building, ... )
   1 = the player
   2 = enemy index 0
   3 = enemy index 1
   ...
*/
  int i,j,TargetRow,TargetColumn;
  
  if(TargetType==0){
    TargetColumn=gProjectileList[ProjectileIndex][0]/320;
    TargetRow=gProjectileList[ProjectileIndex][1]/320;
    switch(gProjectileList[ProjectileIndex][8]){//ProjectileType
      case 0://fire spell
        switch(gMap[TargetRow][TargetColumn]){//Tile modifications
          case 11:case 73:case 75:case 137:case 202:case 265:case 705:case 734:
            gMap[TargetRow][TargetColumn]=74;break;//If the tile index was any for the above, it turns into tile 26 (=burned tile).
          case 644:gMap[TargetRow][TargetColumn]=645;break;
          case 645:case 733:gMap[TargetRow][TargetColumn]=705;break;
          case 730:gMap[TargetRow][TargetColumn]=640;break;
          case 732:gMap[TargetRow][TargetColumn]=642;break;
          case 794:gMap[TargetRow][TargetColumn]=832;break;
          case 795:gMap[TargetRow][TargetColumn]=833;break;
          case 796:gMap[TargetRow][TargetColumn]=834;break;
          case 798:gMap[TargetRow][TargetColumn]=643;break;
        }
        break;
      case 1://standard damage spell, doesn't modify terrain
        break;
      case 2://ice spell
        switch(gMap[TargetRow][TargetColumn]){//Tile modifications
          case 74:gMap[TargetRow][TargetColumn]=734;break;
          case 640:gMap[TargetRow][TargetColumn]=730;break;
          case 642:gMap[TargetRow][TargetColumn]=732;break;
          case 643:gMap[TargetRow][TargetColumn]=798;break;
          case 705:gMap[TargetRow][TargetColumn]=733;break;
          case 832:gMap[TargetRow][TargetColumn]=794;break;
          case 833:gMap[TargetRow][TargetColumn]=795;break;
          case 834:gMap[TargetRow][TargetColumn]=796;break;
        }
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        break;
    }
  }
  else if(TargetType==1){
    gPlayerHP--;
    gPlayerSkipFrames=2;//makes the player blink when hit
  }
  else{
    gEnemyList[TargetType-2][7]=2;//makes the enemy blink when hit
    if(--gEnemyList[TargetType-2][3]==0){//Reduce enemy HP and remove it if HP=0
      gNumberOfEnemies--;
      for(i=TargetType-2;i<gNumberOfEnemies;i++)
        for(j=0;j<ENEMY_INFO;j++)gEnemyList[i][j]=gEnemyList[i+1][j];
    }
  }
  
  {//Remove projectile
    gNumberOfProjectiles--;
    for(i=ProjectileIndex;i<gNumberOfProjectiles;i++)
      for(j=0;j<PROJECTILE_INFO;j++)gProjectileList[i][j]=gProjectileList[i+1][j];
  }
}

void moveProjectiles(void){
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
          applyProjectileImpact(j+2,i--);
          HasHit=1;
          break;
        }
      }
    if(HasHit==0){
      DeltaX=gProjectileList[i][0]-gPlayerX;
      DeltaY=gProjectileList[i][1]-gPlayerY;
      DestinationColumn=gProjectileList[i][0]/320;
      DestinationRow=gProjectileList[i][1]/320;
      if(gProjectileList[i][9]&&-190<DeltaX&&DeltaX<200&&-460<DeltaY&&DeltaY<140)//Checks if an enemy projectile hits the player
        applyProjectileImpact(1,i--);
      else if(gProjectileList[i][4]==-1||gProjectileObstacle[gMap[DestinationRow][DestinationColumn]])//Checks if the projectile reached its target or encountered an obstacle
        applyProjectileImpact(0,i--);
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
  gProjectileList[gNumberOfProjectiles][5]=gPlayerX/10;
  gProjectileList[gNumberOfProjectiles][6]=gPlayerY/10;
  gProjectileList[gNumberOfProjectiles][8]=6;
  gProjectileList[gNumberOfProjectiles][9]=1;
  computeProjectileVelocityAndSteps(gNumberOfProjectiles);
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
  int i,Cooldown=0;
  int PlayerFrame=8;//frame number in player animation times 10
  int MouseX,MouseY;//Mouse location in screen coordinates
  int KnownSpells=0;
  int SpellTypeLeft=0,SpellTypeRight=2;//SpellType for left and right mouse button, 6 = no spell
  int SpellType;
  int SwitchSpell;//Switch spell flag
  int CastSpell=0;//Cast spell flag
  int ScreenX,ScreenY;//Upper left corner of the screen expressed in map coordinates
  gPlayerX=5*SCREEN_WIDTH,gPlayerY=5*SCREEN_HEIGHT;
  centreScreenOnPlayer(&ScreenX,&ScreenY);
  gPlayerHP=20;gPlayerMaxHP=20;
  gNumberOfProjectiles=0;
  gNumberOfEnemies=0;
  gPlayerOrientation=2;
  gPlayerSkipFrames=0;
  
  gHasSpell[0]=1;KnownSpells++;gSpellStock[0]=100;
  gHasSpell[1]=1;KnownSpells++;gSpellStock[1]=200;
  gHasSpell[2]=1;KnownSpells++;gSpellStock[2]=100;
  gHasSpell[3]=1;KnownSpells++;gSpellStock[3]=100;
  
  gEnemyList[gNumberOfEnemies][0]=1120;
  gEnemyList[gNumberOfEnemies][1]=1120;
  gEnemyList[gNumberOfEnemies][3]=20;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gEnemyList[gNumberOfEnemies][6]=0;
  gEnemyList[gNumberOfEnemies][7]=0;
  gNumberOfEnemies++;
  gEnemyList[gNumberOfEnemies][0]=29*320;
  gEnemyList[gNumberOfEnemies][1]=25*320;
  gEnemyList[gNumberOfEnemies][3]=20;
  gEnemyList[gNumberOfEnemies][4]=1;
  gEnemyList[gNumberOfEnemies][5]=1;
  gEnemyList[gNumberOfEnemies][6]=0;
  gEnemyList[gNumberOfEnemies][7]=0;
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
        gProjectileList[gNumberOfProjectiles][0]=gPlayerX;
        gProjectileList[gNumberOfProjectiles][1]=gPlayerY;
        gProjectileList[gNumberOfProjectiles][5]=ScreenX+MouseX;
        gProjectileList[gNumberOfProjectiles][6]=ScreenY+MouseY;
        gProjectileList[gNumberOfProjectiles][8]=SpellType;
        gProjectileList[gNumberOfProjectiles][9]=0;
        computeProjectileVelocityAndSteps(gNumberOfProjectiles);
        gProjectileList[gNumberOfProjectiles][7]=computeProjectileDirection(gNumberOfProjectiles);
        gPlayerOrientation=gPlayerDirection[gProjectileList[gNumberOfProjectiles][7]];
        gNumberOfProjectiles++;
        #ifdef Audio
        // Play the firing sound. This is just a test, since we only have two sounds right now.
        if(SpellType == eFire)
          playSound(gSoundFire, 0);
        else if(SpellType == eIce)
          playSound(gSoundIce, 0);
        #endif
      }
    }
    
    if(movePlayer()){
      PlayerFrame+=2;if(PlayerFrame>=90)PlayerFrame=10;//cycle animation frames PlayerFrame/10 in [1,8]
    }
    else{
      PlayerFrame=8;// PlayerFrame/10 = 0 --> static frame
    }
    centreScreenOnPlayer(&ScreenX,&ScreenY);
    
    moveProjectiles();
    
    SDL_RenderClear(gRenderer);//Clear screen
    renderMap(ScreenX,ScreenY);
    
    for(i=0;i<gNumberOfEnemies;i++){
      if(gEnemyList[i][7]==0)
        renderEnemy(ScreenX,ScreenY,1,1,i);//render the enemy if it was not hit
      else
        gEnemyList[i][7]--;
      
      if(gEnemyList[i][6]==0){//Enemy cooldown
        makeEnemyShoot(i);
        gEnemyList[i][6]=gProjectileCooldown[6];
        }
      else gEnemyList[i][6]--;
    }
    
    renderProjectiles(ScreenX,ScreenY);
    if(gPlayerSkipFrames==0)
      renderPlayer(ScreenX,ScreenY,PlayerFrame/10,gPlayerOrientation);//render the player if it was not hit
    else
      gPlayerSkipFrames--;
    
    if(Hello>0){
      gTextX=gPlayerX/10+12-ScreenX;
      gTextY=gPlayerY/10-45-ScreenY;
      renderText(gTextX+5,"Hello FlyingJester !\n\nHow are you doing ?");
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
    if(loadTiles()||loadPlayer()||loadEnemy()||loadProjectiles()||loadLife()||loadFont()||loadTitle()||loadGameOver()
      #ifdef AUDIO
      ||loadAudio(audioCtx)
      #endif
      ){//Load tiles, player, enemy, projectiles, life, title, game over and font
      printf("Failed to load media!\n");
    }
    else{
      int MainLoopIsRunning=1;//Main loop running flag
      while(MainLoopIsRunning){//While application is running
        MainLoopIsRunning=displayTitleScreen();
        if(MainLoopIsRunning){
            struct Map map;
            loadMap("field_test.json", gRenderer, NULL);
            MainLoopIsRunning=play();
        }
      }
    }
  }
  closeSDL();//Free resources and close SDL
  return EXIT_SUCCESS;
}