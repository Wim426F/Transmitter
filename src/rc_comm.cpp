#include <rc_comm.h>
#include <globals.h>

uint8_t last_channel = eeprom.read(CH_STATE_ADDR);
uint8_t last_baud = eeprom.read(BD_STATE_ADDR);
uint8_t last_txpower = eeprom.read(TP_STATE_ADDR);
uint8_t last_txmode = eeprom.read(TM_STATE_ADDR);

const float rc_txpower[8] = {0.8, 1.6, 3.2, 6.3, 12, 25, 50, 100};
const uint32_t rc_baudrate[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

uint8_t send_packet[25];
uint8_t receive_packet[25];

elapsedMillis since_send;

HC12::rc_ack_t HC12::setReceiverSettings(enum HC12::MODE param, byte value)
{
  ACK ack;
  uint8_t crc_length = 4;

  send_packet[0] = TRANSMITTER_CMD; // 'T'
  send_packet[1] = param;
  send_packet[2] = 1; // length of actual data content (default only 4 joysticks)

  send_packet[3] = value;

  uint16_t crc = crc_xmodem(send_packet, crc_length); //crc

  send_packet[4] = crc >> 8;   // Low byte
  send_packet[5] = crc & 0xff; // High byte

  for (int i = 0; i < crc_length + 2; i++)
  {
    hc12_uart.write(send_packet[i]);
  }

  uint64_t waiting = 0;

  while (!hc12_uart.available())
  {
    delay(1);
    waiting++;
    if (waiting > 3000)
    {
      Serial.println("Receiver didn't answer");
      ack = ACK::TIMEOUT;
      break;
    }
  }
  if (hc12_uart.available() > 0)
  {
    ack = readData();
    if (ack == OK)
    {
      Serial.println("Receiver set succesfully");
    }
    if (ack == CMD_FAILED)
    {
      Serial.println("failed to set receiver");
    }
  }
  return ack;
}

HC12::rc_ack_t HC12::setChannel(int new_channel, bool set_receiver)
{
  ACK ack;
  if (new_channel != last_channel)
  {
    if (set_receiver)
    {
      setReceiverSettings(SET_CHANNEL, new_channel);
    }
    delay(2000); // hc12_uart can't take command mode faster than 2 sec after transmission

    String set_channel;
    if (new_channel < 10)
    {
      set_channel = "AT+C00" + (String)new_channel;
    }
    if (new_channel >= 10 && new_channel < 100)
    {
      set_channel = "AT+C0" + (String)new_channel;
    }
    if (new_channel >= 100)
    {
      set_channel = "AT+C" + (String)new_channel;
    }

    digitalWrite(HC12_CMD_MODE, LOW);
    delay(40);

    hc12_uart.println(set_channel);
    Serial.println(new_channel);

    String feedback = hc12_uart.readString();
    if (feedback.indexOf("OK+C") != -1)
    {
      Serial.print("Succes! : ");
      Serial.println(feedback);
      EEPROM.write(CH_STATE_ADDR, new_channel);
      last_channel = EEPROM.read(CH_STATE_ADDR);
      ack = ACK::OK;
    }
    else
    {
      Serial.print("Failed! : ");
      Serial.println(feedback);
    }
    digitalWrite(HC12_CMD_MODE, HIGH);
    delay(80);
  }
  else
  {
    Serial.println((String) "Channel is already " + new_channel);
    ack = ACK::OK;
  }

  return ack;
}

HC12::rc_ack_t HC12::setTxPower(int new_txpower, bool set_receiver)
{
  ACK ack;
  if (new_txpower != last_txpower)
  {
    if (set_receiver)
    {
      setReceiverSettings(SET_TXPOWER, new_txpower);
    }
    delay(2000);

    digitalWrite(HC12_CMD_MODE, LOW);
    delay(40);
    Serial.println((String) "AT+P" + (new_txpower));
    hc12_uart.println((String) "AT+P" + (new_txpower));

    String feedback = hc12_uart.readString();
    if (feedback.indexOf("OK+P") != -1)
    {
      Serial.print("Succes! : ");
      Serial.println(feedback);
      EEPROM.write(TP_STATE_ADDR, new_txpower);
      last_txpower = EEPROM.read(TP_STATE_ADDR);
      Serial.println((String)rc_txpower[new_txpower - 1] + "mW");
      ack = ACK::OK;
    }
    else
    {
      Serial.println("Failed!");
      Serial.println(feedback);
      ack = ACK::CMD_FAILED;
    }

    digitalWrite(HC12_CMD_MODE, HIGH);
    delay(80);
  }
  else
  {
    Serial.println((String) "Transmit power is already " + rc_txpower[new_txpower - 1] + "mW");
    ack = ACK::OK;
  }
  return ack;
}

HC12::rc_ack_t HC12::setBaudRate(int new_baud, bool set_receiver)
{
  ACK ack;
  if (set_receiver)
  {
    setReceiverSettings(SET_BAUD, new_baud);
  }
  delay(2000);

  digitalWrite(HC12_CMD_MODE, LOW); // Then change transmitter param
  delay(40);

  hc12_uart.println("AT+B" + (String)rc_baudrate[new_baud]);
  Serial.println("AT+B" + (String)rc_baudrate[new_baud]);

  String feedback = hc12_uart.readString();
  if (feedback.indexOf("OK+B") != -1)
  {
    Serial.print("Succes! : ");
    Serial.println(feedback);
    EEPROM.write(BD_STATE_ADDR, new_baud);
    last_baud = EEPROM.read(BD_STATE_ADDR);

    hc12_uart.end();
    hc12_uart.begin(rc_baudrate[last_baud], SERIAL_8N1);
  }
  else
  {
    Serial.print("Failed! : ");
    Serial.println(feedback);
    ack = ACK::CMD_FAILED;
  }
  digitalWrite(HC12_CMD_MODE, HIGH);
  delay(80);

  return ack;
}

HC12::rc_ack_t HC12::setTransmitMode(int new_mode, bool set_receiver)
{
  ACK ack;
  if (new_mode != last_txmode)
  {
    if (set_receiver)
    {
      setReceiverSettings(SET_MODE, new_mode);
    }
    delay(2000);

    digitalWrite(HC12_CMD_MODE, LOW);
    delay(40);
    hc12_uart.println("AT+FU" + (String)new_mode);
    Serial.println("AT+FU" + (String)new_mode);

    String feedback = hc12_uart.readString();
    if (feedback.indexOf("OK+FU") != -1)
    {
      if (new_mode == 4)
      {
        Serial.println("mode 4: baudrate set to 1200bps");
        EEPROM.write(BD_STATE_ADDR, 0); // set baudrate to 1200bps
        last_baud = EEPROM.read(BD_STATE_ADDR);
        hc12_uart.end();
        hc12_uart.begin(rc_baudrate[last_baud], SERIAL_8N1);
      }

      if (new_mode == 2)
      {
        if (rc_baudrate[last_baud] > 4800)
        {
          Serial.println("mode 2: baudrate set to 4800bps");
          EEPROM.write(BD_STATE_ADDR, 2); // set baudrate to 4800bps
          last_baud = EEPROM.read(BD_STATE_ADDR);
          hc12_uart.end();
          hc12_uart.begin(rc_baudrate[last_baud], SERIAL_8N1);
        }
      }

      Serial.print("Succes! : ");
      Serial.println(feedback);
      EEPROM.write(TM_STATE_ADDR, new_mode);
      last_txmode = EEPROM.read(TM_STATE_ADDR);
      ack = ACK::OK;
    }
    else
    {
      Serial.print("Failed! : ");
      Serial.println(feedback);
      ack = ACK::CMD_FAILED;
    }

    digitalWrite(HC12_CMD_MODE, HIGH);
    delay(80);
  }
  else
  {
    Serial.println((String) "Transmit Mode is already " + new_mode);
    ack = ACK::OK;
  }
  return ack;
}

HC12::rc_ack_t HC12::setAllDefault(bool set_receiver)
{
  ACK ack;
  if (set_receiver)
  {
    setReceiverSettings(SET_DEFAULT);
  }
  delay(2000);

  digitalWrite(HC12_CMD_MODE, LOW);
  delay(40);
  hc12_uart.println("AT+DEFAULT");
  Serial.println("AT+DEFAULT");

  String feedback = hc12_uart.readString();
  if (feedback.indexOf("OK+DEFAULT") != -1)
  {
    Serial.print("Succes! : ");
    Serial.println(feedback);
    Serial.println("channel: 1, baud: 9600, txpower: 100mw, transmit_mode: FU3");
    EEPROM.write(CH_STATE_ADDR, 1); // channel 1
    EEPROM.write(BD_STATE_ADDR, 3); // baudrate of 9600
    EEPROM.write(TP_STATE_ADDR, 8); // 100mW
    EEPROM.write(TM_STATE_ADDR, 3); // FU3

    last_channel = EEPROM.read(CH_STATE_ADDR);
    last_baud = EEPROM.read(BD_STATE_ADDR);
    last_txpower = EEPROM.read(TP_STATE_ADDR);
    last_txmode = EEPROM.read(TM_STATE_ADDR);

    hc12_uart.end();
    hc12_uart.begin(rc_baudrate[last_baud], SERIAL_8N1);
    ack = ACK::OK;
  }
  else
  {
    Serial.print("Failed! : ");
    Serial.println(feedback);
    ack = ACK::CMD_FAILED;
  }

  digitalWrite(HC12_CMD_MODE, HIGH);
  delay(80);
  return ack;
}

void HC12::getVersion()
{
  digitalWrite(HC12_CMD_MODE, LOW);
  delay(40);
  hc12_uart.println("AT+V");
  Serial.println("AT+V");

  String feedback = hc12_uart.readString();
  if (feedback.indexOf("HC") != -1)
  {
    Serial.print("Firmware version : ");
    Serial.println(feedback);
  }
  else
  {
    Serial.print("Get version failed! : ");
    Serial.println(feedback);
  }

  digitalWrite(HC12_CMD_MODE, HIGH);
  delay(80);
}

void HC12::sleep()
{
  digitalWrite(HC12_CMD_MODE, LOW);
  delay(40);
  hc12_uart.println("AT+SLEEP");

  String feedback = hc12_uart.readString();
  if (feedback.indexOf("OK+SLEEP") != -1)
  {
    Serial.println("HC12 entered sleep mode");
  }
  else
  {
    Serial.print("HC12 not sleepy! : ");
    Serial.println(feedback);
  }

  digitalWrite(HC12_CMD_MODE, HIGH);
  delay(80);
}

void HC12::wakeUp()
{
  digitalWrite(HC12_CMD_MODE, LOW);
  delay(40);

  hc12_uart.println("AT");
  String feedback = hc12_uart.readString();

  if (feedback.indexOf("OK") != -1)
  {
    Serial.println("HC12 exited sleep mode");
  }
  else
  {
    Serial.print("HC12 didn't wake up! : ");
    Serial.println(feedback);
  }
  digitalWrite(HC12_CMD_MODE, HIGH);
  delay(80);
}

HC12::rc_ack_t HC12::sendData(int extra_data)
{
  ACK ack;

  if (packets_lost > MAX_PACKETS_LOST)
  {
    is_connected = false;
    packets_lost_total += packets_lost; // save all packages ever lost and reset packets lost since connect
    packets_lost = 0;
  }
  else
  {
    is_connected = true;
  }

  packets_per_sec = 1000 / response_time;

  if (waiting_for_response == false) // send data
  {
    uint8_t crc_length = 7 + extra_data;

    send_packet[0] = TRANSMITTER_CMD; // 'T'
    send_packet[1] = MODE::NORMAL;    // 'R'
    send_packet[2] = 4 + extra_data;  // length of actual data content (default only 4 joysticks)

    send_packet[3] = pot_throttle;
    send_packet[4] = pot_yaw;
    send_packet[5] = pot_pitch;
    send_packet[6] = pot_roll;

    uint16_t crc = crc_xmodem(send_packet, crc_length); //crc

    if (extra_data == 0)
    {
      send_packet[7] = crc >> 8;   // Low byte
      send_packet[8] = crc & 0xff; // High byte
    }

    if (extra_data == 4)
    {
      send_packet[7] = button1.state;
      send_packet[8] = button2.state;
      send_packet[9] = button3.state;
      send_packet[10] = button4.state;

      send_packet[11] = crc >> 8;
      send_packet[12] = crc & 0xff;
    }

    for (int i = 0; i < crc_length + 2; i++)
    {
      hc12_uart.write(send_packet[i]);
    }

    //Serial.println("Sending package...");
    since_send = 0;
    waiting_for_response = true;
  }

  if (waiting_for_response == true) // wait for incoming data
  {
    if (since_send > PACKET_TIMEOUT)
    {
      Serial.println("  Package timeout!");
      waiting_for_response = false; // otherwise it stops sending packages
      packets_lost++;
      since_send = 0;
      ack = ACK::TIMEOUT;
    }

    if (readData() == ACK::OK)
    {
      ack = ACK::OK;
    }
  }

  return ack;
}

HC12::rc_ack_t HC12::readData()
{
  ACK ack;
  if (hc12_uart.available() > 0)
  {
    //Serial.print("  Answer received: ");
    byte incoming_bytes = 0;

    for (int i = 0; hc12_uart.available() > 0; i++)
    {
      receive_packet[i] = hc12_uart.read();
      long serial_delay = 1000000 / (rc_baudrate[last_baud] / 8) * 1.5;
      delayMicroseconds(serial_delay);
      incoming_bytes++;
      //Serial.print(receive_packet[i]);
      //Serial.print(" ");
    }

    if (receive_packet[0] == RECEIVER_ANSWER)
    {
      uint16_t local_crc = crc_xmodem(receive_packet, incoming_bytes - 2);
      uint16_t receiver_crc = receive_packet[incoming_bytes - 2] << 8 | receive_packet[incoming_bytes - 1];

      /*
      Serial.print("  crc received: ");
      Serial.print(receiver_crc);
      Serial.print("  crc local: ");
      Serial.println(local_crc); */

      if (local_crc == receiver_crc)
      {
        if (receive_packet[1] == ACK::OK)
        {
          if (receive_packet[2] == 6)
          {
            uint16_t i_batt_voltage = receive_packet[3] << 8 | receive_packet[4]; // floats take up too much space as payload
            uint16_t i_batt_temp = receive_packet[5] << 8 | receive_packet[6];
            uint16_t i_batt_amps = receive_packet[7] << 8 | receive_packet[8];
            dr_batt_voltage = (float)i_batt_voltage / 1000;
            dr_batt_temp = (float)i_batt_temp / 100;
            dr_batt_amps = (float)i_batt_amps / 100;
          }
        }

        if (receive_packet[1] == ACK::CMD_INVALID)
        {
          ack = ACK::CMD_INVALID;
          Serial.println("COMMAND_INVALID\n\r");
        }

        if (receive_packet[1] == ACK::INVALID_CRC)
        {
          ack = ACK::INVALID_CRC;
          Serial.println("INVALID_CRC\n\r");
        }

        ack = ACK::OK;
      }

      packets_lost = 0;
      waiting_for_response = false;
      response_time = since_send;
    }
    else
    {
      ack = ACK::CMD_INVALID;
    }
  }
  else
  {
    ack = ACK::NOT_READY;
  }
  return ack;
}
