#include <rc_comm.h>
#include <globals.h>

const float rc_txpower[8] = {0.8, 1.6, 3.2, 6.3, 12, 25, 50, 100};
const uint32_t rc_baudrate[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

uint8_t send_packet[16];
uint8_t receive_packet[8];

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
  HC12.write((uint8_t)255); // let the drone know that we are sending parameters
  HC12.write((uint8_t)channel);

  digitalWrite(hc_set, LOW);
  delay(40);
  // Then change transmitter param
  HC12.println(set_channel);
  Serial.begin(9600);
  delay(100);
  Serial.println(set_channel);
  Serial.end();
  delay(20);
  digitalWrite(hc_set, HIGH);
  delay(80);
  return 0;
}
int RadioCommunication::setTxPower(int power)
{
  // Change receiver param first
  HC12.write((uint8_t)255); // let the drone know that we are sending parameters
  HC12.write((uint8_t)0);   // skip first parameter
  HC12.write(txpower_state);

  digitalWrite(hc_set, LOW);
  delay(40);
  // Then change transmitter param
  HC12.println((String) "AT+P" + (txpower_state + 1));
  Serial.begin(9600);
  delay(100);
  Serial.println((String) "AT+P" + (txpower_state + 1));
  Serial.end();
  delay(20);
  digitalWrite(hc_set, HIGH);
  delay(80);
  return 0;
}
int RadioCommunication::setBaudRate(int baud)
{
  // Change receiver param first
  HC12.write((uint8_t)255); // let the drone know that we are sending parameters
  HC12.write((uint8_t)0);   // skip two parameters
  HC12.write((uint8_t)0);
  HC12.write(baud_state);

  digitalWrite(hc_set, LOW);
  delay(40);
  // Then change transmitter param
  HC12.println("AT+B" + (String)baud);
  delay(20);
  Serial.begin(9600);
  delay(100);
  Serial.println("AT+B" + (String)baud);
  Serial.println("AT+B" + (String)(rc_baudrate[baud_state]));
  Serial.println((rc_baudrate[baud_state]));
  Serial.end();

  digitalWrite(hc_set, HIGH);
  delay(80);

  HC12.end();
  HC12.begin(baud);
  delay(50);
  return 0;
}

int RadioCommunication::transmit(bool enabled)
{
  send_packet[0] = button1;
  send_packet[1] = button2;
  send_packet[2] = button3;
  send_packet[3] = button4;

  send_packet[4] = pot_throttle;
  send_packet[5] = pot_yaw;
  send_packet[6] = pot_pitch;
  send_packet[7] = pot_roll;

  //checksum (average number of all values)
  uint16_t local_check = (send_packet[0] + send_packet[1] + send_packet[2] + send_packet[3] + send_packet[4] + send_packet[5] + send_packet[6] + send_packet[7])/8;
  send_packet[8] = (uint8_t)local_check;
  Serial.begin(9600);
  delay(100);
  Serial.println(local_check);
  Serial.end();

  for (int j = 0; j < 9 && enabled; j++)
  {
    HC12.write(send_packet[j]);
  }
  HC12.flush();
  return 0;
}

int RadioCommunication::receive()
{
  if (HC12.available() > 0)
  {
    steer_correction = HC12.read();

    for (int i = 0; i < HC12.available(); i++)
    {
      //receive_packet[i] = HC12.read();
      steer_correction = HC12.read();
      digitalWrite(led_red, LOW);
      digitalWrite(led_green, HIGH);
    }
    return 1;
  }
  else
  {
    digitalWrite(led_green, LOW);
    digitalWrite(led_red, HIGH);
    return 0;
  }
}