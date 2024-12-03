# TeslaLight

## ESP32 > 74 led issue
There is an issue when using the ESP32 with the NeoPixel library with more than 75 Leds
Use the following fix version of NeoPixel: https://github.com/teknynja/Adafruit_NeoPixel/tree/esp32_rmt_memory_allocation_fix_safe