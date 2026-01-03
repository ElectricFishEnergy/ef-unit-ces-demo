/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

/*
 * ===================================================================
 * BACKUP - FASE 1 COMPLETA
 * ===================================================================
 * Este arquivo é um backup do código da Fase 1 do projeto ESP32-TFT.
 * 
 * Fase 1 inclui:
 * - UI responsiva com dimensões dinâmicas (W/H)
 * - Layout ajustado para rotação 90 graus
 * - Cores purple mais claras
 * - Cards NACS e CCS posicionados corretamente
 * - Touch mapping funcional
 * - WiFi desabilitado no início (inicia direto na UI)
 * 
 * Data do backup: Antes da Fase 2 (funcionalidades da UI)
 * ===================================================================
 */

#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd/ or https://RandomNerdTutorials.com/esp32-tft/   */
#include <TFT_eSPI.h>

// Install PNGdec library by Bodmer for PNG image support: https://github.com/Bodmer/PNGdec
#include <PNGdec.h>
PNG png;

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
// Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Screen dimensions - set dynamically after rotation
// Using W/H instead of constants ensures UI adapts to actual screen size
int W = 0;  // Screen width (after rotation)
int H = 0;  // Screen height (after rotation)
#define FONT_SIZE 2

// WiFi credentials
const char* ssid = "zoinnelin";
const char* password = "M@ya2023!";

// UI Mode: 'image' to show image from GitHub, 'ui' to show custom UI
#define UI_MODE 'ui'  // Change to 'image' to show image instead

// GitHub image URL
const char* imageUrl = "https://raw.githubusercontent.com/ElectricFishEnergy/ef-unit-ces-demo/refs/heads/main/base_template_small.png";

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

// Browser scroll position
int scrollY = 0;
const int LINE_HEIGHT = 16;
const int MAX_LINES = 13;
String cachedHTML = "";

// HTML element structure for rendering
struct HTMLElement {
  String tag;
  String text;
  String style;
  int fontSize;
  uint16_t textColor;
  uint16_t bgColor;
  bool isLink;
  String linkUrl;
  bool isBold;
  bool isHeading;
};

HTMLElement displayElements[100];
int totalElements = 0;

// UI State
int selectedConnector = 0;  // 0 = none, 1 = NACS, 2 = CCS
int currentStep = 1;  // Current step in progress (1-4)
String currentTime = "02:50";
int batteryLevel = 88;
String version = "02.6.5";

// Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
void printTouchToSerial(int touchX, int touchY, int touchZ) {
  Serial.print("X = ");
  Serial.print(touchX);
  Serial.print(" | Y = ");
  Serial.print(touchY);
  Serial.print(" | Pressure = ");
  Serial.print(touchZ);
  Serial.println();
}

// Custom colors for ElectricFish UI - lighter purple tones
#define EF_PURPLE_DARK    0x5A6B  // Lighter dark purple background
#define EF_PURPLE_MEDIUM  0x7B0C  // Lighter medium purple for cards
#define EF_PURPLE_LIGHT   0x9C8E  // Lighter light purple
#define EF_WHITE          0xFFFF
#define EF_BLACK          0x0000
#define EF_GREEN          0x07E0
#define EF_YELLOW         0xFFE0

// Draw rounded rectangle (simplified - just rectangle with border)
void drawRoundedRect(int x, int y, int w, int h, uint16_t color, uint16_t bgColor) {
  // Draw background
  tft.fillRect(x, y, w, h, bgColor);
  // Draw border
  tft.drawRect(x, y, w, h, color);
  tft.drawRect(x+1, y+1, w-2, h-2, color);
}

// Draw connector selection card
void drawConnectorCard(int x, int y, int width, int height, String title, String subtitle, int number, bool selected) {
  // Card background
  uint16_t cardColor = selected ? EF_PURPLE_LIGHT : EF_PURPLE_MEDIUM;
  tft.fillRect(x, y, width, height, cardColor);
  
  // Card border (highlight if selected)
  if (selected) {
    tft.drawRect(x, y, width, height, EF_WHITE);
    tft.drawRect(x+1, y+1, width-2, height-2, EF_WHITE);
  } else {
    tft.drawRect(x, y, width, height, EF_PURPLE_DARK);
  }
  
  // Title - using font parameter only (not setTextSize)
  tft.setTextColor(EF_WHITE, cardColor);
  tft.setTextSize(1);  // Fixed - font parameter controls size
  tft.drawCentreString(title, x + width/2, y + 8, 2);
  
  // Subtitle
  tft.drawCentreString(subtitle, x + width/2, y + 28, 1);
  
  // Number circle at bottom (adjusted position)
  int circleX = x + width/2;
  int circleY = y + height - 18;
  int circleRadius = 10;  // Slightly smaller
  
  // Draw circle background
  tft.fillCircle(circleX, circleY, circleRadius, EF_WHITE);
  tft.drawCircle(circleX, circleY, circleRadius, EF_PURPLE_DARK);
  
  // Draw number
  tft.setTextColor(EF_PURPLE_DARK, EF_WHITE);
  tft.setTextSize(1);
  tft.drawCentreString(String(number), circleX, circleY - 4, 1);
}

