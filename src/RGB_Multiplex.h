#pragma once

#include <Arduino.h>

#if defined(ARDUINO_ARCH_RP2040)
#include <hardware/irq.h>
#include <hardware/timer.h>
#endif

class RGBMultiplex {
 public:
  // 3-bit color enum: bit 2 = blue, bit 1 = green, bit 0 = red
  enum class Color3Bits : uint8_t {
    kOff     = 0b000,
    kRed     = 0b001,
    kGreen   = 0b010,
    kYellow  = 0b011,
    kBlue    = 0b100,
    kMagenta = 0b101,
    kCyan    = 0b110,
    kWhite   = 0b111
  };
  RGBMultiplex(const uint8_t* anode_pins, uint8_t num_leds, uint8_t r_pin, uint8_t g_pin, uint8_t b_pin);
  void Begin();
  void SetColor(uint8_t led_index, bool r, bool g, bool b);
  void SetColor(uint8_t led_index, Color3Bits color);
  void Off(uint8_t led_index);
  void AllOff();
  void Update();

  // Global brightness: 1-8 (default 8 = max)
  void SetGlobalBrightness(uint8_t brightness);
  uint8_t GetGlobalBrightness() const;

  void SetResistorValues(float r_ohms, float g_ohms, float b_ohms);
  void SetForwardVoltages(float r_vf, float g_vf, float b_vf);
  void SetSupplyVoltage(float vcc);
  float GetSupplyVoltage() const;
  float GetEstimatedCurrent() const;
  float GetEstimatedMaxCurrent() const;

#if defined(ARDUINO_ARCH_RP2040)
  void StartAutoUpdate();
  void StopAutoUpdate();
#endif

  private:
  struct RGB {
    bool r, g, b;
  };
  uint8_t global_brightness_ = 8; // 1-8, default max
  uint8_t pwm_cycle_ = 0;
  const uint8_t* anode_pins_;
  uint8_t num_leds_;
  uint8_t r_pin_, g_pin_, b_pin_;
  RGB* values_;
  uint8_t current_led_;
  float r_resistor_, g_resistor_, b_resistor_;
  float r_vf_, g_vf_, b_vf_;
  float supply_voltage_;

#if defined(ARDUINO_ARCH_RP2040)
  alarm_id_t auto_update_alarm_id_;
#endif

};
