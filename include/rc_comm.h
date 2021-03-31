#ifndef RC_COMM_H
#define RC_COMM_H

#include <Arduino.h>

class HC12
{
public:
  typedef enum ACK
  {
    OK = 0x00,           // Operation succeeded. No Error.
    UNKOWN_ERROR = 0x01, // Device communication failed for unknown reason
    INVALID_CRC = 0x02,  // calculated a different CRC / data transmission from Master failed
    VERIFY_ERROR = 0x03, // Interface did a successful write operation, but the read back data did not match
    CMD_FAILED = 0x04,   // Device communication failed
    CMD_INVALID = 0x05,
    PARAM_INVALID = 0x06,
    NOT_READY = 0x07,
    TIMEOUT = 0x08
  } rc_ack_t;

  enum protocol
  {
    TRANSMITTER_CMD = 0x54, // 'T'
    RECEIVER_ANSWER = 0x48, // 'H'
  };

  enum MODE
  {
    NORMAL = 0x4e,      // 'N'
    SET_CHANNEL = 0x43, // 'C'
    SET_TXPOWER = 0x50, // 'P'
    SET_BAUD = 0x42,    // 'B'
    SET_MODE = 0x4d,    // 'M'
    SET_DEFAULT = 0x44  // 'D'
  };

  rc_ack_t setChannel(int channel = 1, bool set_receiver = true);
  rc_ack_t setTxPower(int power = 8, bool set_receiver = true);
  rc_ack_t setBaudRate(int baud = 3, bool set_receiver = true); // 9600bps
  rc_ack_t setTransmitMode(int new_mode, bool set_receiver = true);
  rc_ack_t setAllDefault(bool set_receiver = true);
  void getVersion();
  void sleep();
  void wakeUp();
  rc_ack_t sendData(int extra_data = 0);
  rc_ack_t readData();

  bool is_connected = false;
  bool waiting_for_response = true;

  float packets_per_sec = 0;
  int packets_lost = 0;
  int packets_lost_total = 0;
  int response_time = 0;
  const int MAX_PACKETS_LOST = 3;
  uint64_t PACKET_TIMEOUT = 500; // 500ms
  
private:
  rc_ack_t setReceiverSettings(enum HC12::MODE param, byte value = 0);
};

extern uint8_t last_channel;
extern uint8_t last_baud;
extern uint8_t last_txpower;
extern uint8_t last_txmode;

extern uint8_t send_packet[];
extern uint8_t receive_packet[];

extern const float rc_txpower[];
extern const uint32_t rc_baudrate[];

#endif
