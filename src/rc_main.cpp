/*  
   Created by:    Wim Boone
   Project:       Hexacopter
   Subject:       Radio controller
   Date Started:  20/12/2018     
 */
#include <rc_comm.h>
#include <globals.h>
#include <EEPROM.h>

// interface variables
uint8_t menu = 0;
const uint8_t max_pages = 2;
uint8_t on_pageload = 0;
bool blink_cursor = true;
uint8_t move_vert = 1;

const uint8_t ch_state_adrr = 0;
const uint8_t bd_state_adrr = 1;
const uint8_t tp_state_adrr = 2;
uint8_t channel_state = EEPROM.read(ch_state_adrr);
uint8_t baud_state = EEPROM.read(bd_state_adrr);
uint8_t txpower_state = EEPROM.read(tp_state_adrr);

uint8_t steer_correction = 0;

elapsedMillis since_int1;
elapsedMillis since_int2;
elapsedMillis since_int3;
elapsedMillis since_int4;
const long interval1 = 10;
const long interval2 = 50;
const long interval3 = 3000;
const long interval4 = 300;

unsigned long current_millis = 0;
unsigned long last_millis = 0;
const long long_press = 1000;

const uint8_t hc_set = 2;
//const uint8_t led_green = 8;
//const uint8_t led_red = 7;

//Buttons
const uint8_t button1_pin = 27;
const uint8_t button2_pin = 25;
const uint8_t button3_pin = 34;
const uint8_t button4_pin = 14;

uint8_t button1 = 0;
uint8_t button2 = 0;
uint8_t button3 = 0;
uint8_t button4 = 0;

//Joysticks
const uint8_t pot_ay = 36;
const uint8_t pot_ax = 39;
const uint8_t pot_by = 35;
const uint8_t pot_bx = 33;

uint8_t pot_throttle = 0;
uint8_t pot_yaw = 0;
uint8_t pot_pitch = 0;
uint8_t pot_roll = 0;

int on_press = 0;

#define HC12 Serial1
LiquidCrystal_I2C lcd(0x27, 20, 4); // Display  I2C 20 x 4
RadioCommunication rc;
#define EEPROM_SIZE 1

int interface(int);
void changeParam();

void setup()
{
  //Setting buttons as input
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  pinMode(button3_pin, INPUT_PULLUP);
  pinMode(button4_pin, INPUT_PULLUP);
  pinMode(hc_set, OUTPUT);
  digitalWrite(hc_set, HIGH);
  lcd.init();
  lcd.backlight();
  
  HC12.begin((rc_baudrate[baud_state]));
  //EEPROM.write(ch_state_adrr, 0);
  EEPROM.begin(EEPROM_SIZE);
}

void loop()
{
  current_millis = millis();

  pot_throttle = (uint8_t)(analogRead(pot_ay) / 4);
  pot_yaw = (uint8_t)(analogRead(pot_ax) / 4);
  pot_pitch = (uint8_t)(analogRead(pot_by) / 4);
  pot_roll = (uint8_t)(analogRead(pot_bx) / 4);

  button1 = digitalRead(button1_pin);
  button2 = digitalRead(button2_pin);
  button3 = digitalRead(button3_pin);
  button4 = digitalRead(button4_pin);

  if (button3 == 0 && button4 == 0) // switch between menu's
  {
    if (on_press < 1) // start waiting for long press
    {
      on_press = 1;
      last_millis = millis();
    }

    if ((current_millis - last_millis) > long_press) // increment menu after button was pressed long enough
    {
      last_millis = millis();
      menu += 1;
      lcd.clear();
      if (menu > max_pages)
      {
        menu = 0;
      }
      on_press = 0;
      on_pageload = 0;
      //Serial.println(menu);
    }
  }
  else
  {
    on_press = 0;
    last_millis = millis();
  }

  // 10ms
  if (since_int1 > interval1)
  {
    since_int1 -= interval1;
  }

  // 50ms
  if (since_int2 > interval2)
  {
    since_int2 -= interval2;
    if (menu == 0)
    {
      rc.transmit();
    }
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
    //rc.receive();
    interface(menu);
  }
}

int interface(int page)
{
  if (page == 1)
  {
    changeParam();
    lcd.cursor();
    lcd.setCursor(0, 0);
    lcd.print("Config: Transmitter");
    lcd.setCursor(0, 1);
    lcd.print((String) "Tx: " + (rc_txpower[txpower_state]) + "mw  ");
    lcd.setCursor(0, 2);
    lcd.print((String) "Ch: " + channel_state + " ");
    lcd.setCursor(0, 3);
    lcd.print((String) "Bd: " + (rc_baudrate[baud_state]) + "b/s      ");
    lcd.setCursor(4, move_vert);
  }
  return 0;
}

void changeParam()
{
  if (move_vert == 1) // tx power parameter
  {
    if (button3 == 0 && txpower_state > 0)
    {
      delay(200);
      txpower_state -= 1;
    }
    if (button4 == 0 && txpower_state < 7)
    {
      delay(200);
      txpower_state += 1;
    }
    if (button2 == 0 && EEPROM.read(tp_state_adrr) != txpower_state)
    {
      EEPROM.update(tp_state_adrr, txpower_state);
      rc.setTxPower((rc_txpower[txpower_state]));
      lcd.setCursor(4, 1);
      lcd.print("OK      ");
      delay(500);
    }
  }
  if (move_vert == 2) // channel parameter
  {
    if (button3 == 0 && channel_state > 0)
    {
      delay(200);
      channel_state -= 1;
    }
    if (button4 == 0 && channel_state < 110)
    {
      delay(200);
      channel_state += 1;
    }
    if (channel_state == 110)
    {
      channel_state = 0;
    }

    if (button2 == 0 && EEPROM.read(ch_state_adrr) != channel_state)
    {
      EEPROM.update(ch_state_adrr, channel_state);
      rc.setChannel(channel_state);
      lcd.setCursor(4, 2);
      lcd.print("OK     ");
      delay(500);
    }
  }
  if (move_vert == 3) // baudrate parameter
  {
    if (button3 == 0 && baud_state > 0)
    {
      delay(200);
      baud_state -= 1;
    }
    if (button4 == 0 && baud_state < 8)
    {
      delay(200);
      baud_state += 1;
    }
    if (baud_state == 8)
    {
      baud_state = 0;
    }
    baud_state = constrain(baud_state, 0, 8);

    if (button2 == 0 && EEPROM.read(bd_state_adrr) != baud_state)
    {
      EEPROM.update(bd_state_adrr, baud_state);
      rc.setBaudRate((rc_baudrate[baud_state]));
      lcd.setCursor(4, 3);
      lcd.print("OK       ");
      delay(500);
    }
  }
}