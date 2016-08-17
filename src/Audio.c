#include "Audio.h"

#ifdef __APPLE__
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include <ogg/ogg.h>
#include <opus/opus.h>

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE (5760 * 2)
static const unsigned gSampleRate = 48000;

struct AudioCtx {
    ALCdevice *device;
    ALCcontext *context;
    short buffer[BUFFER_SIZE];
};

struct Sound {
    ALuint name;
};

struct AudioCtx *createAudioCtx(){
  struct AudioCtx *const ctx = malloc(sizeof(struct AudioCtx));
  ctx->device = alcOpenDevice(NULL);
  ctx->context = alcCreateContext(ctx->device, NULL);

  if(!(ctx->device && ctx->context)){
    free(ctx);
    return NULL;
  }

  alcMakeContextCurrent(ctx->context);

  alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
  alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
  alListener3f(AL_ORIENTATION, 0.0f, 0.0f, 0.0f);

  return ctx;
}

struct Sound *loadSound(struct AudioCtx *ctx, const char *path){
  ALuint name = 0;
  int err = 0;
  const unsigned short buffer_size = 8192;
  OpusDecoder *const decoder = opus_decoder_create(gSampleRate, 1, &err);
  if(!decoder)
    return NULL;

  FILE *const file = fopen(path, "rb");
  if(!file)
    return NULL;

  ogg_sync_state state;
  ogg_page page;
  ogg_stream_state stream;
  int inited = 0, eofed = 0;
        
  alGenSources(1, &name);
        
  if(err!=OPUS_OK)
    return NULL;
    
  err = ogg_sync_init(&state);
    
  while(!eofed){
    ogg_packet packet;
    while(ogg_sync_pageout(&state, &page) == 1){
      if(ogg_page_bos(&page)){
        ogg_stream_init(&stream, ogg_page_serialno(&page));
        inited = 1;
#ifndef NDEBUG
        printf("[Audio]Inited %u\n", name);
#endif
      }

      ogg_stream_pagein(&stream, &page);
    }

    eofed = feof(file);

    {
      char *const buffer = ogg_sync_buffer(&state, buffer_size);
      const unsigned short readin =
        fread(buffer, 1, buffer_size, file);
      ogg_sync_wrote(&state, readin);
#ifndef NDEBUG
      printf("[Audio]Read %i bytes into buffer of %i bytes\n", readin, buffer_size);
#endif
    }

    while(inited && ogg_stream_packetout(&stream, &packet) == 1){
      const int r = opus_decode(decoder, packet.packet, packet.bytes,
        ctx->buffer, BUFFER_SIZE, 0);
#ifndef NDEBUG
      printf("[Audio]Decoded %i bytes\n", r);
#endif
      /* Get an OpenAL buffer. If there are any fully processed, reuse one of them.
       * Otherwise, we will need to generate a new one. */
      if(r > 0){ /* r is the amount to read out */
        ALuint buffer = 0;
        ALint i = 0;

        alGetSourcei(name, AL_BUFFERS_PROCESSED, &i);
        if(i){
          alSourceUnqueueBuffers(name, 1, &buffer);
            
          /* If there are more than one buffers ready to be unqueued, we will simply
           * destroy them. This is mainly an issue with OS X's OpenAL. */
          if(--i){
            ALuint other[8];
#ifndef NDEBUG
            fprintf(stderr, "[Audio]Destroying %i buffers.\n", i);
#endif
            while(i){
              const unsigned to_delete = (i >= 8) ? 8 : i;
              alSourceUnqueueBuffers(name, to_delete, other);
              alDeleteBuffers(to_delete, other);
                
              i -= to_delete;
            }
          }
        }
        else{
          alGenBuffers(1, &buffer);
        }
            
        alBufferData(buffer, AL_FORMAT_MONO16, ctx->buffer, r, gSampleRate);
        alSourceQueueBuffers(name, 1, &buffer);
      } /* if(r > 0) */
        else switch(r){
          case OPUS_ALLOC_FAIL:
            fputs("[Audio]Allocation Failure (out of memory?)\n", stderr);
            break;
          case OPUS_BAD_ARG:
            fputs("[Audio]Invalid Args\n", stderr);
            break;
          case OPUS_BUFFER_TOO_SMALL:
            fputs("[Audio]Buffer Too Smal\n", stderr);
            break;
          case OPUS_INTERNAL_ERROR:
            fputs("[Audio]Internal libopus error\n", stderr);
            break;
          case OPUS_INVALID_PACKET:
            fputs("[Audio]Invalid Packet/bad file\n", stderr);
            break;
          case OPUS_UNIMPLEMENTED:
            fputs("Audio]Unimplemented (libopus is too old or too new)\n", stderr);
            break;
      }
    } /* if(ogg_stream_packetout(...)) */
  } /* while(!feof(file)) */

  /* Clean up if possible. This is mostly for the Creative AL. */
  {
    ALuint other[8];
    ALint i;
    alGetSourcei(name, AL_BUFFERS_PROCESSED, &i);
#ifndef NDEBUG
    fprintf(stderr, "[Audio]Cleaning up %i buffers.\n", i);
#endif
    while(i){
      const unsigned to_delete = (i >= 8) ? 8 : i;
      alSourceUnqueueBuffers(name, to_delete, other);
      alDeleteBuffers(to_delete, other);
      i -= to_delete;
    }
  }

  fclose(file);

  opus_decoder_destroy(decoder);

  struct Sound *const sound = malloc(sizeof(struct Sound));
  sound->name = name;
  return sound;
}

void destroySound(struct Sound *snd){
  alDeleteSources(1, &snd->name);
  free(snd);
}

void playSound(struct Sound *snd, unsigned loop){
  alSourcei(snd->name, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(snd->name);
}

void stopSound(struct Sound *snd){
  alSourceStop(snd->name);
}

// Calling playSound will resume playback.
void pauseSound(struct Sound *snd){
  alSourcePause(snd->name);
}

void rewindSound(struct Sound *snd){
  alSourceRewind(snd->name);
}
