#include "Adafruit_TCS34725.h"

//Color detector pins
#define COL_DET_SDA_PIN 4
#define COL_SET_SCL_PIN 5
#define COL_SET_LED_PIN 6

int COL_Sen_add = 0x29;
Adafruit_TCS34725 COL_Sensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

void crystal_LED(bool status) {
  pinMode(COL_SET_LED_PIN, !status);
}

void start_color_sensor() {
  pinMode(COL_SET_LED_PIN, OUTPUT);
  COL_Sensor.begin();
}

void get_crystal_color(int* hue, int* saturation) {
  float r, g, b;
  COL_Sensor.getRGB(&r, &g, &b);
  CHSV hsv;
  CRGB rgb(r, g, b);
  hsv = rgb2hsv_approximate(rgb);
  *hue = hsv.h;
  *saturation = hsv.s;
}

void calculate_blade_color() {
  //if (blade_color.detected_saturation <= 116) {
  //  blade_color.detected_saturation = 0;
  //} else {
  blade_color.detected_saturation = 255;
  //}
  for (int i = 0; i < 6; i++) {
    blade_color.hue[i] = blade_color.detected_hue;
    blade_color.saturation[i] = blade_color.detected_saturation;
    blade_color.brightness[i] = 255;
  }
  Serial.print("detected hue: ");
  Serial.print(blade_color.detected_hue);
  Serial.print(" detected saturation: ");
  Serial.println(blade_color.detected_saturation);
}