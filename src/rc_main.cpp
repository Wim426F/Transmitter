/*  
   Created by:    Wim Boone
   Project:       Hexacopter
   Subject:       Radio controller
   Date Started:  20/12/2018     
 */
#include <rc_comm.h>
#include <globals.h>
#include <SPI.h>
#include <Encoder.h>
#include <util/crc16.h>

RF24 rf24(7, 8); // NRF24 Radio Transmitter & Receiver
HC12 radio;
EEPROMClass eeprom;

elapsedMillis since_int1;
elapsedMillis since_int2;
elapsedMillis since_int3;
elapsedMillis since_int4;
uint64_t interval1 = 10;
const uint64_t interval2 = 50;
const uint64_t interval3 = 1000;
const uint64_t interval4 = 100;

Button button1 = {P1, HIGH, 0};
Button button2 = {P2, HIGH, 0};
Button button3 = {P3, HIGH, 0};
Button button4 = {P4, HIGH, 0};

uint16_t pot_throttle = 0;
uint16_t pot_yaw = 0;
uint16_t pot_pitch = 0;
uint16_t pot_roll = 0;

float dr_batt_amps = 0;
float dr_batt_temp = 0;
float dr_batt_voltage = 0;
float dr_altitude = 0;

Encoder rot_enc1(ROT_ENC_1A, ROT_ENC_1B);
Encoder rot_enc2(ROT_ENC_2A, ROT_ENC_2B);

uint16_t crc_xmodem(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0;
  for (uint16_t i = 0; i < len; i++)
  {
    crc = crc_xmodem_update(crc, data[i]);
  }
  return crc;
}

void setup()
{
  pinMode(HC12_CMD_MODE, OUTPUT);
  digitalWrite(HC12_CMD_MODE, HIGH); // don't enter command mode
  Serial.begin(115200);
  hc12_uart.begin((rc_baudrate[last_baud]), SERIAL_8N1);
  Serial.println((String) "HC12 started at baud: " + rc_baudrate[last_baud] + "\n");

  //Serial.print("Setting channel: ");
  //radio.setChannel(1);

  //Serial.print("Setting power: ");
  //radio.setTxPower(8);
  //Serial.print("Setting baudrate: ");
  //radio.setBaudRate(7, true); // 3 = 9600bps
  /*
  Serial.print("Setting Mode: "); 
  radio.setTransmitMode(3);
  //radio.setAllDefault(); */
  radio.getVersion();
  //radio.setAllDefault();
  analogReadResolution(8);
  analogReadAveraging(32);
}

void loop()
{
  if (last_txmode == 1)
  {
    radio.PACKET_TIMEOUT = 500;
  }
  if (last_txmode == 2)
  {
    radio.PACKET_TIMEOUT = 1500;
  }
  if (last_txmode == 3)
  {
    radio.PACKET_TIMEOUT = 500;
  }
  if (last_txmode == 4)
  {
    radio.PACKET_TIMEOUT = 3000;
  }

  if (Serial.available() > 0 && radio.waiting_for_response == false)
  {
    delay(50);
    Serial.println("command mode");
    char param = Serial.read();
    int i = Serial.parseInt();

    Serial.print((String)param + (String)i);
    if (param == 'b')
    {
      radio.setBaudRate(i, true);
      delay(1000);
    }

    if (param == 'c')
    {
      radio.setChannel(i, true);
      delay(1000);
    }

    if (param == 'm')
    {
      radio.setTransmitMode(i, true);
      delay(1000);
    }

    if (param == 'p')
    {
      radio.setTxPower(i, true);
      delay(1000);
    }

    if (param == 'd')
    {
      radio.setAllDefault(true);
      delay(1000);
    }

    if (param == 's')
    {
      Serial.print("HC12 started at: ");
      EEPROM.write(BD_STATE_ADDR, i);
      last_baud = EEPROM.read(BD_STATE_ADDR);

      Serial.println(rc_baudrate[last_baud]);
      hc12_uart.end();
      hc12_uart.begin(rc_baudrate[last_baud]);
      delay(100);
    }

    if (param == 'a')
    {
      delay(2000);
      String param = Serial.readString();
      digitalWrite(HC12_CMD_MODE, LOW);
      delay(40);
      hc12_uart.print(param);
      delay(100);
      digitalWrite(HC12_CMD_MODE, HIGH);
      Serial.println(hc12_uart.readString());
      delay(1000);
    }
  }

  pot_throttle = map(analogRead(POT_AY), 5, 237, 0, 255);
  pot_yaw = map(analogRead(POT_AX), 5, 240, 0, 255);
  pot_pitch = map(analogRead(POT_BY), 0, 240, 0, 255);
  pot_roll = map(analogRead(POT_BX), 10, 250, 0, 255);

  radio.sendData();

  if (since_int1 > interval1)
  {
    since_int1 -= interval1;
  }

  // 50ms
  if (since_int2 > interval2)
  {
    since_int2 -= interval2;
  }

  // 1000ms
  if (since_int3 > interval3)
  {
    /*
    Serial.println(" ");
    Serial.print(pot_throttle);
    Serial.print(" ");
    Serial.print(pot_yaw);
    Serial.print(" ");
    Serial.print(pot_pitch);
    Serial.print(" ");
    Serial.print(pot_roll);
    Serial.println(" "); */
    if (radio.is_connected == true)
    {
      Serial.print("\nConnected  ");
    }
    else
    {
      Serial.print("\nDisconnected  ");
    }
    Serial.print("volt: ");
    Serial.print(dr_batt_voltage, 3);
    Serial.print("  temp: ");
    Serial.print(dr_batt_temp, 2);
    Serial.print("  amps: ");
    Serial.print(dr_batt_amps, 2);
    Serial.print("  pps: ");
    Serial.print(radio.packets_per_sec, 1);
    Serial.print("  resp_time: " + (String)radio.response_time);
    Serial.println("  pl: " + (String)radio.packets_lost_total);

    since_int3 -= interval3;
  }

  // 100ms
  if (since_int4 > interval4)
  {
    since_int4 -= interval4;
  }
}
