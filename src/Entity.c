#include "Entity.h"

#include <SDL2/SDL_image.h>

#include <assert.h>

struct EntityType lEntityTypes[NUM_ENTITY_TYPES];

const struct EntityType *const entityTypes = lEntityTypes;

int loadEntityTypes(SDL_Renderer *renderer){

  SDL_Surface *const loadedSurface=IMG_Load("res/Player.png");//Load PNG texture
  if(loadedSurface==NULL){
    printf("Unable to load image Player.png! SDL_image Error: %s\n", IMG_GetError());
  }
  else{//Create texture from surface pixels
    SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format,0,255,255));//Set the transparent pixel to cyan
    lEntityTypes[PLAYER_ENTITY_TYPE].spriteset = SDL_CreateTextureFromSurface(renderer,loadedSurface);
    if(lEntityTypes[PLAYER_ENTITY_TYPE].spriteset==NULL){
      printf("Unable to create texture from Player.png! SDL Error: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(loadedSurface);//Get rid of old loaded surface
  }
  
  lEntityTypes[PLAYER_ENTITY_TYPE].speed = 60;
  lEntityTypes[PLAYER_ENTITY_TYPE].diagSpeed = 60*71/100;
  
  if(lEntityTypes[PLAYER_ENTITY_TYPE].spriteset==NULL){
    printf("Failed to load texture image!\n");
    return 1;
  }
  
  return 0;
}

unsigned getSpeed(const struct Entity *entity){
  assert(entity->type < NUM_ENTITY_TYPES);
  return entityTypes[entity->type].speed;
}

unsigned getDiagonalSpeed(const struct Entity *entity){
  assert(entity->type < NUM_ENTITY_TYPES);
  return entityTypes[entity->type].diagSpeed;
}

SDL_Texture *getSpriteset(const struct Entity *entity){
  assert(entity->type < NUM_ENTITY_TYPES);
  return entityTypes[entity->type].spriteset;
}
