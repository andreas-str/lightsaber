#include <PinButton.h>
#include <Smooth.h>
#define FASTLED_RP2040_CLOCKLESS_M0_FALLBACK 0
//Define the power pins
#define KEEP_ALIVE_PIN 14
#define POWER_SWITCH_SENSE_PIN 15
#define ADC_BATTERY_LEVEL 29

float max_volume = 0.6; //Max volume allowed, on a scale of 0.0 to 1.0
int G_Hit = 5;           //G amount before a hit is registered.
int default_brightness = 240; //value between 1 and 255
int default_color = 7; // set one of the default colors. 0 = disabled, 1=red, 2=blue, 3=green, 4=purple, 5=yellow, 6=white, 7=lgbt
bool enable_crystal = false; //if enabled, the color scanned in the chamber will be the default color. If true, set default_color = 0;
bool enable_single_click = true; //single click is used to check battery level
bool enable_double_click = true; // double click is used to cycle the different colors.
bool enable_blade_detect = true; //enable so that the hilt becomes inactive if no blade is detected


//Setup a button object
PinButton power_button(POWER_SWITCH_SENSE_PIN);

//Setup the battery monitoring section
Smooth adc_voltage_buffer(30);
float adc_voltage;

//State machine variables
enum STATEMACHINE_STATE { INIT,
                          NO_BLADE,
                          LIVE_CHANGE,
                          STARTUP_SOUND,
                          STARTUP_BLADE,
                          ACTIVE,
                          HIT,
                          BATTERY_BLADE,
                          SHUTDOWN_SOUND,
                          SHUTDOWN_BLADE,
                          POWEROFF };
STATEMACHINE_STATE CURRENT_STATE = INIT;
STATEMACHINE_STATE NEXT_STATE;

struct blade_type {
  int detected_hue;
  int detected_saturation;
  int hue[6];
  int hue_hit = 220;
  int saturation[6];
  int saturation_hit = 10;
  int brightness[6];
  int brightness_hit = 255;
  int speed;
  int animation;
  int brightness_global = default_brightness;
} blade_color;

void setup() {
  //Setup Power related GPIO
  pinMode(KEEP_ALIVE_PIN, OUTPUT);
  digitalWrite(KEEP_ALIVE_PIN, HIGH);
  //Start Watchdog
  rp2040.wdt_begin(4000);
  rp2040.wdt_reset();
  //Start serial interface
  Serial.begin(115200);
  //Setup FastLEDs
  LED_setup();
  //Start ADXL345 Accelerometer
  start_accelerometer();
  if(enable_crystal)
  {
    //Start TCS3472 Color Sensor
    start_color_sensor();
    //Set crystal LED to ON
    crystal_LED(HIGH);
  }
  else
  {
    //Set crystal LED to OFF
    crystal_LED(LOW);
  }
}

void loop() {
  // Check if we need to change states
  if (NEXT_STATE != CURRENT_STATE) {
    CURRENT_STATE = NEXT_STATE;
  }
  //Kick the dog
  rp2040.wdt_reset();
  //Run the button controller in a loop forever
  power_button.update();
  //Read the battery voltage and add it to the buffer
  adc_voltage_buffer.add(analogRead(ADC_BATTERY_LEVEL));
  //Convert adc reading to the real voltage. 1.99 is a coef for the voltage divider (10k/10k)
  adc_voltage = adc_voltage_buffer.get_avg() * (3.27 / 1023.0) * 1.99;
  //map battery voltage to a percentage. Not very accurate, but good enough.
  //Serial.println(adc_voltage);

  switch (CURRENT_STATE) {
    case INIT:  //Startup, we are only here once.
      if(enable_crystal)
      {
        //get the color of the crystal first, this is a blocking call, so when its done, we move to the next state.
        get_crystal_color(&blade_color.detected_hue, &blade_color.detected_saturation);
        get_crystal_color(&blade_color.detected_hue, &blade_color.detected_saturation);
        // based on the read color, set the right color for the blade
        calculate_blade_color();
        //set blade speed here
      }
      else
      {
        //If crystal is disabled, use the cycle function to set the color to a predefined value.
        cycle_predefined_blade_colors(default_color);
      }
      blade_color.speed = 5;
      //Start audio output
      start_audio_worker();
      if(detect_blade_inserted())
      {
        NEXT_STATE = STARTUP_SOUND;
      }
      else
      {
        NEXT_STATE = NO_BLADE;
      }
      break;
    case NO_BLADE:
      change_swing_weights(0, 0, 0, 0.3);
      if(power_button.isLongClick())
      {
        NEXT_STATE = SHUTDOWN_SOUND;
      }
      break;
    case LIVE_CHANGE:  //Live change of the color of the blade. We can come here with a button press event. This replaces the init function, and gets the color from memory instead of reading teh crystal.
      //set color blade here
      cycle_predefined_blade_colors(0);
      NEXT_STATE = ACTIVE;
      break;
    case STARTUP_SOUND:
      startup_sound();  //Start playing the startup sound, this runs in the background so we can get out of here asap.
      NEXT_STATE = STARTUP_BLADE;
      break;
    case STARTUP_BLADE:
      if (activate_blade(blade_color.hue, blade_color.saturation, blade_color.brightness, blade_color.speed)) {  //Loop here until the blade is fully on.
        NEXT_STATE = ACTIVE;
      }
      break;
    case ACTIVE:
      normal_blade(blade_color.hue, blade_color.saturation, blade_color.brightness);  //set normal color mode
      movement_to_sound_controller();                                                 //run controller that creates sounds based on movements
      if (hard_hit_detect()) {                                                        //if we detect a hard hit, go to hit state
        NEXT_STATE = HIT;
      }
      if(power_button.isSingleClick() && enable_single_click == true)
      {
        NEXT_STATE = BATTERY_BLADE;
      }
      if(power_button.isDoubleClick() && enable_double_click == true)
      {
        NEXT_STATE = LIVE_CHANGE;
      }
      if(power_button.isLongClick())
      {
        NEXT_STATE = SHUTDOWN_SOUND;
      }
      if(adc_voltage < 3.55)
      {
        low_battery_brightness();
      }
      break;
    case HIT:
      hit_blade(blade_color.hue_hit, blade_color.saturation_hit, blade_color.brightness_hit);  //set blade to hit color
      if (!hard_hit_detect())                                                                  //if hard hit is cleared, return to active mode
      {
        NEXT_STATE = ACTIVE;
      }
      break;
    case BATTERY_BLADE:
      battery_blade(blade_color.hue, blade_color.saturation, blade_color.brightness, adc_voltage);  //set blade to hit color
      if(power_button.isSingleClick() && enable_single_click == true)
      {
        NEXT_STATE = ACTIVE;
      }
      if(power_button.isLongClick())
      {
        NEXT_STATE = SHUTDOWN_SOUND;
      }
      break;
    case SHUTDOWN_SOUND:
      shutdown_sound();
      NEXT_STATE = SHUTDOWN_BLADE;
      break;
    case SHUTDOWN_BLADE:
      if (deactivate_blade(blade_color.hue, blade_color.saturation, blade_color.brightness, blade_color.speed)) {  //Loop here until the blade is fully off.
        NEXT_STATE = POWEROFF;
      }
      break;
    case POWEROFF:
      digitalWrite(KEEP_ALIVE_PIN, LOW);
      break;
  }
}