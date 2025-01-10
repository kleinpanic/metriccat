# MetricPet

**MetricPet** is a fun, visual terminal program that displays a pixel-art pet cat whose appearance changes dynamically based on your system's metrics. The cat will display different moods and eye colors based on CPU usage, RAM usage, battery percentage, and internet connection status.

## Features

- **Dynamic Appearance**: The cat changes its eye color or body color based on system metrics:
  - **Green Eyes**: Full battery.
  - **Red Eyes**: High CPU usage.
  - **Yellow Eyes**: High RAM usage.
  - **Grey Body**: No internet connection.
- **Blinking Animation**: The cat blinks every few seconds.
- **Test Mode**: Simulates different system conditions to visualize how the pet reacts.
- **Verbose Mode**: Shows detailed debug information about current system metrics.

## Installation

1. Clone the repository and navigate to the directory.
2. Build and install the program using the following commands:
   ```bash
   make
   sudo make install
   ```
   This will move the compiled binary to `/usr/local/bin`, making it accessible from anywhere.

3. To clean up the build files, run:
   ```bash
   make clean
   ```

## Usage

```bash
metricpet [OPTIONS]
```

### Available Options

- `--verbose`: Show detailed debug information about system metrics.
- `--test`: Simulate changing metrics to see how the pet reacts.
- `--version`: Display the program version.
- `--help`: Display the help message.

## Example

```bash
metricpet --verbose
```

This command will display the pet while showing detailed metric information in the terminal.

```bash
metricpet --test
```

This command will simulate high CPU, high RAM, low battery, and no internet states in sequence.

## Requirements

- SDL2 library
- SDL2_image library

You can install these libraries using the following commands:

### On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev
```

### On Arch Linux:

```bash
sudo pacman -S sdl2 sdl2_image
```
---
