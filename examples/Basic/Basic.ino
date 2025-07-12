#include <Arduino.h>
#include <RGB_Multiplex.h>

// Anode pins for 6 LEDs
constexpr uint8_t kAnodePins[6] = {4, 5, 6, 7, 8, 9};
// RGB cathode pins
constexpr uint8_t kRedPin = 11;
constexpr uint8_t kGreenPin = 12;
constexpr uint8_t kBluePin = 13;

RGBMultiplex rgb(kAnodePins, 6, kRedPin, kGreenPin, kBluePin);

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
#if defined(ARDUINO_ARCH_RP2040)
  rgb.StartAutoUpdate();
#endif
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
    Serial.print("[");
    PrintBarGraph(percent);
    Serial.print("] ");
    Serial.print(static_cast<int>(percent * 100));
    Serial.print("%  I=");
    Serial.print(total_current * 1000.0f, 1);
    Serial.print(" mA  max: ");
    Serial.print(max_current, 1);
    Serial.println(" mA");
  }

  rgb.Update();
}
