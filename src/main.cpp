#include <Arduino.h>

#include "../lib/RGB_Multiplex/RGB_Multiplex.h"


// Anode pins for 6 LEDs
constexpr uint8_t kAnodePins[6] = {3, 2, 1, 6, 5, 4};
// RGB cathode pins
constexpr uint8_t kRedPin = 11;
constexpr uint8_t kGreenPin = 9;
constexpr uint8_t kBluePin = 10;

RGBMultiplex rgb(kAnodePins, 6, kRedPin, kGreenPin, kBluePin);

// 3-bit color enum: bit 2 = blue, bit 1 = green, bit 0 = red

// Array of colors for 6 LEDs
RGBMultiplex::Color3Bits colors[6] = {
  RGBMultiplex::Color3Bits::kRed,
  RGBMultiplex::Color3Bits::kGreen,
  RGBMultiplex::Color3Bits::kBlue,
  RGBMultiplex::Color3Bits::kYellow,
  RGBMultiplex::Color3Bits::kCyan,
  RGBMultiplex::Color3Bits::kMagenta
};

void ApplyColors() {
  for (uint8_t i = 0; i < 6; ++i) {
    rgb.SetColor(i, colors[i]);
  }
}

void setup() {
  rgb.Begin();
  rgb.SetResistorValues(100, 10, 10);
  rgb.SetForwardVoltages(2.2, 3.2, 3.2);
  rgb.SetSupplyVoltage(3.3);
  ApplyColors();
}

void PrintBarGraph(float percent) {
  int bars = static_cast<int>(percent * 40.0f + 0.5f);
  for (int i = 0; i < bars; ++i) Serial.print("#");
  for (int i = bars; i < 40; ++i) Serial.print(" ");
}

void loop() {
  static unsigned long last_shift = 0;
  static unsigned long last_print = 0;
  unsigned long now = millis();
  if (now - last_shift >= 1000) {
    last_shift = now;
    // Shift colors down
    RGBMultiplex::Color3Bits last = colors[5];
    for (int i = 5; i > 0; --i) {
      colors[i] = colors[i - 1];
    }
    colors[0] = last;
    ApplyColors();
  }

  if (now - last_print >= 1000) {
    last_print = now;
    float total_current = rgb.GetEstimatedCurrent();
    float max_current = rgb.GetEstimatedMaxCurrent() * 1000.0f;
    float percent = (max_current > 0) ? (total_current * 1000.0f / max_current) : 0.0f;
    Serial.print("\033[32m[");
    PrintBarGraph(percent);
    Serial.print("] ");
    Serial.print(static_cast<int>(percent * 100));
    Serial.print("%  I=");
    Serial.print(total_current * 1000.0f, 1);
    Serial.print(" mA  max: ");
    Serial.print(max_current, 1);
    Serial.println(" mA\033[0m");
  }

  rgb.Update();
}