// Draw progress step indicator
void drawProgressStep(int x, int y, int stepNum, String label, bool completed) {
  int circleRadius = 10;  // Slightly smaller to fit better
  
  // Draw circle
  if (completed) {
    tft.fillCircle(x, y, circleRadius, EF_PURPLE_MEDIUM);
    tft.drawCircle(x, y, circleRadius, EF_WHITE);
    // Draw checkmark
    tft.setTextColor(EF_WHITE, EF_PURPLE_MEDIUM);
    tft.setTextSize(1);
    tft.drawCentreString("✓", x, y - 4, 1);
  } else {
    tft.fillCircle(x, y, circleRadius, EF_PURPLE_DARK);
    tft.drawCircle(x, y, circleRadius, EF_PURPLE_MEDIUM);
  }
  
  // Draw label below (adjusted position)
  tft.setTextColor(EF_WHITE, EF_PURPLE_DARK);
  tft.setTextSize(1);
  tft.drawCentreString(label, x, y + 16, 1);
  
  // Draw connecting line to next step (if not last)
  if (stepNum < 4) {
    int lineLength = 50 - (circleRadius * 2);
    tft.drawLine(x + circleRadius, y, x + circleRadius + lineLength, y, EF_PURPLE_MEDIUM);
  }
}

// Draw ElectricFish UI
void drawElectricFishUI() {
  // Clear screen with dark purple background
  tft.fillScreen(EF_PURPLE_DARK);
  
  // Layout constants - responsive to screen size
  int margin = 8;
  int headerH = 18;  // Fixed header height
  int headerY = 3;
  
  // Logo and title (left side)
  tft.setTextColor(EF_WHITE, EF_PURPLE_DARK);
  tft.setTextSize(1);
  tft.drawString("ElectricFish", margin, headerY, 1);
  
  // Time (center)
  tft.drawCentreString(currentTime, W / 2, headerY, 1);
  
  // Battery (right side)
  String batteryText = String(batteryLevel) + "%";
  tft.drawRightString(batteryText, W - 3, headerY, 1);
  
  // Main title - using font parameter only (not setTextSize)
  // Don't mix setTextSize() with drawCentreString(..., font) - use font parameter for size
  int titleY = headerH + 8;
  tft.setTextColor(EF_WHITE, EF_PURPLE_DARK);
  tft.setTextSize(1);  // Fixed - font parameter controls size
  tft.drawCentreString("Select connect", W / 2, titleY, 2);
  
  // Subtitle (two lines)
  int subtitleY = titleY + 22;
  tft.setTextColor(EF_PURPLE_LIGHT, EF_PURPLE_DARK);
  tft.drawCentreString("Press button for", W / 2, subtitleY, 1);
  tft.drawCentreString("your car", W / 2, subtitleY + 12, 1);
  
  // Connector cards - responsive to screen width
  // Position lowered by 30% more (increased spacing from subtitle)
  int gap = 8;
  int cardY = subtitleY + 51;  // Increased from 39 to 51 (30% more spacing)
  int cardWidth = (W - 2 * margin - gap) / 2;
  int cardHeight = min(90, max(70, (H - cardY - 70)));  // Clamp to fit remaining space
  int card1X = margin;
  int card2X = margin + cardWidth + gap;
  
  // NACS Card
  drawConnectorCard(card1X, cardY, cardWidth, cardHeight, 
                    "NACS", "North", 1, selectedConnector == 1);
  
  // CCS Card
  drawConnectorCard(card2X, cardY, cardWidth, cardHeight, 
                    "CCS", "Combined", 2, selectedConnector == 2);
  
  // Progress bar at bottom - always at footer with safe margin
  int progressY = H - 60;  // Reserve 60px for footer/progress
  int stepSpacing = 48;
  int firstStepX = (W - (stepSpacing * 3)) / 2;
  
  // Step 1
  drawProgressStep(firstStepX, progressY, 1, "Start", currentStep >= 1);
  
  // Step 2
  drawProgressStep(firstStepX + stepSpacing, progressY, 2, "Payment", currentStep >= 2);
  
  // Step 3
  drawProgressStep(firstStepX + stepSpacing * 2, progressY, 3, "Connect", currentStep >= 3);
  
  // Step 4
  drawProgressStep(firstStepX + stepSpacing * 3, progressY, 4, "Finish", currentStep >= 4);
  
  // Version in bottom left (format: 02.6.5)
  tft.setTextColor(EF_PURPLE_LIGHT, EF_PURPLE_DARK);
  tft.setTextSize(1);
  String versionDisplay = "02.6.5";
  tft.drawString(versionDisplay, margin, H - 12, 1);
}

