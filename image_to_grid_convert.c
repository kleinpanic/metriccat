#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GRID_SIZE 25  // 25x25 grid
#define PIXEL_THRESHOLD 128  // Threshold to decide between 0 and 1 for pixel intensity

void convert_image_to_grid(const char *image_path) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    SDL_Surface *image = IMG_Load(image_path);
    if (!image) {
        fprintf(stderr, "Failed to load image: %s\n", IMG_GetError());
        IMG_Quit();
        SDL_Quit();
        return;
    }

    int width = image->w;
    int height = image->h;

    SDL_Surface *scaled_image = SDL_CreateRGBSurface(0, GRID_SIZE, GRID_SIZE, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
    SDL_BlitScaled(image, NULL, scaled_image, NULL);
    SDL_FreeSurface(image);

    printf("const int grid[%d][%d] = {\n", GRID_SIZE, GRID_SIZE);

    for (int y = 0; y < GRID_SIZE; y++) {
        printf("    { ");
        for (int x = 0; x < GRID_SIZE; x++) {
            Uint32 pixel = ((Uint32 *)scaled_image->pixels)[y * GRID_SIZE + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, scaled_image->format, &r, &g, &b);
            int intensity = (r + g + b) / 3;
            printf("%d%s", intensity < PIXEL_THRESHOLD ? 0 : 1, x == GRID_SIZE - 1 ? "" : ", ");
        }
        printf(" }%s\n", y == GRID_SIZE - 1 ? "" : ",");
    }

    printf("};\n\n");

    printf("Pixel Art:\n");
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            Uint32 pixel = ((Uint32 *)scaled_image->pixels)[y * GRID_SIZE + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, scaled_image->format, &r, &g, &b);
            int intensity = (r + g + b) / 3;
            printf(intensity < PIXEL_THRESHOLD ? " " : "#");
        }
        printf("\n");
    }

    SDL_FreeSurface(scaled_image);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    convert_image_to_grid(argv[1]);

    return 0;
}

