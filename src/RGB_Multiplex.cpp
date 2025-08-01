#include "RGB_Multiplex.h"

#if defined(ARDUINO_ARCH_RP2040)
#include <hardware/irq.h>
#include <hardware/timer.h>
static RGBMultiplex* g_rgb_multiplex_instance = nullptr;

// Timer callback for 1ms update
int64_t rgb_multiplex_timer_callback(alarm_id_t, void*) {
  if (g_rgb_multiplex_instance) {
    g_rgb_multiplex_instance->Update();
  }
  // Re-arm for next .1 millisecond
  return 100;
}

// For RP2040: start/stop timer interrupt for multiplexing
void RGBMultiplex::StartAutoUpdate() {
  auto_update_alarm_id_ = add_alarm_in_us(100, rgb_multiplex_timer_callback, nullptr, true);
}

void RGBMultiplex::StopAutoUpdate() {
  cancel_alarm(auto_update_alarm_id_);
}
#endif

RGBMultiplex::RGBMultiplex(const uint8_t* anode_pins, uint8_t num_leds, uint8_t r_pin, uint8_t g_pin, uint8_t b_pin)
    : anode_pins_(anode_pins), num_leds_(num_leds), r_pin_(r_pin), g_pin_(g_pin), b_pin_(b_pin),
      current_led_(0), r_resistor_(0), g_resistor_(0), b_resistor_(0),
      r_vf_(0), g_vf_(0), b_vf_(0), supply_voltage_(0) {
  values_ = new RGB[num_leds_];
  for (uint8_t i = 0; i < num_leds_; ++i) {
    values_[i] = {false, false, false};
  }
}

void RGBMultiplex::Begin() {
  for (uint8_t i = 0; i < num_leds_; ++i) {
    pinMode(anode_pins_[i], OUTPUT_8MA);
    digitalWrite(anode_pins_[i], LOW);
  }
  pinMode(r_pin_, OUTPUT);
  pinMode(g_pin_, OUTPUT);
  pinMode(b_pin_, OUTPUT);
  AllOff();

  #if defined(ARDUINO_ARCH_RP2040)
    g_rgb_multiplex_instance = this;
    StartAutoUpdate();
  #endif
}

void RGBMultiplex::SetColor(uint8_t led_index, bool r, bool g, bool b) {
  if (led_index >= num_leds_) return;

#if defined(ARDUINO_ARCH_RP2040)
  static critical_section_t rgbmux_critsec;
  static bool critsec_init = false;
  if (!critsec_init) { critical_section_init(&rgbmux_critsec); critsec_init = true; }
  critical_section_enter_blocking(&rgbmux_critsec);
#endif

  values_[led_index].r = r;
  values_[led_index].g = g;
  values_[led_index].b = b;

#if defined(ARDUINO_ARCH_RP2040)
  critical_section_exit(&rgbmux_critsec);
#endif

}

void RGBMultiplex::SetColor(uint8_t led_index, Color3Bits color) {
  uint8_t val = static_cast<uint8_t>(color);
  bool r = val & 0x01;
  bool g = (val >> 1) & 0x01;
  bool b = (val >> 2) & 0x01;
  SetColor(led_index, r, g, b);
}

void RGBMultiplex::Off(uint8_t led_index) {
  if (led_index >= num_leds_) return;
  values_[led_index] = {false, false, false};
  Update();
}

void RGBMultiplex::AllOff() {
  for (uint8_t i = 0; i < num_leds_; ++i) {
    values_[i] = {false, false, false};
  }
  Update();
}

void RGBMultiplex::Update() {

  #if defined(ARDUINO_ARCH_RP2040)
    static critical_section_t rgbmux_critsec;
    static bool critsec_init = false;
    if (!critsec_init) { critical_section_init(&rgbmux_critsec); critsec_init = true; }
    critical_section_enter_blocking(&rgbmux_critsec);
  #endif
  
  digitalWrite(r_pin_, HIGH);
  digitalWrite(g_pin_, HIGH);
  digitalWrite(b_pin_, HIGH);
  for (uint8_t i = 0; i < num_leds_; ++i) {
    digitalWrite(anode_pins_[i], LOW);
  }
  // Global brightness: 0=off, 7=on every cycle, distribute ON cycles evenly
  bool on = false;
  if (global_brightness_ > 0) {
    on = (pwm_cycle_ < global_brightness_);
  }
  if (on) {
    digitalWrite(r_pin_, values_[current_led_].r ? LOW : HIGH);
    digitalWrite(g_pin_, values_[current_led_].g ? LOW : HIGH);
    digitalWrite(b_pin_, values_[current_led_].b ? LOW : HIGH);
    digitalWrite(anode_pins_[current_led_], HIGH);
  } else {
    digitalWrite(r_pin_, HIGH);
    digitalWrite(g_pin_, HIGH);
    digitalWrite(b_pin_, HIGH);
    digitalWrite(anode_pins_[current_led_], LOW);
  }
  current_led_ = (current_led_ + 1) % num_leds_;
  if (current_led_ == num_leds_ - 1) {
    pwm_cycle_ = (pwm_cycle_ + 1) % 8;
  }

  #if defined(ARDUINO_ARCH_RP2040)
    critical_section_exit(&rgbmux_critsec);
  #endif

}

void RGBMultiplex::SetGlobalBrightness(uint8_t brightness) {
  if (brightness > 8) brightness = 8;
  global_brightness_ = brightness;
}

uint8_t RGBMultiplex::GetGlobalBrightness() const {
  return global_brightness_;
}

void RGBMultiplex::SetResistorValues(float r_ohms, float g_ohms, float b_ohms) {
  r_resistor_ = r_ohms;
  g_resistor_ = g_ohms;
  b_resistor_ = b_ohms;
}

void RGBMultiplex::SetForwardVoltages(float r_vf, float g_vf, float b_vf) {
  r_vf_ = r_vf;
  g_vf_ = g_vf;
  b_vf_ = b_vf;
}

void RGBMultiplex::SetSupplyVoltage(float vcc) {
  supply_voltage_ = vcc;
}

float RGBMultiplex::GetSupplyVoltage() const {
  return supply_voltage_;
}

float RGBMultiplex::GetEstimatedCurrent() const {
  float vcc = supply_voltage_;
  float total = 0;
  for (uint8_t i = 0; i < num_leds_; ++i) {
    float ir = (r_resistor_ > 0 && values_[i].r) ? (vcc - r_vf_) / r_resistor_ : 0;
    float ig = (g_resistor_ > 0 && values_[i].g) ? (vcc - g_vf_) / g_resistor_ : 0;
    float ib = (b_resistor_ > 0 && values_[i].b) ? (vcc - b_vf_) / b_resistor_ : 0;
    float led_current = ir + ig + ib;
    if (led_current > total) total = led_current;
  }
  return total;
}

float RGBMultiplex::GetEstimatedMaxCurrent() const {
  float vcc = supply_voltage_;
  float ir = (r_resistor_ > 0) ? (vcc - r_vf_) / r_resistor_ : 0;
  float ig = (g_resistor_ > 0) ? (vcc - g_vf_) / g_resistor_ : 0;
  float ib = (b_resistor_ > 0) ? (vcc - b_vf_) / b_resistor_ : 0;
  return ir + ig + ib;
}
