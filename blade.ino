#include <FastLED.h>
//LEDs
#define NUM_LEDS 828  //138 per strip 14

//LED strip pins
#define LEDS_DATA_PIN 12
#define LEDS_DETECT_PIN 8
#define ONBOARD_LED_DATA_PIN 16

const uint8_t kMatrixWidth = 6;
const uint8_t kMatrixHeight = 138;
int animation_index = 0;
int color_selector_index = 1;

//Define the arrays of leds
CRGB leds[NUM_LEDS];

void LED_setup() {
  pinMode(LEDS_DETECT_PIN, INPUT_PULLUP);
  //Setup FastLEDs
  FastLED.addLeds<WS2812B, LEDS_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(blade_color.brightness_global);
}

bool detect_blade_inserted()
{
  if(enable_blade_detect)
  {
    return !digitalRead(LEDS_DETECT_PIN);
  }
  return true;
}

void low_battery_brightness() {
  blade_color.brightness_global = blade_color.brightness_global - 100;
}

uint16_t XY(uint8_t x, uint8_t y) {
  uint16_t i;
  if (x & 0x01) {
    i = kMatrixHeight * (kMatrixWidth - (x + 1)) + y;
  } else {
    i = kMatrixHeight * (kMatrixWidth - x) - (y + 1);
  }
  return i;
}

void cycle_blade_brightness() {
  blade_color.brightness_global = blade_color.brightness_global + 50;
  if (blade_color.brightness_global > 250) {
    blade_color.brightness_global = 50;
  }
}

void cycle_predefined_blade_colors(int select_color)
{
  if(select_color > 0)
  {
    color_selector_index = select_color;
  }
  if(color_selector_index == 1) ///RED
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 0;
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 2) ///BLUE
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 160;
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 3) ///GREEN
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 96;
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 4) ///PURPLE
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 192;
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 5) ///YELLOW
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 64;
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 6) ///WHITE
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.hue[i] = 255;
      blade_color.saturation[i] = 0;
      blade_color.brightness[i] = 255;
    }
  }
  else if(color_selector_index == 7) ///LGBT
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
    blade_color.hue[0] = 0; //RED
    blade_color.hue[1] = 32; //ORANGE
    blade_color.hue[2] = 64; //YELLOW
    blade_color.hue[3] = 96; //GREEN
    blade_color.hue[4] = 160; //BLUE
    blade_color.hue[5] = 192; //PURPLE
  }
  else if(color_selector_index == 8) ///BI
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
    blade_color.hue[0] = 224; //pink
    blade_color.hue[1] = 224; //pink
    blade_color.hue[2] = 192; //purple
    blade_color.hue[3] = 160; //blue
    blade_color.hue[4] = 160; //blue
    blade_color.hue[5] = 192; //purple
  }
  else if(color_selector_index == 9) ///TRANS
  {
    for(int i = 0; i < 6; i++)
    {
      blade_color.saturation[i] = 255;
      blade_color.brightness[i] = 255;
    }
    blade_color.hue[0] = 128; //aqua
    blade_color.hue[1] = 224; //pink
    blade_color.hue[2] = 224; //pink
    blade_color.hue[3] = 128; //aqua
    blade_color.hue[4] = 224; //pink
    blade_color.hue[5] = 224; //pink
  }
  color_selector_index++;
  if(color_selector_index > 9)
  {
    color_selector_index = 1;
  }
}

bool activate_blade(int hue[6], int saturation[6], int brightness[6], int speed) {
  for (int j = 0; j < speed; j++) {
    for (int i = 0; i < 6; i++) { //we have 6 rows of led strips, we light them all up at the same time
      leds[XY(i, animation_index)] = CHSV(hue[i], saturation[i], brightness[i]);
    }
    animation_index++;
    if (animation_index >= kMatrixHeight) {
      animation_index = kMatrixHeight;
      FastLED.show();
      return true;
    }
  }
  FastLED.show();
  return false;
}

bool deactivate_blade(int hue[6], int saturation[6], int brightness[6], int speed) {
  for (int j = 0; j < speed; j++) {
    for (int i = 0; i < 6; i++) { //we have 6 rows of led strips, we light them all up at the same time
      leds[XY(i, animation_index)] = CHSV(0, 0, 0);
    }
    animation_index--;
    if (animation_index <= 0) {
      animation_index = 0;
      FastLED.show();
      return true;
    }
  }
  FastLED.show();
  return false;
}

int normal_blade(int hue[6], int saturation[6], int brightness[6]) {
  for(int i = 0; i < 6; i++)
  {
    for(int j = 0; j < kMatrixHeight; j++)
    {
      leds[XY(i, j)] = CHSV(hue[i], saturation[i], brightness[i]);
    }
  }
  FastLED.setBrightness(blade_color.brightness_global);
  FastLED.show();
  return 0;
}

int hit_blade(int hue, int saturation, int brightness) {
  if(brightness > 100)
  {
    brightness = random(brightness-100, brightness);
  }
  else
  {
    brightness = random(0, brightness);
  }
  fill_solid(leds, NUM_LEDS, CHSV(hue, saturation, brightness));
  FastLED.show();
  return 0;
}

int battery_blade(int hue[6], int saturation[6], int brightness[6], float battery_voltage) {
  battery_voltage = battery_voltage*100;
  int height = map(int(battery_voltage), 350, 430, 0, kMatrixHeight);
  //Serial.print("Voltage:");
  //Serial.print(adc_voltage);
  //Serial.print(" Height:");
  //Serial.println(height);

  //int height = map(battery_voltage, 350, 420, 0, kMatrixHeight);
  fill_solid(leds, NUM_LEDS, CHSV(0, 0, 0));
  for(int i = 0; i < 6; i++)
  {
    for(int j = 0; j < height; j++)
    {
      leds[XY(i, j)] = CHSV(hue[i], saturation[i], brightness[i]);
    }
  }
  FastLED.setBrightness(blade_color.brightness_global);
  FastLED.show();
  return 0;
}
