#pragma once

struct AudioCtx;
struct Sound;

struct AudioCtx *createAudioCtx();
void destroyAudioCtx(struct AudioCtx *ctx);

struct Sound *loadSound(struct AudioCtx *ctx, const char *path);
void destroySound(struct Sound *snd);

void playSound(struct Sound *snd, unsigned loop);
void stopSound(struct Sound *snd);

// Calling playSound will resume playback.
void pauseSound(struct Sound *snd);

void rewindSound(struct Sound *snd);
