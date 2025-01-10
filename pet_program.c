#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>

#define WINDOW_WIDTH 200
#define WINDOW_HEIGHT 200
#define PIXEL_SIZE 8  // Size of each pixel in the pet's display grid
#define GRID_SIZE 25  // 25x25 grid for the cat pixel art
#define BATTERY_THRESHOLD 20
#define EYE_GREEN_CONDITION 100  // Condition for green eyes when battery is fully charged
#define BLINK_INTERVAL 2500  // Interval in milliseconds for blinking
#define CPU_USAGE_THRESHOLD 80
#define RAM_USAGE_THRESHOLD 80
#define METRIC_UPDATE_INTERVAL 60000  // 1 minute in milliseconds
#define VERSION "1.0.0"

// Function prototypes
int get_battery_percentage();
double get_cpu_usage();
double get_ram_usage();
bool is_connected();
void generate_colored_grid(int grid[GRID_SIZE][GRID_SIZE], SDL_Renderer *renderer, SDL_Color color);
void parse_arguments(int argc, char *argv[], bool *verbose, bool *test);
void modify_grid(int grid[GRID_SIZE][GRID_SIZE], bool grey_body, bool red_eyes, bool yellow_eyes, bool green_eyes);
void draw_cat(SDL_Renderer *renderer, int grid[GRID_SIZE][GRID_SIZE]);
void print_help();
void print_version();

// Placeholder for cat pixel art frames and additional views
const int cat_frames[2][GRID_SIZE][GRID_SIZE] = {
    // Frame 0: Default cat sitting (add pixel art)
    {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    },
    // Frame 1: Blinking cat (add pixel art)
    {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

    }
};

const int sleepy_cat[GRID_SIZE][GRID_SIZE] = {
    // Sleepy cat pixel art (add pixel art)
    {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    }
};

// Function to get battery percentage
int get_battery_percentage() {
    FILE *fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (!fp) {
        fprintf(stderr, "Error reading battery percentage: %s\n", strerror(errno));
        return -1;
    }
    int battery;
    fscanf(fp, "%d", &battery);
    fclose(fp);
    return battery;
}

// Function to get CPU usage
double get_cpu_usage() {
    static long prev_idle = 0, prev_total = 0;
    long idle, total;
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        fprintf(stderr, "Error reading CPU usage: %s\n", strerror(errno));
        return 0.0;
    }
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    long user, nice, system, idle_now, iowait, irq, softirq;
    sscanf(buffer, "cpu  %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle_now, &iowait, &irq, &softirq);

    idle = idle_now;
    total = user + nice + system + idle_now + iowait + irq + softirq;

    double usage = (1.0 - (double)(idle - prev_idle) / (total - prev_total)) * 100.0;

    prev_idle = idle;
    prev_total = total;
    return usage;
}

// Function to get RAM usage
double get_ram_usage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        fprintf(stderr, "Error reading RAM usage: %s\n", strerror(errno));
        return 0.0;
    }
    double used_ram = (double)(info.totalram - info.freeram) / info.totalram * 100.0;
    return used_ram;
}

// Function to check internet connection
bool is_connected() {
    struct ifaddrs *ifaddr, *ifa;
    bool connected = false;

    if (getifaddrs(&ifaddr) == -1) {
        fprintf(stderr, "Error checking internet connection: %s\n", strerror(errno));
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            connected = true;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return connected;
}

// Function to parse command line arguments
void parse_arguments(int argc, char *argv[], bool *verbose, bool *test) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
            *verbose = true;
        } else if (strcmp(argv[i], "--test") == 0) {
            *verbose = true;
            *test = true;
        } else if (strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(0);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_help();
            exit(1);
        }
    }
}

// Function to print help message
void print_help() {
    printf("Usage: pet_program [OPTIONS]\n");
    printf("Options:\n");
    printf("  --verbose       Show detailed debug information\n");
    printf("  --test          Simulate changing metrics for testing\n");
    printf("  --version       Show program version\n");
    printf("  --help          Show this help message\n");
}

// Function to print version
void print_version() {
    printf("Pet Program Version %s\n", VERSION);
}

// Function to modify grid for high CPU, RAM, or no internet
void modify_grid(int grid[GRID_SIZE][GRID_SIZE], bool grey_body, bool red_eyes, bool yellow_eyes, bool green_eyes) {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grey_body) {
                if (grid[y][x] == 1) {
                    grid[y][x] = 2;  // Grey for no internet
                }
            } else if (green_eyes) {
                if (y >= 7 && y <= 8 && (x == 8 || x == 16)) {  // Eye positions
                    grid[y][x] = 5;  // Green for full battery
                }
            } else if (red_eyes) {
                if (y >= 7 && y <= 8 && (x == 8 || x == 16)) {  // Eye positions
                    grid[y][x] = 3;  // Red for high CPU
                }
            } else if (yellow_eyes) {
                if (y >= 7 && y <= 8 && (x == 8 || x == 16)) {  // Eye positions
                    grid[y][x] = 4;  // Yellow for high RAM
                }
            }
        }
    }
}

