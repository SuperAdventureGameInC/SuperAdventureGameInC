#include "Map.h"

#include "TurboJSON/object.h"
#include "TurboJSON/parse.h"
#include "bufferfile.h"

#include <SDL2/SDL_image.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

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

void drawLayer(const struct Tileset *tilesets, const struct Layer *layer,
    SDL_Renderer *renderer, int x, int y);

// Returns the key of the specified name, or NULL if it does not exist
static const struct Turbo_Value *jsonFindObjectValue(const struct Turbo_Value *value, const char *key){
    const unsigned keyLen = strlen(key);
    unsigned i;
    for(i = 0; i < value->length; i++){
        const struct Turbo_Property *const prop = value->value.object + i;
        if(prop->name_length == keyLen && memcmp(prop->name, key, keyLen) == 0){
            return &prop->value;
        }
    }
    return NULL;
}

static bool loadTileset(struct Tileset *tileset, SDL_Renderer *renderer, const struct Turbo_Value *tilesetValue, struct Map *reuse_images){
    unsigned i;
    
    assert(tilesetValue);
    assert(tileset);

    memset(tileset, 0, sizeof(struct Tileset));
    
    if(tilesetValue->type == TJ_Error){
        Turbo_WriteError(tilesetValue, stderr, 0);
        return false;
    }
    
    if(tilesetValue->type != TJ_Object){
        fprintf(stderr, "Invalid tileset object\n");
        return false;
    }
    
    {
        const struct Turbo_Value *const atlasWidth = jsonFindObjectValue(tilesetValue, "columns"),
            *const tileCount  = jsonFindObjectValue(tilesetValue, "tilecount"),
            *const tileWidth  = jsonFindObjectValue(tilesetValue, "tilewidth"),
            *const tileHeight = jsonFindObjectValue(tilesetValue, "tileheight"),
            *const tileData   = jsonFindObjectValue(tilesetValue, "tiles"),
            *const imagePath  = jsonFindObjectValue(tilesetValue, "image");
        
        if(!atlasWidth  || atlasWidth->type != TJ_Number ||
            !tileCount  || tileCount->type  != TJ_Number ||
            !tileWidth  || tileWidth->type  != TJ_Number ||
            !tileHeight || tileHeight->type != TJ_Number ||
            !tileData   || tileData->type   != TJ_Object ||
            !imagePath  || imagePath->type  != TJ_String){
            fprintf(stderr, "Invalid tileset format\n");
            return false;
        }
        
        tileset->tileWidth = (int)tileWidth->value.number;
        tileset->tileHeight = (int)tileHeight->value.number;
        tileset->atlasWidth = (int)atlasWidth->value.number;
        tileset->atlasHeight = ((int)tileCount->value.number) / tileset->atlasWidth;

        if(imagePath->length >= 0xFF - 1){
            fprintf(stderr, "Overlength tileset image path.\n");
            return false;
        }

        memcpy(tileset->imagePath, imagePath->value.string, imagePath->length);
        tileset->imagePath[imagePath->length] = '\0';

        // Normalize the path, because Tiled's JSON exporter is a bit jacked.
        {
            char *path = tileset->imagePath;
            while(*path){
                if(path[0] == '\\' && path[1] == '/'){
                    char *iter = path;
                    iter[0] = '/';
                    while(*iter){
                        iter[0] = iter[1];
                        iter++;
                    }
                }
                path++;
            }
        }
        
        tileset->obstruction = calloc(1, (int)tileCount->value.number);

        // Get obstruction data.
        for(i = 0; i < tileData->length; i++){
            // HACK: The Tiled JSON exporter will only print the names of tiles with attributes.
            const int index = atoi(tileData->value.object[i].name);
            assert(index < (int)tileCount->value.number);
            tileset->obstruction[index] = true;
        }
        
        tileset->tileHeight = (int)tileHeight->value.number;
    }

    // Load the tileset image, or reuse it from the map.
    if(reuse_images){
        for(i = 0; i < reuse_images->numTilesets; i++){
            if(strncmp(tileset->imagePath, reuse_images->tilesets[i].imagePath, 0xFF) == 0){
                tileset->image = reuse_images->tilesets[i].image;
                break;
            }
        }
    }
    if(!tileset->image){
        SDL_Surface *const surface = IMG_Load(tileset->imagePath);
        if(!surface){
            fprintf(stderr, "Could not load image %i for tileset\n", tileset->imagePath);
            return false;
        }
        
        tileset->image = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    return true;
}

static void destroyTileset(struct Tileset *tileset){
    free(tileset->obstruction);
    if(tileset->image)
        SDL_DestroyTexture(tileset->image);
}

static bool loadLayer(struct Layer *layer, const struct Turbo_Value *layerValue){
    
    assert(layerValue);
    assert(layer);

    memset(layer, 0, sizeof(struct Layer));
    
    if(layerValue->type == TJ_Error){
        Turbo_WriteError(layerValue, stderr, 0);
        return false;
    }
    
    if(layerValue->type != TJ_Object){
        fprintf(stderr, "Invalid layer object\n");
        return false;
    }
    
    {
        const struct Turbo_Value *const width = jsonFindObjectValue(layerValue, "width"),
            *const height = jsonFindObjectValue(layerValue, "height"),
            *const x      = jsonFindObjectValue(layerValue, "x"),
            *const y      = jsonFindObjectValue(layerValue, "y"),
            *const data   = jsonFindObjectValue(layerValue, "data");
        
        if(!width   || width->type  != TJ_Number ||
            !height || height->type != TJ_Number ||
            !x      || x->type      != TJ_Number ||
            !y      || y->type      != TJ_Number ||
            !data   || data->type   != TJ_Array){
            fprintf(stderr, "Invalid layer format\n");
            return false;
        }
        
        if(width->value.number <= 0 || height->value.number <= 0){
            fprintf(stderr, "Invalid layer dimensions\n");
            return false;
        }
        
        layer->width = (unsigned)width->value.number;
        layer->height = (unsigned)height->value.number;
        layer->offsetX = (unsigned)x->value.number;
        layer->offsetY = (unsigned)y->value.number;

        if(layer->width * layer->height != data->length){
            fprintf(stderr, "Layer dimensions do not match data length\n");
            return false;
        }
        
        layer->tileIndices = malloc(data->length * sizeof(unsigned));
        {
            unsigned i;
            for(i = 0; i < data->length; i++){
                if(data->value.array[i].type != TJ_Number){
                    fprintf(stderr, "Layer data index %i is not a number.\n", i);
                    free(layer->tileIndices);
                    layer->tileIndices = NULL;
                    return false;
                }
                if(data->value.array[i].value.number < 0.0){
                    fprintf(stderr, "Layer data index %i is negative.\n", i);
                    free(layer->tileIndices);
                    layer->tileIndices = NULL;
                    return false;
                }
                layer->tileIndices[i] = (unsigned)data->value.array[i].value.number;
            }
        }
    }
    
    return true;
}

static void destroyLayer(struct Layer *layer){
    layer->width = layer->height = 0;
    free(layer->tileIndices);
}

// An old map may be passed in to avoid reloading the images for the tilesets.
struct Map *loadMap(const char *path, SDL_Renderer *renderer, struct Map *reuse_images){
    // Load the 
    int size;
    struct Turbo_Value mapValue;
    struct Map *map = NULL;
    
    char *const data = BufferFile(path, &size);
    if(!data){
        fprintf(stderr, "Could not open map file %s\n", path);
        return NULL;
    }

    assert(path);

    Turbo_Parse(&mapValue, data, data + size);
    
    if(mapValue.type == TJ_Error){
        Turbo_WriteError(&mapValue, stderr, 0);
        goto ending;
    }

    if(mapValue.type != TJ_Object){
        fprintf(stderr, "Invalid map object\n");
        goto ending;
    }
    
    // Load the tilesets first.
    {
        unsigned i;
        const struct Turbo_Value *const tilesetArray = jsonFindObjectValue(&mapValue, "tilesets");
        if(!tilesetArray){
            fprintf(stderr, "Invalid map object: no tilesets\n");
            goto ending;
        }
        else if(tilesetArray->type == TJ_Error){
            Turbo_WriteError(&mapValue, stderr, 0);
            goto ending;
        }
        else if(tilesetArray->type != TJ_Array){
            fprintf(stderr, "Invalid tileset array\n");
            goto ending;
        }
        
        map = malloc(sizeof(struct Map));
        map->numTilesets = tilesetArray->length;
        map->tilesets = calloc(map->numTilesets, sizeof(struct Tileset));
        
        // Load all tilesets.
        for(i = 0; i < map->numTilesets; i++){
            if(!loadTileset(map->tilesets + i, renderer, tilesetArray->value.array + i, reuse_images)){
                // Free what we've loaded so far.
                unsigned e;
                for(e = 0; e != i; e++){
                    unsigned j;

                    // If we reused an existing image, don't touch it.
                    for(j = 0; j < reuse_images->numTilesets; j++){
                        if(reuse_images->tilesets[j].image == map->tilesets[e].image){
                            map->tilesets[e].image = NULL;
                            map->tilesets[e].imagePath[0] = '\0';
                            break;
                        }
                    }
                    destroyTileset(map->tilesets + e);
                }
                fprintf(stderr, "Error loading tileset %i\n", i);
                goto error_free_tilesets_only;
            }
        }

        // Change all reused surfaces to avoid any double-frees or use-after-frees.
        if(reuse_images){
            for(i = 0; i < map->numTilesets; i++){
                unsigned e;
                for(e = 0; e < reuse_images->numTilesets; e++){
                    if(reuse_images->tilesets[e].image == map->tilesets[i].image){
                        reuse_images->tilesets[e].image = NULL;
                        reuse_images->tilesets[e].imagePath[0] = '\0';
                    }
                }
            }
        }
    }

    // Load layers
    {
        const struct Turbo_Value *const layers = jsonFindObjectValue(&mapValue, "layers");
        if(!layers || layers->type != TJ_Array){
            // Huh. This is at least a warning.
            fprintf(stderr, "Warning: No layers in map.");
            map->numLayers = 0;
            map->layers = NULL;
        }
        else{
            unsigned i;
            map->numLayers = layers->length;
            map->layers = calloc(layers->length, sizeof(struct Layer));
            
            for(i = 0; i < map->numLayers; i++){
                if(!loadLayer(map->layers + i, layers->value.array + i)){
                    unsigned e;
                    for(e = 0; e < i; e++){
                        destroyLayer(map->layers + e);
                    }
                    free(map->layers);
                    goto error_free_tilesets;
                }
            }
        }
    }
    // All OK!
    goto ending;

error_free_tilesets:
    {
        unsigned i;
        for(i = 0; i < map->numTilesets; i++)
            destroyTileset(map->tilesets + i);
    }

error_free_tilesets_only:
    free(map->tilesets);

error_free_map:
    free(map);
    map = NULL;

ending:
    Turbo_FreeParse(&mapValue);
    FreeBufferFile(data, size);
    return map;
}

void destroyMap(struct Map *map);

// Characters are only obstructed by layer 0.
bool isObstructed(const struct Map *map, int pixelX, int pixelY, unsigned width, unsigned height);
bool isTileObstructed(const struct Map *map, int tileX, int tileY);

void drawMap(const struct Map *map, SDL_Renderer *renderer, int x, int y){
    unsigned i;
    if(map->numLayers == 0)
        return;
    drawLayer(map->tilesets, map->layers, renderer, x, y);
    // TODO: draw entities
    for(i = 1; i < map->numLayers; i++){
        drawLayer(map->tilesets, map->layers + i, renderer, x, y);
    }
}

void drawLayer(const struct Tileset *tileset, const struct Layer *layer,
    SDL_Renderer *renderer, int screenX, int screenY){

    // TODO: cull map drawing to screen.
    unsigned x, y;
    const int drawX = screenX - layer->offsetX, drawY = screenY - layer->offsetY;
    SDL_Rect source, dest;

    // We only need to set these values once.
    source.w = tileset->tileWidth;
    source.h = tileset->tileHeight;
    dest.w = tileset->tileWidth;
    dest.h = tileset->tileHeight;

    for(y = 0; y < layer->height; y++){
        for(x = 0; x < layer->width; x++){
            const unsigned raw_index = layer->tileIndices[x + (y * layer->width)],
                index = raw_index - 1;
            if(raw_index == 0 || raw_index - 1 >= tileset->atlasWidth * tileset->atlasHeight)
                continue;

            source.x = tileset->tileWidth * (index % tileset->atlasWidth);
            source.y = tileset->tileHeight * (index / tileset->atlasWidth);

            dest.x = (x * tileset->tileWidth) + drawX;
            dest.y = (y * tileset->tileHeight) + drawY;

            SDL_RenderCopy(renderer, tileset->image, &source, &dest);
        }
    }
}
