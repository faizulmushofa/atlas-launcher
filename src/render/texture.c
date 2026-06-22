#include "texture.h"
#include <stdio.h>

SDL_GPUTexture* texture_create_solid(SDL_GPUDevice* device, float r, float g, float b, float a) {
    if (!device) return NULL;

    SDL_GPUTextureCreateInfo tex_info;
    SDL_zero(tex_info);
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = 1;
    tex_info.height = 1;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &tex_info);
    if (!texture) {
        printf("Gagal membuat solid texture: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_GPUCommandBuffer* cmd_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (cmd_buffer) {
        SDL_GPUColorTargetInfo color_target;
        SDL_zero(color_target);
        color_target.texture = texture;
        color_target.clear_color.r = r;
        color_target.clear_color.g = g;
        color_target.clear_color.b = b;
        color_target.clear_color.a = a;
        color_target.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(cmd_buffer, &color_target, 1, NULL);
        if (render_pass) {
            SDL_EndGPURenderPass(render_pass);
        }
        SDL_SubmitGPUCommandBuffer(cmd_buffer);
    } else {
        printf("Gagal mengambil command buffer untuk inisialisasi solid texture: %s\n", SDL_GetError());
        SDL_ReleaseGPUTexture(device, texture);
        texture = NULL;
    }

    return texture;
}
