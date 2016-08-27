#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

struct Map;

// An old map may be passed in to avoid reloading the images for the tilesets.
struct Map *loadMap(const char *path, SDL_Renderer *renderer, struct Map *reuse_images);
void destroyMap(struct Map *map);

// Characters are only obstructed by layer 0.
bool isObstructed(const struct Map *map, int pixelX, int pixelY, unsigned width, unsigned height);
bool isTileObstructed(const struct Map *map, int tileX, int tileY);

void drawMap(const struct Map *map, SDL_Renderer *renderer, int x, int y);