// Function to draw the cat with different colors based on system metrics
void draw_cat(SDL_Renderer *renderer, int grid[GRID_SIZE][GRID_SIZE]) {
    SDL_Color color;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer);

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            switch (grid[y][x]) {
                case 1:
                    color = (SDL_Color){255, 255, 255, 255};  // White
                    break;
                case 2:
                    color = (SDL_Color){169, 169, 169, 255};  // Grey
                    break;
                case 3:
                    color = (SDL_Color){255, 0, 0, 255};  // Red
                    break;
                case 4:
                    color = (SDL_Color){255, 255, 0, 255};  // Yellow
                    break;
                case 5:
                    color = (SDL_Color){0, 255, 0, 255};  // Green
                    break;
                default:
                    continue;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_Rect rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    bool verbose = false;
    bool test = false;
    parse_arguments(argc, argv, &verbose, &test);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Cat Pet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    int frame = 0;
    Uint32 last_blink_time = SDL_GetTicks();
    Uint32 last_metric_update = SDL_GetTicks();
    int test_state = 0;  // State counter for cycling through test scenarios

    int battery = 100;
    double cpu_usage = 0.0;
    double ram_usage = 0.0;
    bool connected = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_metric_update >= METRIC_UPDATE_INTERVAL) {
            if (test) {
                // Cycle through different test scenarios
                switch (test_state) {
                    case 0:
                        battery = 100;  // Full battery (green eyes)
                        cpu_usage = 0.0;
                        ram_usage = 0.0;
                        connected = true;
                        break;
                    case 1:
                        battery = 50;
                        cpu_usage = 90.0;  // High CPU (red eyes)
                        ram_usage = 0.0;
                        connected = true;
                        break;
                    case 2:
                        battery = 50;
                        cpu_usage = 0.0;
                        ram_usage = 90.0;  // High RAM (yellow eyes)
                        connected = true;
                        break;
                    case 3:
                        battery = 50;
                        cpu_usage = 0.0;
                        ram_usage = 0.0;
                        connected = false;  // No internet (grey body)
                        break;
                    default:
                        battery = 100;
                        cpu_usage = 0.0;
                        ram_usage = 0.0;
                        connected = true;
                        break;
                }
                test_state = (test_state + 1) % 4;
            } else {
                battery = get_battery_percentage();
                cpu_usage = get_cpu_usage();
                ram_usage = get_ram_usage();
                connected = is_connected();
            }
            last_metric_update = current_time;

            if (verbose) {
                printf("Battery: %d%%\n", battery);
                printf("CPU Usage: %.2f%%\n", cpu_usage);
                printf("RAM Usage: %.2f%%\n", ram_usage);
                printf("Internet Connection: %s\n", connected ? "Connected" : "Disconnected");
            }
        }

        bool green_eyes = (battery >= EYE_GREEN_CONDITION);
        bool grey_body = !connected;
        bool red_eyes = (cpu_usage >= CPU_USAGE_THRESHOLD);
        bool yellow_eyes = (ram_usage >= RAM_USAGE_THRESHOLD);
        bool sleepy_mode = (battery < BATTERY_THRESHOLD);

        // Ensure logging happens before updating test state
        last_metric_update = current_time;

        if (test) {
            // Cycle through different test scenarios
            switch (test_state) {
                case 0:
                    battery = 100;  // Full battery (green eyes)
                    cpu_usage = 0.0;
                    ram_usage = 0.0;
                    connected = true;
                    break;
                case 1:
                    battery = 50;
                    cpu_usage = 90.0;  // High CPU (red eyes)
                    ram_usage = 0.0;
                    connected = true;
                    break;
                case 2:
                    battery = 50;
                    cpu_usage = 0.0;
                    ram_usage = 90.0;  // High RAM (yellow eyes)
                    connected = true;
                    break;
                case 3:
                    battery = 50;
                    cpu_usage = 0.0;
                    ram_usage = 0.0;
                    connected = false;  // No internet (grey body)
                    break;
            }
            test_state = (test_state + 1) % 4;
        }

        if (current_time - last_blink_time >= BLINK_INTERVAL) {
            frame = (frame + 1) % 2;  // Toggle between frame 0 and 1 for blinking
            last_blink_time = current_time;
        }

        int current_grid[GRID_SIZE][GRID_SIZE];
        if (sleepy_mode) {
            memcpy(current_grid, sleepy_cat, sizeof(current_grid));  // Use sleepy cat grid
        } else {
            memcpy(current_grid, cat_frames[frame], sizeof(current_grid));  // Use regular/blinking grid
        }

        modify_grid(current_grid, grey_body, red_eyes && frame == 0, yellow_eyes && frame == 0, green_eyes && frame == 0);
        draw_cat(renderer, current_grid);

        SDL_Delay(100);  // Delay to control frame rate
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

