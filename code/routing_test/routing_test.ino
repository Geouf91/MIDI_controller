/**
 * Routing Test Prototype
 * 
 * Hardware Setup:
 * - 2x MCP23017 I2C expanders
 * - 3x Rotary encoders with integrated switches
 * - 1x SSD1306 OLED display (128x64)
 * 
 * MCP23017 #1 (Address 0x20):
 *   GPB0 → CLK1 (Encoder 1 pin A)
 *   GPB1 → DT1  (Encoder 1 pin B)
 *   GPB2 → SW1  (Encoder 1 switch)
 *   GPB4 → CLK2 (Encoder 2 pin A)
 *   GPB5 → DT2  (Encoder 2 pin B)
 *   GPB6 → SW2  (Encoder 2 switch)
 * 
 * MCP23017 #2 (Address 0x21):
 *   GPB0 → CLK3 (Encoder 3 pin A)
 *   GPB1 → DT3  (Encoder 3 pin B)
 *   GPB2 → SW3  (Encoder 3 switch)
 * 
 * Connections:
 *   - SDA: MCP23017 SDA pins + OLED SDA
 *   - SCL: MCP23017 SCL pins + OLED SCL
 *   - Pin 12: MCP23017 #1 INTA/INTB
 *   - Pin 13: MCP23017 #2 INTA/INTB
 * 
 * MCP23017 Address Configuration:
 *   MCP #1: A0=LOW, A1=LOW, A2=LOW (0x20)
 *   MCP #2: A0=HIGH, A1=LOW, A2=LOW (0x21)
 */

#include <Wire.h>
#include <Control_Surface.h>
#include <AH/Hardware/MCP23017Encoders.hpp>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin (-1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // Common I2C address for SSD1306

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Type definitions for MCP23017 encoders
using WireType = decltype(Wire);
using EncoderPositionType = int16_t;  // Using signed int for bidirectional counting
using MCPEncoderType = MCP23017Encoders<WireType, EncoderPositionType>;

// Create two MCP23017 encoder managers
MCPEncoderType mcp1 {Wire, 0x0, 12};  // Address 0x20, interrupt on pin 12
//                   │     │    └─ Interrupt pin
//                   │     └────── Address offset (0x0 = 0x20)
//                   └──────────── I²C interface

MCPEncoderType mcp2 {Wire, 0x1, 13};  // Address 0x21, interrupt on pin 13
//                   │     │    └─ Interrupt pin
//                   │     └────── Address offset (0x1 = 0x21)
//                   └──────────── I²C interface

// Pin assignments for encoders and switches on MCP23017
// 
// ENCODER INDEX MAPPING (library uses pairs of GPIO pins):
//   Index 0 = GPA0+GPA1,  Index 1 = GPA2+GPA3,  Index 2 = GPA4+GPA5,  Index 3 = GPA6+GPA7
//   Index 4 = GPB0+GPB1,  Index 5 = GPB2+GPB3,  Index 6 = GPB4+GPB5,  Index 7 = GPB6+GPB7
//
// SWITCH PIN MAPPING (library uses GPIO bit numbers, NOT physical chip pins):
//   GPA0-7 = library pins 0-7  (physical chip pins 21-28)
//   GPB0-7 = library pins 8-15 (physical chip pins 1-8)
//   Example: GPB2 = physical chip pin 3 = library pin 10
//
// YOUR WIRING:
//   MCP #1: GPB0(clk1), GPB1(dt1), GPB2(sw1) → physical pins 1,2,3
//   MCP #1: GPB4(clk2), GPB5(dt2), GPB6(sw2) → physical pins 5,6,7
//   MCP #2: GPB0(clk3), GPB1(dt3), GPB2(sw3) → physical pins 1,2,3

// MCP #1: Encoder 1 on GPB0-1 (index 4), Encoder 2 on GPB4-5 (index 6)
const uint8_t ENC1_INDEX = 4;  // GPB0-GPB1 (physical pins 1-2)
const uint8_t SW1_PIN = 10;    // GPB2 (physical pin 3, library pin 10)
const uint8_t ENC2_INDEX = 6;  // GPB4-GPB5 (physical pins 5-6)
const uint8_t SW2_PIN = 14;    // GPB6 (physical pin 7, library pin 14)

// MCP #2: Encoder 3 on GPB0-1 (index 4)
const uint8_t ENC3_INDEX = 4;  // GPB0-GPB1 (physical pins 1-2)
const uint8_t SW3_PIN = 10;    // GPB2 (physical pin 3, library pin 10)

// Encoder values and switch states
int16_t encoder1_value = 0;
int16_t encoder2_value = 0;
int16_t encoder3_value = 0;

bool switch1_state = false;
bool switch2_state = false;
bool switch3_state = false;

// Previous values for change detection
int16_t prev_enc1 = 0;
int16_t prev_enc2 = 0;
int16_t prev_enc3 = 0;
bool prev_sw1 = false;
bool prev_sw2 = false;
bool prev_sw3 = false;

