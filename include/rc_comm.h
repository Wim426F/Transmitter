#ifndef RC_COMM_H
#define RC_COMM_H

#include <Arduino.h>

extern const uint8_t DATA_BYTES;
extern const uint8_t CONFIG_BYTES;

class RadioCommunication
{
private:
public:
  int setChannel(int channel = 1);
  int setTxPower(int power = 8);
  int setBaudRate(int baud = 4800);
  int sendBytes();
  int parseIncomingBytes();
};



#endif
