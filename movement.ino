#include <Wire.h>
#include <ADXL345.h>
#include <Smooth.h>

#define SMOOTHED_SAMPLE_SIZE 8

//Accelerometer pins
#define ACC_SDA_PIN 26
#define ACC_SCL_PIN 27
#define X_offset 0.01
#define Y_offset 0.03
#define Z_offset 0.19
int t_old, t_old_hit;
bool hum_track;

ADXL345 accel(ADXL345_ALT);

Smooth dcx(SMOOTHED_SAMPLE_SIZE);
Smooth dcy(SMOOTHED_SAMPLE_SIZE);
Smooth dcz(SMOOTHED_SAMPLE_SIZE);
float old_x, old_y, old_z;

void start_accelerometer() {
  Wire1.begin();
  accel.writeRate(ADXL345_RATE_800HZ);
  accel.writeRange(ADXL345_RANGE_4G);
  accel.start();
}

float get_accelerometer_data() {
  if (accel.update()) {
    float L = sqrt(pow((accel.getX() - X_offset), 2) + pow((accel.getY() - Y_offset), 2) + pow((accel.getZ() - Z_offset), 2));
    dcx.add((accel.getX() / L) + 1);
    dcy.add((accel.getY() / L) + 1);
    dcz.add((accel.getZ() / L) + 1);
    float deltax = abs(old_x - dcx.get_avg());
    float deltay = abs(old_y - dcy.get_avg());
    float deltaz = abs(old_z - dcz.get_avg());
    float sum = deltax + deltay + deltaz;
    old_x = dcx.get_avg();
    old_y = dcy.get_avg();
    old_z = dcz.get_avg();
    return sum;
  }
  return -1.0;
}

double mapf(double val, double in_min, double in_max, double out_min, double out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void movement_to_sound_controller() {
  float G_sum = get_accelerometer_data();
  G_sum = mapf(G_sum, 0, 0.2, 0, max_volume);
  float cur_time = micros();
  float delta_time = cur_time - t_old;
  if (G_sum <= 0.15) {
    change_swing_weights(max_volume, 0, 0, 0);
    if (delta_time > 1000000) {
      t_old = cur_time;
      hum_track = !hum_track;
    }
  } else {
    float normal_hum = max_volume - G_sum;
    if (hum_track) {
      change_swing_weights(normal_hum, G_sum, 0, 0);
    } else {
      change_swing_weights(normal_hum, 0, G_sum, 0);
    }
  }
}

bool hard_hit_detect() {
  if (accel.update()) {
    float cur_time = micros();
    float L = sqrt(pow((accel.getX() - X_offset), 2) + pow((accel.getY() - Y_offset), 2) + pow((accel.getZ() - Z_offset), 2));
    if (L > G_Hit) {
      t_old_hit = cur_time;
    }
    if (cur_time - t_old_hit < 100000) {
      change_swing_weights(0, 0, 0, max_volume);
      return true;
    }
  }
  return false;
}