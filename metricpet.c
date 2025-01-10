#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#define PIXEL_SIZE 8  // Size of each pixel in the pet's display grid
#define BATTERY_THRESHOLD 20
#define CONNECTION_CHECK_CMD "ping -c 1 google.com > /dev/null 2>&1"

// Pet mood states
typedef enum { HAPPY, NEUTRAL, SAD, ANGRY, SWEATING, TIRED, CONFUSED } Mood;

// Colors for different moods
const SDL_Color colors[] = {
    {255, 255, 255, 255}, // HAPPY - white
    {200, 200, 200, 255}, // NEUTRAL - grey
    {0, 0, 255, 255},     // SAD - blue
    {255, 0, 0, 255},     // ANGRY - red
    {255, 165, 0, 255},   // SWEATING - orange
    {169, 169, 169, 255}, // TIRED - grey
    {128, 0, 128, 255}    // CONFUSED - purple
};

// Hardcoded pixel art for the cat (3 frames for animation)
const int cat_frames[3][8][8] = {
    { // Frame 1
        {0, 1, 1, 0, 0, 1, 1, 0},
        {1, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Frame 2
        {0, 1, 1, 0, 0, 1, 1, 0},
        {1, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 1, 1},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Frame 3
        {0, 1, 1, 0, 0, 1, 1, 0},
        {1, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 1, 1},
        {0, 0, 1, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    }
};

// Function to get CPU usage (simplified)
double get_cpu_usage() {
    static long prev_idle = 0, prev_total = 0;
    long idle, total;
    double usage = 0.0;

    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("Failed to open /proc/stat");
        return 0.0;
    }

    long user, nice, system, idle_time;
    fscanf(file, "cpu  %ld %ld %ld %ld", &user, &nice, &system, &idle_time);
    fclose(file);

    idle = idle_time;
    total = user + nice + system + idle;

    long delta_idle = idle - prev_idle;
    long delta_total = total - prev_total;

    if (delta_total != 0) {
        usage = (1.0 - ((double)delta_idle / delta_total)) * 100.0;
    }

    prev_idle = idle;
    prev_total = total;

    return usage;
}

// Function to get RAM usage percentage
double get_ram_usage() {
    long total, free, available;
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Failed to open /proc/meminfo");
        return 0.0;
    }

    fscanf(file, "MemTotal: %ld kB\nMemFree: %ld kB\nMemAvailable: %ld kB", &total, &free, &available);
    fclose(file);

    return ((double)(total - available) / total) * 100.0;
}

// Function to check battery percentage
int get_battery_percentage() {
    int percentage = 100;
    FILE *file = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (file) {
        fscanf(file, "%d", &percentage);
        fclose(file);
    }
    return percentage;
}

// Function to check internet connection
bool is_connected() {
    return system(CONNECTION_CHECK_CMD) == 0;
}

// Function to draw the pet with animation and color based on mood
void draw_pet(SDL_Renderer *renderer, Mood mood, int frame) {
    const int (*cat)[8][8] = &cat_frames[frame % 3];
    SDL_SetRenderDrawColor(renderer, colors[mood].r, colors[mood].g, colors[mood].b, colors[mood].a);

    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((*cat)[y][x] == 1) {
                SDL_Rect rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void print_verbose_info(Mood mood, double cpu_usage, double ram_usage, int battery, bool connected) {
    const char *mood_names[] = {"HAPPY", "NEUTRAL", "SAD", "ANGRY", "SWEATING", "TIRED", "CONFUSED"};
    printf("Mood: %s\n", mood_names[mood]);
    printf("CPU Usage: %.2f%%\n", cpu_usage);
    printf("RAM Usage: %.2f%%\n", ram_usage);
    printf("Battery: %d%%\n", battery);
    printf("Internet Connection: %s\n", connected ? "Connected" : "Disconnected");
}

int main(int argc, char *argv[]) {
    bool verbose = false;
    if (argc > 1 && strcmp(argv[1], "--verbose") == 0) {
        verbose = true;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Cat Pet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    int frame = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        double cpu_usage = get_cpu_usage();
        double ram_usage = get_ram_usage();
        int battery = get_battery_percentage();
        bool connected = is_connected();

        Mood mood;
        if (!connected) {
            mood = CONFUSED;
        } else if (battery < BATTERY_THRESHOLD) {
            mood = TIRED;
        } else if (cpu_usage > 80.0) {
            mood = ANGRY;
        } else if (ram_usage > 80.0) {
            mood = SAD;
        } else if (cpu_usage > 60.0) {
            mood = SWEATING;
        } else {
            mood = HAPPY;
        }

        if (verbose) {
            print_verbose_info(mood, cpu_usage, ram_usage, battery, connected);
        }

        draw_pet(renderer, mood, frame);
        frame++;
        SDL_Delay(500);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

