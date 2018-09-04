
#ifndef LINUX_CAN_TEST_CANRECEIVE_H
#define LINUX_CAN_TEST_CANRECEIVE_H

#include <string>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>


class CanDevice{
 public:
  CanDevice();

  CanDevice(std::string can_interface);

  bool InitSocketCan();

  bool InitSocketCanWithFilter(struct can_filter * filter);

  int Read(uint8_t * buffer);

  int Read(unsigned int & can_id, uint8_t * buffer);

  int Read(struct can_frame & frame);

  int Send(unsigned int can_id, const uint8_t *buffer);

  int Send(const struct can_frame & frame);

 private:
  std::string can_interface_;
  sockaddr_can can_addr_;
  ifreq ifreq_;
  int sockfd_;
  int rc_;

};

#endif //LINUX_CAN_TEST_CANRECEIVE_H