// Handle touch on connector cards - uses same dimensions as drawElectricFishUI
void handleConnectorTouch(int touchX, int touchY) {
  int margin = 8;
  int gap = 8;
  int headerH = 18;
  int titleY = headerH + 8;
  int subtitleY = titleY + 22;
  int cardY = subtitleY + 51;  // Increased from 39 to 51 (30% more spacing)
  int cardWidth = (W - 2 * margin - gap) / 2;
  int cardHeight = min(90, max(70, (H - cardY - 70)));
  int card1X = margin;
  int card2X = margin + cardWidth + gap;
  
  // Check if touch is in NACS card
  if (touchX >= card1X && touchX <= card1X + cardWidth &&
      touchY >= cardY && touchY <= cardY + cardHeight) {
    selectedConnector = (selectedConnector == 1) ? 0 : 1;
    drawElectricFishUI();
    Serial.println("NACS card touched");
  }
  // Check if touch is in CCS card
  else if (touchX >= card2X && touchX <= card2X + cardWidth &&
           touchY >= cardY && touchY <= cardY + cardHeight) {
    selectedConnector = (selectedConnector == 2) ? 0 : 2;
    drawElectricFishUI();
    Serial.println("CCS card touched");
  }
}

// Connect to WiFi
void connectToWiFi() {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawCentreString("Connecting to WiFi...", W / 2, 100, FONT_SIZE);
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_GREEN, TFT_WHITE);
    tft.drawCentreString("WiFi Connected!", W / 2, 100, FONT_SIZE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawCentreString("IP: " + WiFi.localIP().toString(), W / 2, 130, FONT_SIZE);
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    delay(2000);
  } else {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.drawCentreString("WiFi Failed!", W / 2, 100, FONT_SIZE);
    Serial.println();
    Serial.println("WiFi connection failed!");
  }
}

// Decode HTML entities
String decodeEntities(String text) {
  text.replace("&nbsp;", " ");
  text.replace("&amp;", "&");
  text.replace("&lt;", "<");
  text.replace("&gt;", ">");
  text.replace("&quot;", "\"");
  text.replace("&#39;", "'");
  text.replace("&apos;", "'");
  text.replace("&copy;", "(c)");
  text.replace("&reg;", "(R)");
  text.replace("&trade;", "(TM)");
  return text;
}

// Extract style attribute value
String getStyleValue(String style, String property) {
  int propIndex = style.indexOf(property);
  if (propIndex == -1) return "";
  
  int colonIndex = style.indexOf(":", propIndex);
  if (colonIndex == -1) return "";
  
  int semicolonIndex = style.indexOf(";", colonIndex);
  int endIndex = (semicolonIndex == -1) ? style.length() : semicolonIndex;
  
  String value = style.substring(colonIndex + 1, endIndex);
  value.trim();
  return value;
}

// Convert CSS color to TFT color
uint16_t parseColor(String colorStr) {
  colorStr.toLowerCase();
  colorStr.trim();
  
  // Named colors
  if (colorStr == "black") return TFT_BLACK;
  if (colorStr == "white") return TFT_WHITE;
  if (colorStr == "red") return TFT_RED;
  if (colorStr == "green") return TFT_GREEN;
  if (colorStr == "blue") return TFT_BLUE;
  if (colorStr == "yellow") return TFT_YELLOW;
  if (colorStr == "cyan") return TFT_CYAN;
  if (colorStr == "magenta") return TFT_MAGENTA;
  if (colorStr == "orange") return TFT_ORANGE;
  if (colorStr == "darkgreen") return TFT_DARKGREEN;
  if (colorStr == "navy") return TFT_NAVY;
  if (colorStr == "maroon") return TFT_MAROON;
  if (colorStr == "purple") return TFT_PURPLE;
  if (colorStr == "olive") return TFT_OLIVE;
  if (colorStr == "lightgrey") return TFT_LIGHTGREY;
  if (colorStr == "darkgrey") return TFT_DARKGREY;
  
  // Hex colors (#RRGGBB or #RGB)
  if (colorStr.startsWith("#")) {
    colorStr = colorStr.substring(1);
    if (colorStr.length() == 6) {
      long color = strtol(colorStr.c_str(), NULL, 16);
      return ((color >> 8) & 0xF8) << 8 | ((color >> 3) & 0xFC) << 3 | (color & 0xF8) >> 3;
    } else if (colorStr.length() == 3) {
      // Expand #RGB to #RRGGBB
      String expanded = "";
      for (int i = 0; i < 3; i++) {
        expanded += colorStr.charAt(i);
        expanded += colorStr.charAt(i);
      }
      long color = strtol(expanded.c_str(), NULL, 16);
      return ((color >> 8) & 0xF8) << 8 | ((color >> 3) & 0xFC) << 3 | (color & 0xF8) >> 3;
    }
  }
  
  // RGB(r, g, b)
  if (colorStr.startsWith("rgb")) {
    int start = colorStr.indexOf("(");
    int end = colorStr.indexOf(")");
    if (start != -1 && end != -1) {
      String rgb = colorStr.substring(start + 1, end);
      rgb.replace(" ", "");
      int comma1 = rgb.indexOf(",");
      int comma2 = rgb.indexOf(",", comma1 + 1);
      if (comma1 != -1 && comma2 != -1) {
        int r = rgb.substring(0, comma1).toInt();
        int g = rgb.substring(comma1 + 1, comma2).toInt();
        int b = rgb.substring(comma2 + 1).toInt();
        return tft.color565(r, g, b);
      }
    }
  }
  
  return TFT_BLACK; // Default
}

