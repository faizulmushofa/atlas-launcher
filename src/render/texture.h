#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL3/SDL.h>

/**
 * Membuat tekstur 1x1 piksel dengan warna solid tertentu.
 * 
 * @param device Pointer ke perangkat GPU (SDL_GPUDevice).
 * @param r Nilai komponen warna Merah (0.0f - 1.0f).
 * @param g Nilai komponen warna Hijau (0.0f - 1.0f).
 * @param b Nilai komponen warna Biru (0.0f - 1.0f).
 * @param a Nilai komponen warna Alfa/Transparansi (0.0f - 1.0f).
 * @return Pointer ke SDL_GPUTexture yang baru dibuat, atau NULL jika gagal.
 */
SDL_GPUTexture* texture_create_solid(SDL_GPUDevice* device, float r, float g, float b, float a);

#endif // TEXTURE_H
