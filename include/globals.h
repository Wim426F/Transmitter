#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <elapsedMillis.h>

#define hc12 Serial2 
#define HC12_CMD_MODE GPIO_NUM_4

extern uint8_t send_packet[];
extern uint8_t receive_packet[];

extern const float rc_txpower[];
extern const uint32_t rc_baudrate[];

extern uint8_t channel_state;
extern uint8_t baud_state;
extern uint8_t txpower_state;

extern String set_channel;

//Buttons
struct Button
{
  gpio_num_t PIN;
  int state;
  uint64_t since_press;
};
extern Button button1;
extern Button button2;
extern Button button3;
extern Button button4;

//Joysticks
extern uint16_t pot_throttle;
extern uint16_t pot_yaw;
extern uint16_t pot_pitch;
extern uint16_t pot_roll;

//extern const uint8_t LED_GREEN;
//extern const uint8_t LED_RED;

#endif
