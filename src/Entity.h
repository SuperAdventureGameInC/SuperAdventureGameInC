#pragma once

#include <SDL2/SDL.h>

struct EntityType {
    SDL_Texture *spriteset;
    unsigned boundsX, boundsY, boundsW, boundsH; // In tenths of a pixel.
    unsigned speed, diagSpeed;
};

const extern struct EntityType *const entityTypes;

#define PLAYER_ENTITY_TYPE 0
#define NUM_ENTITY_TYPES 1

struct Entity {
    int x, y; // Stored in tenths of a pixel.
    unsigned type;
};

unsigned getSpeed(const struct Entity *entity);
unsigned getDiagonalSpeed(const struct Entity *entity);
SDL_Texture *getSpriteset(const struct Entity *entity);

int loadEntityTypes(SDL_Renderer *renderer);
