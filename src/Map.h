#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

struct Tileset {
    unsigned tileWidth, tileHeight;
    unsigned atlasWidth, atlasHeight;
    bool *obstruction;

    char imagePath[0xFF];
    SDL_Texture *image;
};

struct Layer {
    unsigned width, height;
    int offsetX, offsetY;
    unsigned *tileIndices;
};

struct Map {
    unsigned numLayers;
    struct Layer *layers;
    
    unsigned numTilesets;
    struct Tileset *tilesets;
};

// An old map may be passed in to avoid reloading the images for the tilesets.
struct Map *loadMap(const char *path, SDL_Renderer *renderer, struct Map *reuse_images);
void destroyMap(struct Map *map);

// Characters are only obstructed by layer 0.
bool isObstructed(const struct Map *map, int x, int y);
void drawMap(const struct Map *map, SDL_Renderer *renderer, int x, int y);
