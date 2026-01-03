# ESP32-TFT ElectricFish UI Project

A responsive touchscreen UI application for ESP32-based TFT displays, featuring a custom ElectricFish charging station interface with dynamic layout adaptation, touch interactions, and network capabilities.

## 📋 Table of Contents

- [Overview](#overview)
- [Hardware Specifications](#hardware-specifications)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Libraries Required](#libraries-required)
- [Troubleshooting](#troubleshooting)
- [License](#license)

## 🎯 Overview

This project implements a responsive user interface for ESP32-based TFT displays, specifically designed for ElectricFish charging station interfaces. The UI features:

- **Dynamic screen adaptation** - Automatically adjusts to actual screen dimensions after rotation
- **Touch interaction** - Full touchscreen support with calibrated coordinates
- **Responsive layout** - UI elements scale and position based on screen size
- **Custom UI components** - Connector selection cards, progress indicators, and status displays
- **Network capabilities** - WiFi connectivity for image fetching and future features

## 🔧 Hardware Specifications

### Supported Hardware

#### Option 1: ESP32-2432S028R (Cheap Yellow Display - CYD)
- **Display**: 2.8 inch TFT LCD
- **Resolution**: 240×320 pixels
- **Touch Controller**: XPT2046 resistive touchscreen
- **MCU**: ESP32-WROOM-32
- **Interface**: SPI
- **Purchase Link**: [Maker Advisor - CYD](https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/)
- **Setup Instructions**: [Random Nerd Tutorials - CYD Setup](https://RandomNerdTutorials.com/cyd/)

#### Option 2: ESP32 Dev Board + Separate TFT Display
- **ESP32 Board**: ESP32-WROOM-32 Development Board
- **Display**: 2.8 inch ILI9341 TFT Display (240×320)
- **Touch Controller**: XPT2046 (if touchscreen version)
- **Purchase Links**:
  - [ESP32 Dev Board](https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/)
  - [2.8" TFT Display](https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/)
- **Setup Instructions**: [Random Nerd Tutorials - ESP32 TFT](https://RandomNerdTutorials.com/esp32-tft/)

### Pin Configuration (ESP32-2432S028R)

#### TFT Display Pins
- **SPI Interface**: Uses default ESP32 SPI pins
- **Backlight**: Typically GPIO 27 (varies by board)

#### Touchscreen Pins (XPT2046)
- **T_IRQ**: GPIO 36
- **T_DIN (MOSI)**: GPIO 32
- **T_OUT (MISO)**: GPIO 39
- **T_CLK (SCK)**: GPIO 25
- **T_CS**: GPIO 33

### Technical Specifications

| Parameter | Value |
|-----------|-------|
| Display Size | 2.8 inches |
| Resolution | 240×320 pixels (portrait) / 320×240 pixels (landscape) |
| Color Depth | 16-bit (RGB565) |
| Touch Type | Resistive (XPT2046) |
| MCU | ESP32-WROOM-32 |
| Flash Memory | 4MB (minimum) |
| RAM | 520KB SRAM |
| WiFi | 802.11 b/g/n |
| Bluetooth | Bluetooth 4.2 |
| Operating Voltage | 3.3V |
| Current Consumption | ~80-240mA (depending on backlight) |

## ✨ Features

### UI Components
- **Header Bar**: Displays ElectricFish logo, current time, and battery level
- **Connector Selection Cards**: Interactive cards for NACS and CCS connector types
- **Progress Indicator**: 4-step progress bar (Start → Payment → Connect → Finish)
- **Version Display**: Shows firmware version in bottom-left corner

### Technical Features
- **Responsive Design**: UI adapts to screen dimensions (240×320 or 320×240)
- **Dynamic Layout**: Elements position based on actual screen size after rotation
- **Touch Calibration**: Automatic touch coordinate mapping
- **Memory Management**: Efficient heap allocation with SPIRAM support
- **PNG Image Support**: Display images from network or local storage
- **WiFi Connectivity**: Optional WiFi connection for network features

### Code Quality
- **Clean Architecture**: Modular function design
- **Responsive Variables**: Uses dynamic W/H instead of hardcoded dimensions
- **Text Rendering**: Standardized font handling (no mixing setTextSize with drawCentreString)
- **Error Handling**: Comprehensive error messages and fallbacks

## 📦 Prerequisites

### Software
- **Arduino IDE** (version 1.8.19 or later) or **PlatformIO**
- **ESP32 Board Support** for Arduino IDE
  - Add ESP32 board support: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- **USB Driver** (CH340/CP2102) if needed for your board

### Hardware
- ESP32-2432S028R or ESP32 Dev Board + TFT Display
- USB-C cable (for ESP32-2432S028R) or USB-A to Micro-USB cable
- Computer with USB port

## 🚀 Installation

### Step 1: Install Arduino IDE and ESP32 Support

1. Download and install [Arduino IDE](https://www.arduino.cc/en/software)
2. Open Arduino IDE → **File** → **Preferences**
3. Add ESP32 board manager URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "ESP32" and install "esp32 by Espressif Systems"
6. Select your board: **Tools** → **Board** → **ESP32 Arduino** → **ESP32 Dev Module**

### Step 2: Install Required Libraries

Install the following libraries via Arduino IDE Library Manager (**Sketch** → **Include Library** → **Manage Libraries**):

1. **TFT_eSPI** by Bodmer
   - Search: "TFT_eSPI"
   - Install: "TFT_eSPI" by Bodmer
   - ⚠️ **IMPORTANT**: You must configure `User_Setup.h` file. See [Configuration](#configuration) section.

2. **PNGdec** by Bodmer
   - Search: "PNGdec"
   - Install: "PNGdec" by Bodmer

3. **XPT2046_Touchscreen** by Paul Stoffregen
   - Search: "XPT2046_Touchscreen"
   - Install: "XPT2046_Touchscreen" by Paul Stoffregen

### Step 3: Configure TFT_eSPI Library

**CRITICAL**: The default `User_Setup.h` may not work with this project. You need to configure it properly.

1. Navigate to your Arduino libraries folder:
   - **Windows**: `C:\Users\[YourUsername]\Documents\Arduino\libraries\TFT_eSPI\`
   - **macOS**: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - **Linux**: `~/Arduino/libraries/TFT_eSPI/`

2. Open `User_Setup.h` in a text editor

3. Configure for ESP32-2432S028R:
   ```cpp
   #define USER_SETUP_LOADED
   #define ILI9341_DRIVER
   #define TFT_MISO 19
   #define TFT_MOSI 23
   #define TFT_SCLK 18
   #define TFT_CS   15
   #define TFT_DC    2
   #define TFT_RST   4
   #define TFT_BL   27
   #define LOAD_GLCD
   #define LOAD_FONT2
   #define LOAD_FONT4
   #define LOAD_FONT6
   #define LOAD_FONT7
   #define LOAD_FONT8
   #define LOAD_GFXFF
   #define SMOOTH_FONT
   ```

4. **Alternative**: Follow the detailed setup guide at [Random Nerd Tutorials - CYD Setup](https://RandomNerdTutorials.com/cyd/)

### Step 4: Upload Code

1. Clone or download this repository
2. Open `esp32-tft/sketch_jan2a/sketch_jan2a.ino` in Arduino IDE
3. Connect your ESP32 board via USB
4. Select the correct port: **Tools** → **Port** → Select your COM port
5. Click **Upload** button (or press `Ctrl+U` / `Cmd+U`)

## ⚙️ Configuration

### WiFi Settings (Optional)

If you want to enable WiFi features, edit the following in `sketch_jan2a.ino`:

```cpp
// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

**Note**: WiFi is disabled by default in the current version. To enable, uncomment `connectToWiFi();` in the `setup()` function.

### UI Mode

Switch between UI mode and image display mode:

```cpp
// UI Mode: 'image' to show image from GitHub, 'ui' to show custom UI
#define UI_MODE 'ui'  // Change to 'image' to show image instead
```

### Display Rotation

The display is rotated 90 degrees by default. To change rotation:

```cpp
// In setup() function
tft.setRotation(2);  // 0=0°, 1=90°, 2=180°, 3=270°
touchscreen.setRotation(2);  // Match TFT rotation
```

### Touch Calibration

If touch coordinates are incorrect, adjust the mapping values in `loop()`:

```cpp
x = map(p.x, 200, 3700, 0, W - 1);  // Adjust min/max values
y = map(p.y, 240, 3800, 0, H - 1);  // Adjust min/max values
```

## 📱 Usage

### Basic Operation

1. **Power On**: Connect the ESP32 board via USB
2. **Initialization**: The device will initialize the display and show the ElectricFish UI
3. **Touch Interaction**: 
   - Tap on **NACS** or **CCS** cards to select a connector type
   - Selected card will highlight with white border
   - Tap again to deselect

### UI Elements

- **Header**: Shows "ElectricFish" logo, current time (02:50), and battery level (88%)
- **Title**: "Select connect" with subtitle "Press button for your car"
- **Connector Cards**: Two cards side-by-side (NACS and CCS)
- **Progress Steps**: Four circular indicators showing current step
- **Version**: Firmware version displayed in bottom-left (02.6.5)

### Serial Monitor

Open Serial Monitor (115200 baud) to see:
- Screen dimensions after rotation
- Touch coordinates (X, Y, Pressure)
- Debug messages
- WiFi connection status (if enabled)
- Memory allocation information

## 📁 Project Structure

```
ef-unit-ces-demo/
├── README.md                          # This file
├── esp32-tft/
│   └── sketch_jan2a/
│       ├── sketch_jan2a.ino          # Main Arduino sketch
│       └── sketch_jan2a_PHASE1_BACKUP.ino  # Phase 1 backup
├── base_template.png                  # Original UI template
├── base_template_small.png           # Optimized template for ESP32
├── optimize_image.py                 # Python script to optimize images
└── rotate_image.py                   # Python script to rotate images
```

### Key Functions

- `drawElectricFishUI()` - Main UI rendering function
- `drawConnectorCard()` - Renders connector selection cards
- `drawProgressStep()` - Renders progress indicator steps
- `handleConnectorTouch()` - Handles touch events on connector cards
- `connectToWiFi()` - WiFi connection handler
- `fetchAndDisplayImage()` - Downloads and displays PNG images from network

## 📚 Libraries Required

| Library | Version | Purpose | Repository |
|---------|---------|---------|------------|
| TFT_eSPI | Latest | TFT display driver | [GitHub](https://github.com/Bodmer/TFT_eSPI) |
| PNGdec | Latest | PNG image decoding | [GitHub](https://github.com/Bodmer/PNGdec) |
| XPT2046_Touchscreen | Latest | Touchscreen driver | [GitHub](https://github.com/PaulStoffregen/XPT2046_Touchscreen) |
| WiFi (Built-in) | ESP32 Core | WiFi connectivity | Included in ESP32 core |
| HTTPClient (Built-in) | ESP32 Core | HTTP client | Included in ESP32 core |

## 🔍 Troubleshooting

### Display Shows Nothing / White Screen

- **Check wiring**: Verify all SPI connections
- **Check User_Setup.h**: Ensure correct pin definitions
- **Check backlight**: GPIO 27 (or your board's backlight pin) should be HIGH
- **Try different rotation**: Change `tft.setRotation()` value

### Touch Not Working

- **Check touchscreen wiring**: Verify XPT2046 connections
- **Calibrate touch**: Adjust mapping values in `map()` function
- **Check rotation**: Ensure `touchscreen.setRotation()` matches `tft.setRotation()`
- **Test in Serial Monitor**: Check if touch coordinates are being read

### Compilation Errors

- **Missing libraries**: Install all required libraries
- **Wrong board selected**: Ensure ESP32 Dev Module is selected
- **Wrong port**: Select correct COM port
- **User_Setup.h**: Verify TFT_eSPI configuration

### Memory Errors

- **Image too large**: Optimize images to < 100KB
- **Heap fragmentation**: Restart device
- **SPIRAM not available**: Some ESP32 boards don't have SPIRAM

### WiFi Connection Issues

- **Credentials**: Verify SSID and password
- **Signal strength**: Ensure good WiFi signal
- **2.4GHz only**: ESP32 only supports 2.4GHz WiFi networks
- **Firewall**: Check if network blocks new devices

### Image Display Issues

- **Network connectivity**: Ensure WiFi is connected
- **Image URL**: Verify URL is accessible
- **Image format**: Only PNG format is supported
- **Image size**: Keep images under 100KB for best performance

## 🎨 Customization

### Colors

Edit color definitions in the code:

```cpp
#define EF_PURPLE_DARK    0x5A6B  // Background color
#define EF_PURPLE_MEDIUM  0x7B0C  // Card color
#define EF_PURPLE_LIGHT   0x9C8E  // Accent color
```

### Layout

Adjust spacing and positioning:

```cpp
int margin = 8;           // Screen margins
int headerH = 18;         // Header height
int cardY = subtitleY + 51;  // Card vertical position
```

### UI State

Modify default values:

```cpp
int selectedConnector = 0;  // 0=none, 1=NACS, 2=CCS
int currentStep = 1;        // Current progress step
String currentTime = "02:50";
int batteryLevel = 88;
String version = "02.6.5";
```

## 📝 Development Notes

### Phase 1 (Completed)
- ✅ Responsive UI with dynamic dimensions
- ✅ Touch interaction
- ✅ Layout optimization for rotation
- ✅ Color scheme implementation
- ✅ Card positioning and sizing

### Phase 2 (In Progress)
- 🔄 UI functionality implementation
- 🔄 State management
- 🔄 Progress flow logic
- 🔄 Additional features

## 📄 License

This project is based on examples from Random Nerd Tutorials by Rui Santos & Sara Santos.

Original copyright notice:
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

## 🙏 Acknowledgments

- **Random Nerd Tutorials** - Original ESP32-TFT examples and setup guides
- **Bodmer** - TFT_eSPI and PNGdec libraries
- **Paul Stoffregen** - XPT2046_Touchscreen library
- **Espressif Systems** - ESP32 hardware and software support

## 📞 Support

For issues, questions, or contributions:
- Check [Troubleshooting](#troubleshooting) section
- Review [Random Nerd Tutorials](https://RandomNerdTutorials.com) for ESP32 guides
- Open an issue on the project repository

---

**Version**: 02.6.5  
**Last Updated**: January 2025  
**Status**: Phase 1 Complete, Phase 2 In Development

