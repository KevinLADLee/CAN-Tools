
#ifndef LINUX_CAN_TEST_CANRECEIVE_H
#define LINUX_CAN_TEST_CANRECEIVE_H

#include <string>
#include <cstring>
#include <iostream>
#include <memory>

#include <stdint.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

constexpr int canfd_on = 1;

class CanDevice{

 public:
  CanDevice();

  CanDevice(std::string can_interface);

  ~CanDevice();

  bool InitSocketCan();

  bool InitSocketCanWithFilter(struct can_filter * filter, unsigned int num);

  int RecvMsg(struct canfd_frame * frame_ptr);

  int Send(unsigned int can_id, const uint8_t *buffer);

  int Send(const struct can_frame & frame);

 private:

  bool Init();

  bool Config();

  bool Bind();

 private:
  std::string can_interface_;
  sockaddr_can can_addr_;
  ifreq ifreq_;
  int sockfd_;
  int rc_;
  int max_dlen_;

  bool disable_filter_; // Used in applications only SEND CAN frames
  bool loopback_; // Local loop back
  bool enable_timeout_;
  struct timeval timeout_config_;

  fd_set rdfs_;
  struct msghdr msg_;
  struct cmsghdr *cmsg_;
  struct canfd_frame frame_;
  struct iovec iov_;
  char ctrlmsg_[CMSG_SPACE(sizeof(struct timeval) + 3*sizeof(struct timespec) + sizeof(__u32))];


};

#endif //LINUX_CAN_TEST_CANRECEIVE_H
