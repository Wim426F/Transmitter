#ifndef GLOBALS_H
#define GLOBALS_H

//#include <Arduino.h>
#include <elapsedMillis.h>
#include <LiquidCrystal_I2C.h>

#define HC12 Serial1 
extern LiquidCrystal_I2C lcd;

extern uint8_t send_packet[];
extern uint8_t receive_packet[];

extern const float rc_txpower[];
extern const uint32_t rc_baudrate[];

extern uint8_t channel_state;
extern uint8_t baud_state;
extern uint8_t txpower_state;

extern String set_channel;

extern uint8_t steer_correction;

//Buttons
extern uint8_t button1;
extern uint8_t button4;
extern uint8_t button2;
extern uint8_t button3;

//Joysticks
extern uint8_t pot_throttle;
extern uint8_t pot_yaw;
extern uint8_t pot_pitch;
extern uint8_t pot_roll;

extern const uint8_t hc_set;
//extern const uint8_t led_green;
//extern const uint8_t led_red;

#endif
