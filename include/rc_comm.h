#ifndef RC_COMM_H
#define RC_COMM_H


class RadioCommunication
{
private:
public:
  int setChannel(int channel = 1);
  int setTxPower(int power = 8);
  int setBaudRate(int baud = 4800);
  int transmit(bool enabled = true);
  int receive();
};



#endif