// Display update timing
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 50;  // Update every 50ms

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);  // Wait for Serial (max 3 seconds)
  
  Serial.println("=== MCP23017 Routing Test ===");
  Serial.println("Initializing...");
  
  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);  // 400kHz I2C speed
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("ERROR: SSD1306 allocation failed!"));
    Serial.println(F("Check OLED wiring and I2C address (0x3C or 0x3D)"));
    // Don't halt - continue without display
  } else {
    Serial.println("OLED initialized successfully");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Routing Test");
    display.println("Initializing...");
    display.display();
  }
  
  // Initialize MCP23017 chips
  Serial.println("Initializing MCP23017 #1...");
  if (!mcp1.begin()) {
    Serial.println("ERROR: MCP23017 #1 not found! Check wiring and address.");
  } else {
    Serial.println("MCP23017 #1 initialized (0x20)");
  }
  
  Serial.println("Initializing MCP23017 #2...");
  if (!mcp2.begin()) {
    Serial.println("ERROR: MCP23017 #2 not found! Check wiring and address.");
  } else {
    Serial.println("MCP23017 #2 initialized (0x21)");
  }
  
  delay(500);
  
  Serial.println("\n=== Setup Complete ===");
  Serial.println("Encoder 1: MCP1 GPB0-1, Switch: GPB2");
  Serial.println("Encoder 2: MCP1 GPB4-5, Switch: GPB6");
  Serial.println("Encoder 3: MCP2 GPB0-1, Switch: GPB2");
  Serial.println("\nRotate encoders and press switches...\n");
}

void loop() {
  // Update both MCP23017 chips
  mcp1.update();
  mcp2.update();
  
  // Read encoder values
  encoder1_value = mcp1[ENC1_INDEX].read();
  encoder2_value = mcp1[ENC2_INDEX].read();
  encoder3_value = mcp2[ENC3_INDEX].read();
  
  // Read switch states (switches connect to ground when pressed)
  // MCP23017 has pull-ups enabled, so LOW = pressed
  switch1_state = !mcp1.getPin(SW1_PIN);  // Invert: LOW = pressed = true
  switch2_state = !mcp1.getPin(SW2_PIN);
  switch3_state = !mcp2.getPin(SW3_PIN);
  
  // Check for encoder changes and print to Serial
  if (encoder1_value != prev_enc1) {
    int16_t delta = encoder1_value - prev_enc1;
    Serial.print("ENC1: ");
    Serial.print(encoder1_value);
    Serial.print(" (");
    Serial.print(delta > 0 ? "+" : "");
    Serial.print(delta);
    Serial.println(")");
    prev_enc1 = encoder1_value;
  }
  
  if (encoder2_value != prev_enc2) {
    int16_t delta = encoder2_value - prev_enc2;
    Serial.print("ENC2: ");
    Serial.print(encoder2_value);
    Serial.print(" (");
    Serial.print(delta > 0 ? "+" : "");
    Serial.print(delta);
    Serial.println(")");
    prev_enc2 = encoder2_value;
  }
  
  if (encoder3_value != prev_enc3) {
    int16_t delta = encoder3_value - prev_enc3;
    Serial.print("ENC3: ");
    Serial.print(encoder3_value);
    Serial.print(" (");
    Serial.print(delta > 0 ? "+" : "");
    Serial.print(delta);
    Serial.println(")");
    prev_enc3 = encoder3_value;
  }
  
  // Check for switch changes and print to Serial
  if (switch1_state != prev_sw1) {
    Serial.print("SW1: ");
    Serial.println(switch1_state ? "PRESSED" : "RELEASED");
    prev_sw1 = switch1_state;
  }
  
  if (switch2_state != prev_sw2) {
    Serial.print("SW2: ");
    Serial.println(switch2_state ? "PRESSED" : "RELEASED");
    prev_sw2 = switch2_state;
  }
  
  if (switch3_state != prev_sw3) {
    Serial.print("SW3: ");
    Serial.println(switch3_state ? "PRESSED" : "RELEASED");
    prev_sw3 = switch3_state;
  }
  
  // Update OLED display periodically
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Title
  display.setCursor(0, 0);
  display.println("=== Routing Test ===");
  
  // Encoder 1
  display.setCursor(0, 16);
  display.print("ENC1: ");
  display.print(encoder1_value);
  display.print("  SW1:");
  display.println(switch1_state ? "ON" : "--");
  
  // Encoder 2
  display.setCursor(0, 28);
  display.print("ENC2: ");
  display.print(encoder2_value);
  display.print("  SW2:");
  display.println(switch2_state ? "ON" : "--");
  
  // Encoder 3
  display.setCursor(0, 40);
  display.print("ENC3: ");
  display.print(encoder3_value);
  display.print("  SW3:");
  display.println(switch3_state ? "ON" : "--");
  
  // Status line
  display.setCursor(0, 56);
  display.print("I2C: ");
  display.print(Wire.getClock() / 1000);
  display.print("kHz");
  
  display.display();
}
