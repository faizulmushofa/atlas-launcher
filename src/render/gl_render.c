#include "gl_render.h"
#include "texture.h"
#include <stdio.h>

/** Pointer statik ke perangkat GPU (SDL_GPUDevice) bawaan SDL3. */
static SDL_GPUDevice* gpu_device = NULL;

/** Tekstur 1x1 untuk warna background agar bisa di-blit sebagai isi jendela. */
static SDL_GPUTexture* bg_texture = NULL;

/**
 * Menginisialisasi perangkat SDL_GPU dan menautkannya ke window.
 * Fungsi ini membuat device GPU abstrak (SDL_GPUDevice) dan mengklaim window
 * utama agar dapat digunakan untuk proses rendering GPU.
 * 
 * @param window Pointer ke SDL_Window yang aktif.
 * @return true jika inisialisasi GPU berhasil, false jika gagal.
 */
bool gl_render_init(SDL_Window* window) {
    // Membuat SDL_GPUDevice dengan mendukung format shader SPIRV (Vulkan) dan MSL (Metal/macOS)
    gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, 
        true, 
        NULL
    );

    if (!gpu_device) {
        printf("Gagal membuat SDL_GPUDevice: %s\n", SDL_GetError());
        return false;
    }

    // Mengklaim window utama agar berada di bawah kendali rendering GPU device
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        printf("Gagal mengklaim window untuk GPU Device: %s\n", SDL_GetError());
        SDL_DestroyGPUDevice(gpu_device);
        gpu_device = NULL;
        return false;
    }

    // Membuat dan menginisialisasi tekstur background 1x1 menggunakan utilitas texture
    bg_texture = texture_create_solid(gpu_device, 0.12f, 0.14f, 0.18f, 1.0f);
    if (!bg_texture) {
        SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
        SDL_DestroyGPUDevice(gpu_device);
        gpu_device = NULL;
        return false;
    }

    printf("SDL_GPU berhasil diinisialisasi!\n");
    printf("Driver GPU yang aktif: %s\n", SDL_GetGPUDeviceDriver(gpu_device));

    return true;
}

/**
 * Membersihkan seluruh sumber daya renderer GPU.
 * Fungsi ini melepas tautan window dari perangkat GPU dan menghancurkan
 * objek SDL_GPUDevice yang aktif.
 * 
 * @param window Pointer ke SDL_Window tempat frame digambar.
 */
void gl_render_cleanup(SDL_Window* window) {
    if (bg_texture) {
        SDL_ReleaseGPUTexture(gpu_device, bg_texture);
        bg_texture = NULL;
    }
    if (gpu_device) {
        // Melepas tautan window dari perangkat GPU
        SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
        
        // Menghancurkan perangkat GPU
        SDL_DestroyGPUDevice(gpu_device);
        gpu_device = NULL;
    }
}

/**
 * Merender satu frame visual menggunakan SDL_GPU.
 * Fungsi ini membuat command buffer, mengambil tekstur swapchain window,
 * memulai render pass dengan warna latar gelap solid, lalu men-submit
 * perintah ke kartu grafis.
 * 
 * @param window Pointer ke SDL_Window tempat frame akan digambar.
 */
void gl_render_frame(SDL_Window* window) {
    if (!gpu_device || !bg_texture) return;

    // Memperoleh Command Buffer baru dari GPU device untuk antrean perintah GPU
    SDL_GPUCommandBuffer* cmd_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
    if (!cmd_buffer) {
        printf("Gagal mengambil GPU Command Buffer: %s\n", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain_texture = NULL;
    // Mengambil tekstur swapchain aktif dari window untuk digambar
    if (SDL_AcquireGPUSwapchainTexture(cmd_buffer, window, &swapchain_texture, NULL, NULL)) {
        if (swapchain_texture != NULL) {
            int w = 800, h = 100;
            SDL_GetWindowSizeInPixels(window, &w, &h);

            // Mengonfigurasi info blit untuk menggambar latar belakang dengan batas putih tipis
            SDL_GPUBlitInfo blit_info;
            SDL_zero(blit_info);

            // Sumber: tekstur 1x1 berwarna background
            blit_info.source.texture = bg_texture;
            blit_info.source.mip_level = 0;
            blit_info.source.layer_or_depth_plane = 0;
            blit_info.source.x = 0;
            blit_info.source.y = 0;
            blit_info.source.w = 1;
            blit_info.source.h = 1;

            // Tujuan: tekstur swapchain dengan inset 1 piksel di seluruh sisi
            blit_info.destination.texture = swapchain_texture;
            blit_info.destination.mip_level = 0;
            blit_info.destination.layer_or_depth_plane = 0;
            blit_info.destination.x = 1;
            blit_info.destination.y = 1;
            blit_info.destination.w = (Uint32)(w - 2);
            blit_info.destination.h = (Uint32)(h - 2);

            // Bersihkan seluruh tekstur tujuan (swapchain) ke warna putih sebelum blit dilakukan
            blit_info.load_op = SDL_GPU_LOADOP_CLEAR;
            blit_info.clear_color.r = 1.0f;
            blit_info.clear_color.g = 1.0f;
            blit_info.clear_color.b = 1.0f;
            blit_info.clear_color.a = 1.0f;

            blit_info.flip_mode = SDL_FLIP_NONE;
            blit_info.filter = SDL_GPU_FILTER_NEAREST;
            blit_info.cycle = false;

            // Lakukan blit (proses clear ke putih + stretch background ke area inset)
            SDL_BlitGPUTexture(cmd_buffer, &blit_info);
        }
    }

    // Men-submit antrean perintah di command buffer ke kartu grafis (GPU)
    SDL_SubmitGPUCommandBuffer(cmd_buffer);
}
