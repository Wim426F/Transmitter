#include <rc_comm.h>
#include <globals.h>

const float rc_txpower[8] = {0.8, 1.6, 3.2, 6.3, 12, 25, 50, 100};
const uint32_t rc_baudrate[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

uint8_t send_packet[16];
uint8_t receive_packet[8];

const uint8_t DATA_BYTES = 255;
const uint8_t CONFIG_BYTES = 127;

uint16_t local_checksum = 0;

const uint8_t params_amount = 6;

int RadioCommunication::setChannel(int channel)
{
  String set_channel;
  if (channel < 10)
  {
    set_channel = "AT+C00" + (String)channel;
  }
  else
  {
    set_channel = "AT+C0" + (String)channel;
  }
  // Change receiver param first
  hc12.write(255); // let the drone know that we are sending parameters
  hc12.write((uint8_t)channel);

  gpio_set_level(HC12_CMD_MODE, LOW);
  delay(40);
  // Then change transmitter param
  hc12.println(set_channel);
  Serial.println(set_channel);
  delay(20);
  gpio_set_level(HC12_CMD_MODE, HIGH);
  delay(80);
  return 0;
}
int RadioCommunication::setTxPower(int power)
{
  // Change receiver param first
  hc12.write(255); // let the drone know that we are sending parameters
  hc12.write(0);   // skip first parameter
  hc12.write(txpower_state);

  gpio_set_level(HC12_CMD_MODE, LOW);
  delay(40);
  // Then change transmitter param
  hc12.println((String) "AT+P" + (txpower_state + 1));
  Serial.println((String) "AT+P" + (txpower_state + 1));
  delay(20);
  gpio_set_level(HC12_CMD_MODE, HIGH);
  delay(80);
  return 0;
}
int RadioCommunication::setBaudRate(int baud)
{
  // Change receiver param first
  hc12.write(255); // let the drone know that we are sending parameters
  hc12.write(0);   // skip two parameters
  hc12.write(0);
  hc12.write(baud_state);

  gpio_set_level(HC12_CMD_MODE, LOW);
  delay(40);
  // Then change transmitter param
  hc12.println("AT+B" + (String)baud);
  delay(20);

  Serial.println("AT+B" + (String)baud);
  Serial.println("AT+B" + (String)(rc_baudrate[baud_state]));
  Serial.println((rc_baudrate[baud_state]));

  gpio_set_level(HC12_CMD_MODE, HIGH);
  delay(80);
  hc12.updateBaudRate(baud);
  return 0;
}

int RadioCommunication::sendBytes()
{
  send_packet[0] = DATA_BYTES; // sending mode

  send_packet[1] = button1.state;
  send_packet[2] = button2.state;
  send_packet[3] = button3.state;
  send_packet[4] = button4.state;

  send_packet[5] = pot_throttle & 0xff; // mask the lower 8 bits
  send_packet[6] = pot_throttle >> 8; // shift higher 8 bits
  send_packet[7] = pot_yaw & 0xff;
  send_packet[8] = pot_yaw >> 8;
  send_packet[9] = pot_pitch & 0xff;
  send_packet[10] = pot_pitch >> 8;
  send_packet[11] = pot_roll & 0xff;
  send_packet[12] = pot_roll >> 8;

  //checksum
  local_checksum = 0;
  for (int i = 1; i < 13; i++)
  {
    local_checksum += send_packet[i];
  }
  send_packet[13] = local_checksum & 0xff;
  send_packet[14] = local_checksum >> 8;

  for (int i = 0; i < 15; i++)
  {
    hc12.write(send_packet[i]);
  }
  hc12.flush();
  return 1;
}

int RadioCommunication::parseIncomingBytes()
{
  if (hc12.available() > 0)
  {
    for (int i = 0; i < hc12.available(); i++)
    {
      receive_packet[i] = hc12.read();
    }
    return 1;
  }
  else
  {
    return 0;
  }
}