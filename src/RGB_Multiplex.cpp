#include "RGB_Multiplex.h"

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
    pinMode(anode_pins_[i], OUTPUT);
    digitalWrite(anode_pins_[i], LOW);
  }
  pinMode(r_pin_, OUTPUT);
  pinMode(g_pin_, OUTPUT);
  pinMode(b_pin_, OUTPUT);
  AllOff();
}

void RGBMultiplex::SetColor(uint8_t led_index, bool r, bool g, bool b) {
  if (led_index >= num_leds_) return;
  values_[led_index].r = r;
  values_[led_index].g = g;
  values_[led_index].b = b;
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
  for (uint8_t i = 0; i < num_leds_; ++i) {
    digitalWrite(anode_pins_[i], LOW);
  }
  digitalWrite(r_pin_, values_[current_led_].r ? HIGH : LOW);
  digitalWrite(g_pin_, values_[current_led_].g ? HIGH : LOW);
  digitalWrite(b_pin_, values_[current_led_].b ? HIGH : LOW);
  digitalWrite(anode_pins_[current_led_], HIGH);
  current_led_ = (current_led_ + 1) % num_leds_;
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