// Parse font size from CSS
int parseFontSize(String fontSize) {
  fontSize.trim();
  fontSize.toLowerCase();
  
  if (fontSize.endsWith("px")) {
    fontSize = fontSize.substring(0, fontSize.length() - 2);
    fontSize.trim();
    int size = fontSize.toInt();
    if (size <= 12) return 1;
    if (size <= 18) return 2;
    return 3;
  }
  
  if (fontSize == "small") return 1;
  if (fontSize == "medium" || fontSize == "") return 2;
  if (fontSize == "large") return 2;
  if (fontSize == "x-large" || fontSize == "xx-large") return 3;
  
  return 2; // Default
}

// Parse HTML and extract elements with styles
void parseHTML(String html) {
  totalElements = 0;
  
  // Remove scripts and styles
  while (html.indexOf("<script") != -1) {
    int start = html.indexOf("<script");
    int end = html.indexOf("</script>", start);
    if (end != -1) {
      html.remove(start, end - start + 9);
    } else {
      int tagEnd = html.indexOf(">", start);
      if (tagEnd != -1) {
        html.remove(start, tagEnd - start + 1);
      } else {
        break;
      }
    }
  }
  
  while (html.indexOf("<style") != -1) {
    int start = html.indexOf("<style");
    int end = html.indexOf("</style>", start);
    if (end != -1) {
      html.remove(start, end - start + 8);
    } else {
      int tagEnd = html.indexOf(">", start);
      if (tagEnd != -1) {
        html.remove(start, tagEnd - start + 1);
      } else {
        break;
      }
    }
  }
  
  // Extract body content
  int bodyStart = html.indexOf("<body");
  if (bodyStart != -1) {
    int bodyTagEnd = html.indexOf(">", bodyStart) + 1;
    int bodyEnd = html.indexOf("</body>", bodyTagEnd);
    if (bodyEnd != -1) {
      html = html.substring(bodyTagEnd, bodyEnd);
    } else {
      html = html.substring(bodyTagEnd);
    }
  }
  
  // Parse elements
  int pos = 0;
  while (pos < html.length() && totalElements < 100) {
    // Find next tag
    int tagStart = html.indexOf("<", pos);
    if (tagStart == -1) {
      // No more tags, get remaining text
      String text = html.substring(pos);
      text.trim();
      if (text.length() > 0) {
        text = decodeEntities(text);
        // Clean whitespace
        while (text.indexOf("  ") != -1) text.replace("  ", " ");
        if (text.length() > 0 && totalElements < 100) {
          displayElements[totalElements].tag = "text";
          displayElements[totalElements].text = text;
          displayElements[totalElements].fontSize = 1;
          displayElements[totalElements].textColor = TFT_BLACK;
          displayElements[totalElements].bgColor = TFT_WHITE;
          displayElements[totalElements].isLink = false;
          displayElements[totalElements].isBold = false;
          displayElements[totalElements].isHeading = false;
          totalElements++;
        }
      }
      break;
    }
    
    // Get text before tag
    if (tagStart > pos) {
      String text = html.substring(pos, tagStart);
      text.trim();
      if (text.length() > 0) {
        text = decodeEntities(text);
        while (text.indexOf("  ") != -1) text.replace("  ", " ");
        if (text.length() > 0 && totalElements < 100) {
          displayElements[totalElements].tag = "text";
          displayElements[totalElements].text = text;
          displayElements[totalElements].fontSize = 1;
          displayElements[totalElements].textColor = TFT_BLACK;
          displayElements[totalElements].bgColor = TFT_WHITE;
          displayElements[totalElements].isLink = false;
          displayElements[totalElements].isBold = false;
          displayElements[totalElements].isHeading = false;
          totalElements++;
        }
      }
    }
    
    // Parse tag
    int tagEnd = html.indexOf(">", tagStart);
    if (tagEnd == -1) break;
    
    String tag = html.substring(tagStart + 1, tagEnd);
    tag.toLowerCase();
    tag.trim();
    
    // Skip closing tags
    if (tag.startsWith("/")) {
      pos = tagEnd + 1;
      continue;
    }
    
    // Extract tag name and attributes
    int spaceIndex = tag.indexOf(" ");
    String tagName = (spaceIndex == -1) ? tag : tag.substring(0, spaceIndex);
    
    // Extract style attribute
    String style = "";
    int styleStart = tag.indexOf("style=\"");
    if (styleStart != -1) {
      int styleEnd = tag.indexOf("\"", styleStart + 7);
      if (styleEnd != -1) {
        style = tag.substring(styleStart + 7, styleEnd);
      }
    }
    
    // Extract href for links
    String href = "";
    bool isLink = (tagName == "a");
    if (isLink) {
      int hrefStart = tag.indexOf("href=\"");
      if (hrefStart != -1) {
        int hrefEnd = tag.indexOf("\"", hrefStart + 6);
        if (hrefEnd != -1) {
          href = tag.substring(hrefStart + 6, hrefEnd);
        }
      }
    }
    
    // Find closing tag
    int contentStart = tagEnd + 1;
    int contentEnd = html.indexOf("</" + tagName + ">", contentStart);
    if (contentEnd == -1) {
      // Self-closing or no closing tag
      if (tagName == "br" || tagName == "hr") {
        if (totalElements < 100) {
          displayElements[totalElements].tag = tagName;
          displayElements[totalElements].text = "";
          displayElements[totalElements].fontSize = 1;
          displayElements[totalElements].textColor = TFT_BLACK;
          displayElements[totalElements].bgColor = TFT_WHITE;
          displayElements[totalElements].isLink = false;
          displayElements[totalElements].isBold = false;
          displayElements[totalElements].isHeading = false;
          totalElements++;
        }
      }
      pos = tagEnd + 1;
      continue;
    }
    
    // Extract content
    String content = html.substring(contentStart, contentEnd);
    content.trim();
    content = decodeEntities(content);
    while (content.indexOf("  ") != -1) content.replace("  ", " ");
    
    if (content.length() > 0 && totalElements < 100) {
      displayElements[totalElements].tag = tagName;
      displayElements[totalElements].text = content;
      displayElements[totalElements].style = style;
      displayElements[totalElements].isLink = isLink;
      displayElements[totalElements].linkUrl = href;
      
      // Determine font size based on tag
      if (tagName == "h1") {
        displayElements[totalElements].fontSize = 3;
        displayElements[totalElements].isHeading = true;
        displayElements[totalElements].isBold = true;
      } else if (tagName == "h2") {
        displayElements[totalElements].fontSize = 2;
        displayElements[totalElements].isHeading = true;
        displayElements[totalElements].isBold = true;
      } else if (tagName == "h3" || tagName == "h4" || tagName == "h5" || tagName == "h6") {
        displayElements[totalElements].fontSize = 2;
        displayElements[totalElements].isHeading = true;
        displayElements[totalElements].isBold = true;
      } else {
        displayElements[totalElements].fontSize = parseFontSize(getStyleValue(style, "font-size"));
        displayElements[totalElements].isHeading = false;
      }
      
      // Parse colors from style
      String color = getStyleValue(style, "color");
      displayElements[totalElements].textColor = (color.length() > 0) ? parseColor(color) : (isLink ? TFT_BLUE : TFT_BLACK);
      
      String bgColor = getStyleValue(style, "background-color");
      displayElements[totalElements].bgColor = (bgColor.length() > 0) ? parseColor(bgColor) : TFT_WHITE;
      
      // Check for bold
      displayElements[totalElements].isBold = (tagName == "b" || tagName == "strong" || 
                                               style.indexOf("font-weight:bold") != -1 ||
                                               style.indexOf("font-weight:700") != -1 ||
                                               displayElements[totalElements].isHeading);
      
      totalElements++;
    }
    
    pos = contentEnd + tagName.length() + 3;
  }
}

