/*  
   Created by:    Wim Boone
   Project:       Hexacopter
   Subject:       Radio controller
   Date Started:  20/12/2018     
 */
#include <rc_comm.h>
#include <globals.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>

RadioCommunication rc;
EEPROMClass flash;

elapsedMillis since_int1;
elapsedMillis since_int2;
elapsedMillis since_int3;
elapsedMillis since_int4;
const long interval1 = 10;
const long interval2 = 50;
const long interval3 = 3000;
const long interval4 = 300;

#define FLASH_SIZE 1
#define CH_STATE_ADRR 0
#define BD_STATE_ADRR 1
#define TP_STATE_ADRR 2
uint8_t channel_state = flash.read(CH_STATE_ADRR);
uint8_t baud_state = flash.read(BD_STATE_ADRR);
uint8_t txpower_state = flash.read(TP_STATE_ADRR);

/* GPIO */
#define LED_GREEN GPIO_NUM_2
#define LED_RED GPIO_NUM_12
#define HC12_TX GPIO_NUM_32
#define HC12_RX GPIO_NUM_26
#define HC12_CMD_MODE GPIO_NUM_4

#define POT_AY GPIO_NUM_39
#define POT_AX GPIO_NUM_36
#define POT_BY GPIO_NUM_33
#define POT_BX GPIO_NUM_35

Button button1 = {GPIO_NUM_27, false, 0};
Button button2 = {GPIO_NUM_25, false, 0};
Button button3 = {GPIO_NUM_34, false, 0};
Button button4 = {GPIO_NUM_14, false, 0};

uint16_t pot_throttle = 0;
uint16_t pot_yaw = 0;
uint16_t pot_pitch = 0;
uint16_t pot_roll = 0;

uint8_t potlower = 0;
uint8_t pothigher = 0;
uint16_t potfinal = 0;

void IRAM_ATTR ISR()
{
  button1.state = gpio_get_level(button1.PIN);
  button2.state = gpio_get_level(button2.PIN);
  button3.state = gpio_get_level(button3.PIN);
  button4.state = gpio_get_level(button4.PIN);

  button1.since_press = millis();
  button2.since_press = millis();
  button3.since_press = millis();
  button4.since_press = millis();
}

void setup()
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  // Set output mode for some pins
  gpio_config(&io_conf);
  gpio_set_direction(HC12_TX, GPIO_MODE_OUTPUT);
  gpio_set_direction(HC12_CMD_MODE, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);

  attachInterrupt(button1.PIN, ISR, RISING);
  attachInterrupt(button2.PIN, ISR, RISING);
  attachInterrupt(button3.PIN, ISR, RISING);
  attachInterrupt(button4.PIN, ISR, RISING);
  analogReadResolution(12);
  hc12.begin((rc_baudrate[baud_state]), SERIAL_8N1, HC12_RX, HC12_TX);
  hc12.begin(4800, SERIAL_8N1, HC12_RX, HC12_TX);
  flash.begin(FLASH_SIZE);
  SD.begin(SS, SPI, 80000000, "/sd", 20);
  Serial.begin(115200);
}

void loop()
{
  pot_throttle = analogRead(POT_AY);
  pot_yaw = analogRead(POT_AX);
  pot_pitch = analogRead(POT_BY);
  pot_roll = analogRead(POT_BX);

  rc.parseIncomingBytes();

  // 10ms
  if (since_int1 > interval1)
  {
    since_int1 -= interval1;
  }

  // 50ms
  if (since_int2 > interval2)
  {
    since_int2 -= interval2;

    rc.sendBytes();
  }

  // 3000ms
  if (since_int3 > interval3)
  {
    since_int3 -= interval3;
  }

  // 300ms
  if (since_int4 > interval4)
  {
    since_int4 -= interval4;

    potlower = pot_throttle & 0xff; 
    pothigher = pot_throttle >> 8;

    potfinal = pothigher << 8 | potlower;
  
    Serial.println(" ");
    Serial.println((String) "throttle:  " + pot_throttle);
    Serial.println((String) "yaw:  " + pot_yaw);
    Serial.println((String) "pitch:  " + pot_pitch);
    Serial.println((String) "roll:  " + pot_roll);
    Serial.println(" ");
  }
}
