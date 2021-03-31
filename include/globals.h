#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <elapsedMillis.h>
#include <RF24.h>
#include <EEPROM.h>

extern RF24 nrf_radio;
#define hc12_uart Serial2 

extern EEPROMClass eeprom;
#define CH_STATE_ADDR 0
#define BD_STATE_ADDR 1
#define TP_STATE_ADDR 2
#define TM_STATE_ADDR 3

/* GPIO */
#define IRQ 3
#define CSN 10
#define CE 9
#define HC12_CMD_MODE 6
#define P1 19
#define P2 18
#define P3 4
#define P4 5
#define ROT_ENC_1A 23
#define ROT_ENC_1B 22
#define ROT_ENC_2A 21
#define ROT_ENC_2B 20
#define POT_AY 16
#define POT_AX 17
#define POT_BY 14
#define POT_BX 15

//Buttons
struct Button
{
  const int PIN;
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

extern float dr_batt_amps;
extern float dr_batt_temp;
extern float dr_batt_voltage;
extern float dr_altitude;

extern uint16_t crc_xmodem(const uint8_t *data, uint16_t len);

extern uint64_t interval1;

#endif