// Render HTML page like a browser
void renderHTMLPage(String html) {
  // Clear screen
  tft.fillScreen(TFT_WHITE);
  
  // Draw browser header bar
  tft.fillRect(0, 0, W, 25, TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setTextSize(1);
  
  // Extract and display title in header
  String title = "";
  int titleStart = html.indexOf("<title>");
  int titleEnd = html.indexOf("</title>");
  if (titleStart != -1 && titleEnd != -1) {
    title = html.substring(titleStart + 7, titleEnd);
    title = decodeEntities(title);
  }
  
  String urlDisplay = (title.length() > 0) ? title : "google.com";
  if (urlDisplay.length() > 20) {
    urlDisplay = urlDisplay.substring(0, 17) + "...";
  }
  tft.drawString(urlDisplay, 5, 8, 1);
  
  // Parse HTML
  parseHTML(html);
  
  // Render the display
  renderDisplay();
}

// Word wrap text to fit screen
String wrapTextToWidth(String text, int maxWidth, int fontSize) {
  int charWidth = (fontSize == 1) ? 6 : (fontSize == 2) ? 12 : 18;
  int maxChars = maxWidth / charWidth;
  
  if (text.length() <= maxChars) return text;
  
  String result = "";
  int lastSpace = -1;
  int lineStart = 0;
  
  for (int i = 0; i < text.length(); i++) {
    if (text.charAt(i) == ' ') {
      lastSpace = i;
    }
    
    if (i - lineStart >= maxChars) {
      if (lastSpace > lineStart) {
        result += text.substring(lineStart, lastSpace) + "\n";
        lineStart = lastSpace + 1;
      } else {
        result += text.substring(lineStart, i) + "\n";
        lineStart = i;
      }
    }
  }
  
  if (lineStart < text.length()) {
    result += text.substring(lineStart);
  }
  
  return result;
}

// Render the display with current scroll position
void renderDisplay() {
  // Clear content area (keep header)
  tft.fillRect(0, 25, W - 5, H - 25, TFT_WHITE);
  
  // Draw scroll indicator background
  tft.fillRect(W - 5, 25, 5, H - 25, TFT_LIGHTGREY);
  
  int yPos = 30;
  int elementIndex = 0;
  int linesRendered = 0;
  const int maxRenderHeight = H - 30;
  
  // Render elements with scroll
  while (elementIndex < totalElements && yPos < H - 5 && linesRendered < MAX_LINES * 2) {
    HTMLElement elem = displayElements[elementIndex];
    
    // Skip if scrolled past
    if (linesRendered < scrollY) {
      linesRendered++;
      elementIndex++;
      continue;
    }
    
    // Check if we've rendered enough
    if (linesRendered >= scrollY + MAX_LINES) {
      break;
    }
    
    // Handle different element types
    if (elem.tag == "br") {
      yPos += LINE_HEIGHT;
      linesRendered++;
    } else if (elem.tag == "hr") {
      tft.drawLine(5, yPos, W - 10, yPos, TFT_DARKGREY);
      yPos += LINE_HEIGHT;
      linesRendered++;
    } else if (elem.text.length() > 0) {
      // Set colors
      tft.setTextColor(elem.textColor, elem.bgColor);
      tft.setTextSize(elem.fontSize);
      
      // Draw background if needed
      if (elem.bgColor != TFT_WHITE) {
        int textHeight = (elem.fontSize == 1) ? 8 : (elem.fontSize == 2) ? 16 : 24;
        tft.fillRect(5, yPos - 2, W - 10, textHeight + 4, elem.bgColor);
      }
      
      // Word wrap text
      int charWidth = (elem.fontSize == 1) ? 6 : (elem.fontSize == 2) ? 12 : 18;
      int maxChars = (W - 10) / charWidth;
      
      String displayText = elem.text;
      if (displayText.length() > maxChars) {
        // Simple truncation for now (could implement proper wrapping)
        displayText = displayText.substring(0, maxChars - 3) + "...";
      }
      
      // Draw text
      if (elem.isLink) {
        // Underline links (draw line below)
        tft.drawString(displayText, 5, yPos, elem.fontSize);
        int textWidth = displayText.length() * charWidth;
        int underlineY = yPos + ((elem.fontSize == 1) ? 8 : (elem.fontSize == 2) ? 16 : 24);
        tft.drawLine(5, underlineY, 5 + textWidth, underlineY, elem.textColor);
      } else {
        tft.drawString(displayText, 5, yPos, elem.fontSize);
      }
      
      // Calculate height based on font size
      int textHeight = (elem.fontSize == 1) ? LINE_HEIGHT : (elem.fontSize == 2) ? 18 : 26;
      yPos += textHeight;
      linesRendered++;
    }
    
    elementIndex++;
  }
  
  // Draw scrollbar
  if (totalElements > MAX_LINES) {
    int scrollbarHeight = max(5, (H - 25) * MAX_LINES / totalElements);
    int maxScroll = totalElements - MAX_LINES;
    int scrollbarY = 25;
    if (maxScroll > 0 && scrollY > 0) {
      scrollbarY = 25 + (scrollY * (H - 25 - scrollbarHeight) / maxScroll);
      if (scrollbarY > H - scrollbarHeight - 1) {
        scrollbarY = H - scrollbarHeight - 1;
      }
    }
    tft.fillRect(W - 5, scrollbarY, 5, scrollbarHeight, TFT_DARKGREY);
  }
}

// Variables for PNG rendering
int pngX = 0;
int pngY = 0;  // Start from top (no header)
int pngOriginalWidth = 0;
int pngOriginalHeight = 0;
float pngScaleX = 1.0;
float pngScaleY = 1.0;

// Draw PNG callback function with scaling (stretch to fit screen)
int pngDraw(PNGDRAW *pDraw) {
  // Use max width (320) for buffer allocation - W may be 0 at compile time
  uint16_t lineBuffer[320];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  
  // Get original line position and width
  int originalY = pDraw->y;
  int originalWidth = pDraw->iWidth;
  
  // Calculate scaled Y position for this line
  float scaledYStart = originalY * pngScaleY;
  float scaledYEnd = (originalY + 1) * pngScaleY;
  int startY = (int)scaledYStart;
  int endY = (int)scaledYEnd;
  int lineHeight = endY - startY;
  
  if (lineHeight < 1) lineHeight = 1;
  
  // Draw each pixel of the line, scaled horizontally
  for (int srcX = 0; srcX < originalWidth; srcX++) {
    // Calculate scaled X position
    float scaledXStart = srcX * pngScaleX;
    float scaledXEnd = (srcX + 1) * pngScaleX;
    int startX = (int)scaledXStart;
    int endX = (int)scaledXEnd;
    int pixelWidth = endX - startX;
    
    if (pixelWidth < 1) pixelWidth = 1;
    
    // Calculate screen position
    int screenX = pngX + startX;
    int screenY = pngY + startY;
    
    // Get pixel color
    uint16_t pixelColor = lineBuffer[srcX];
    
    // Draw scaled pixel (as a rectangle to fill the scaled area)
    if (screenX >= 0 && screenX < W && 
        screenY >= 0 && screenY < H) {
      // Clip to screen bounds
      int drawWidth = min(pixelWidth, W - screenX);
      int drawHeight = min(lineHeight, H - screenY);
      
      if (drawWidth > 0 && drawHeight > 0) {
        // Use fillRect for better quality (avoids gaps)
        tft.fillRect(screenX, screenY, drawWidth, drawHeight, pixelColor);
      }
    }
  }
  
  return 1; // Return 1 to continue decoding
}

// Download and display image from GitHub
void fetchAndDisplayImage() {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawCentreString("Loading image...", W / 2, 100, FONT_SIZE);
  
  Serial.println("Fetching image from GitHub...");
  Serial.println(imageUrl);
  
  WiFiClientSecure client;
  HTTPClient http;
  
  // Allow insecure HTTPS connections (for development/testing)
  client.setInsecure();
  
  http.begin(client, imageUrl);
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpCode);
    
    // Get content length
    int contentLength = http.getSize();
    Serial.printf("Content length: %d bytes\n", contentLength);
    
    if (contentLength > 0) {
      // Get the stream
      WiFiClient *stream = http.getStreamPtr();
      
      // Clear entire screen (no header)
      tft.fillScreen(TFT_WHITE);
      
      // Try to decode PNG directly from stream
      // Note: PNGdec needs the full image in memory or a proper file handle
      // For streaming, we'll download to a buffer first
      
      // Check available free heap memory
      size_t freeHeap = ESP.getFreeHeap();
      size_t maxAlloc = ESP.getMaxAllocHeap();
      Serial.printf("Free heap before allocation: %d bytes (%.2f KB)\n", freeHeap, freeHeap / 1024.0);
      Serial.printf("Largest allocatable block: %d bytes (%.2f KB)\n", maxAlloc, maxAlloc / 1024.0);
      
      // Use a more conservative approach - limit to available memory minus safety margin
      // Leave at least 30KB free for system operations (reduced from 50KB)
      const int safetyMargin = 30000;
      int maxImageSize = maxAlloc - safetyMargin;
      if (maxImageSize < 30000) maxImageSize = 30000; // Minimum 30KB
      
      Serial.printf("Max image size: %d bytes (%.2f KB)\n", maxImageSize, maxImageSize / 1024.0);
      Serial.printf("Image size: %d bytes (%.2f KB)\n", contentLength, contentLength / 1024.0);
      
      uint8_t *imageBuffer = nullptr;
      
      if (contentLength > 0 && contentLength < maxImageSize) {
        Serial.println("Attempting to allocate memory...");
        imageBuffer = (uint8_t *)heap_caps_malloc(contentLength, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        
        // If SPIRAM allocation failed, try regular heap
        if (!imageBuffer) {
          Serial.println("SPIRAM allocation failed, trying regular heap...");
          imageBuffer = (uint8_t *)malloc(contentLength);
        }
        
        if (imageBuffer) {
          Serial.printf("Memory allocated successfully: %d bytes\n", contentLength);
          int bytesRead = 0;
          int totalBytes = 0;
          
          // Read image data
          while (http.connected() && totalBytes < contentLength) {
            bytesRead = stream->readBytes(imageBuffer + totalBytes, min(1024, contentLength - totalBytes));
            totalBytes += bytesRead;
            Serial.printf("Downloaded: %d / %d bytes\n", totalBytes, contentLength);
          }
          
          if (totalBytes == contentLength) {
            Serial.println("Image downloaded successfully!");
            
            // Clear entire screen
            tft.fillScreen(TFT_WHITE);
            
            // Decode and display PNG
            int16_t pngReturn = png.openRAM(imageBuffer, contentLength, pngDraw);
            
            if (pngReturn == PNG_SUCCESS) {
              pngOriginalWidth = png.getWidth();
              pngOriginalHeight = png.getHeight();
              
              Serial.printf("PNG image size: %d x %d\n", pngOriginalWidth, pngOriginalHeight);
              
              // Calculate display area (full screen - no header)
              int displayWidth = W;
              int displayHeight = H;  // Full screen height
              
              // Calculate scale to stretch to fit screen (stretch both dimensions)
              pngScaleX = (float)displayWidth / pngOriginalWidth;
              pngScaleY = (float)displayHeight / pngOriginalHeight;
              
              Serial.printf("Scale X: %.3f, Scale Y: %.3f\n", pngScaleX, pngScaleY);
              Serial.printf("Scaled size: %d x %d\n", 
                            (int)(pngOriginalWidth * pngScaleX), 
                            (int)(pngOriginalHeight * pngScaleY));
              
              // Image starts at top-left (fills entire screen)
              pngX = 0;
              pngY = 0;  // Start from top
              
              Serial.printf("Displaying stretched at: x=%d, y=%d\n", pngX, pngY);
              
              // Decode and render
              int decodeResult = png.decode(NULL, 0);
              
              if (decodeResult == PNG_SUCCESS) {
                Serial.println("Image displayed successfully!");
              } else {
                Serial.printf("PNG decode error: %d\n", decodeResult);
                tft.setTextColor(TFT_RED, TFT_WHITE);
                tft.drawCentreString("Decode error: " + String(decodeResult), W / 2, 120, 1);
              }
              
              png.close();
            } else {
              Serial.printf("PNG open error: %d\n", pngReturn);
              tft.setTextColor(TFT_RED, TFT_WHITE);
              tft.drawCentreString("PNG open error", W / 2, 120, FONT_SIZE);
            }
          } else {
            Serial.println("Download incomplete!");
            tft.setTextColor(TFT_RED, TFT_WHITE);
            tft.drawCentreString("Download failed", W / 2, 120, FONT_SIZE);
          }
          
          free(imageBuffer);
        } else {
          Serial.println("Failed to allocate memory for image");
          Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
          Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
          tft.fillScreen(TFT_WHITE);
          tft.setTextColor(TFT_RED, TFT_WHITE);
          tft.drawCentreString("Memory error!", W / 2, 100, FONT_SIZE);
          tft.setTextColor(TFT_BLACK, TFT_WHITE);
          String freeMsg = "Free: " + String(ESP.getFreeHeap() / 1024) + " KB";
          tft.drawCentreString(freeMsg, W / 2, 120, 1);
          String needMsg = "Need: " + String(contentLength / 1024) + " KB";
          tft.drawCentreString(needMsg, W / 2, 140, 1);
        }
      } else {
        Serial.printf("Image too large: %d bytes (max: %d)\n", contentLength, maxImageSize);
        Serial.printf("Image size: %.2f KB\n", contentLength / 1024.0);
        Serial.println("SOLUTION: Please optimize the image:");
        Serial.println("1. Reduce image dimensions (max 320x240 recommended)");
        Serial.println("2. Compress PNG more (use tools like TinyPNG)");
        Serial.println("3. Convert to JPEG format (smaller file size)");
        Serial.println("4. Or create a smaller version for ESP32");
        
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawCentreString("Image too large!", W / 2, 80, FONT_SIZE);
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        String sizeMsg = String(contentLength / 1024) + " KB";
        tft.drawCentreString(sizeMsg, W / 2, 110, 1);
        String maxMsg = "Max: " + String(maxImageSize / 1024) + " KB";
        tft.drawCentreString(maxMsg, W / 2, 130, 1);
        tft.drawCentreString("Optimize image", W / 2, 160, 1);
        tft.drawCentreString("or use smaller", W / 2, 180, 1);
        tft.drawCentreString("version", W / 2, 200, 1);
      }
    } else {
      Serial.println("Unknown content length");
      tft.setTextColor(TFT_RED, TFT_WHITE);
      tft.drawCentreString("Invalid response", W / 2, 120, FONT_SIZE);
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.drawCentreString("HTTP Error!", W / 2, 100, FONT_SIZE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawCentreString("Code: " + String(httpCode), W / 2, 130, FONT_SIZE);
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation to match display rotation (90 degrees)
  // Note: in some displays, the touchscreen might be upside down, so you might need to adjust: touchscreen.setRotation(2 or 3);
  touchscreen.setRotation(2);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation - rotated 90 degrees from landscape
  tft.setRotation(2);  // 2 = 90 degrees clockwise from rotation 1
  
  // Get actual screen dimensions after rotation
  // This is critical: rotation can swap width/height (e.g., 240x320 vs 320x240)
  W = tft.width();
  H = tft.height();
  Serial.printf("TFT size after rotation: %dx%d\n", W, H);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  
  // WiFi connection disabled - start directly in UI mode
  // connectToWiFi();
  
  // Display UI or image based on mode
  if (UI_MODE == 'ui') {
    // Draw custom ElectricFish UI
    drawElectricFishUI();
  } else {
    // If WiFi connected, fetch and display image
    if (WiFi.status() == WL_CONNECTED) {
      delay(1000);
      fetchAndDisplayImage();
    }
  }
}

void loop() {
  if (UI_MODE == 'ui') {
    // UI Mode - handle touch interactions
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
      // Get Touchscreen points
      TS_Point p = touchscreen.getPoint();
      // Calibrate Touchscreen points with map function to the correct width and height
      // Use W/H (actual screen dimensions) instead of constants
      x = map(p.x, 200, 3700, 0, W - 1);
      y = map(p.y, 240, 3800, 0, H - 1);
      z = p.z;

      printTouchToSerial(x, y, z);
      
      // Handle touch on UI elements
      handleConnectorTouch(x, y);
      
      delay(200);
    }
  } else {
    // Image Mode - original behavior
    // Check WiFi connection status periodically
    if (WiFi.status() != WL_CONNECTED) {
      // Try to reconnect every 10 seconds
      static unsigned long lastReconnectAttempt = 0;
      if (millis() - lastReconnectAttempt > 10000) {
        lastReconnectAttempt = millis();
        connectToWiFi();
        if (WiFi.status() == WL_CONNECTED) {
          delay(1000);
          fetchAndDisplayImage();
        }
      }
    }
    
    // Checks if Touchscreen was touched
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
      // Get Touchscreen points
      TS_Point p = touchscreen.getPoint();
      // Calibrate Touchscreen points with map function to the correct width and height
      // Use W/H (actual screen dimensions) instead of constants
      x = map(p.x, 200, 3700, 0, W - 1);
      y = map(p.y, 240, 3800, 0, H - 1);
      z = p.z;

      printTouchToSerial(x, y, z);
      
      // If WiFi is connected, reload image
      if (WiFi.status() == WL_CONNECTED) {
        fetchAndDisplayImage();
      }

      delay(200);
    }
  }
  
  delay(10);
}

