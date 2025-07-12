# RGB_Multiplex Arduino Library

Multiplexing library for multiple common-anode RGB LEDs with current estimation and flexible configuration.

## Features
- Supports an arbitrary number of common-anode RGB LEDs
- Per-channel resistor and forward voltage configuration
- Supply voltage configuration
- Current estimation for present and max draw
- Bar graph output for current draw
- Google C++ style, clean and idiomatic

## Example Usage
See `examples/Basic/Basic.ino` for a complete usage example.

## Installation
1. Copy the `lib/RGB_Multiplex` folder into your Arduino `libraries` directory, or install via Library Manager (if published).
2. Include the library in your sketch:
   ```cpp
   #include <RGB_Multiplex.h>
   ```

## API
- `RGBMultiplex(const uint8_t* anode_pins, uint8_t num_leds, uint8_t r_pin, uint8_t g_pin, uint8_t b_pin);`
- `void Begin();`
- `void SetColor(uint8_t led_index, bool r, bool g, bool b);`
- `void SetColor(uint8_t led_index, Color3Bits color);`
- `void Off(uint8_t led_index);`
- `void AllOff();`
- `void Update();`
- `void SetResistorValues(float r_ohms, float g_ohms, float b_ohms);`
- `void SetForwardVoltages(float r_vf, float g_vf, float b_vf);`
- `void SetSupplyVoltage(float vcc);`
- `float GetSupplyVoltage() const;`
- `float GetEstimatedCurrent() const;`
- `float GetEstimatedMaxCurrent() const;`

## License
MIT
