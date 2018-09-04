

#include "can_device_linux.h"

CanDevice::CanDevice() {
 can_interface_ = "can0";
}

CanDevice::CanDevice(std::string can_interface) :
                     can_interface_(can_interface) {}

bool CanDevice::InitSocketCan() {
  sockfd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (sockfd_ < 0) {
    std::cout << "open error" << std::endl;
    return false;
  }

  std::strncpy(ifreq_.ifr_name, can_interface_.c_str(), IFNAMSIZ);
  if (ioctl(sockfd_, SIOCGIFINDEX, &ifreq_) == -1) {
    std::cout << "ioctl failed" << std::endl;
    return false;
  }

  can_addr_.can_family = AF_CAN;
  can_addr_.can_ifindex = ifreq_.ifr_ifindex;
  rc_ = bind(sockfd_, reinterpret_cast<struct sockaddr *>(&can_addr_), sizeof(can_addr_));
  if (rc_ == -1) {
    std::cout << "bind failed" << std::endl;
    return false;
  }

  return true;
}


bool CanDevice::InitSocketCanWithFilter(struct can_filter * filter) {
  sockfd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (sockfd_ < 0) {
    std::cout << "open error" << std::endl;
    return false;
  }

  rc_ = setsockopt(sockfd_, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &filter, sizeof(can_filter));
  if (rc_ == -1) {
    std::cout << "setsockopt can_raw_filter failed" << std::endl;
    return false;
  }

  std::strncpy(ifreq_.ifr_name, can_interface_.c_str(), IFNAMSIZ);
  if (ioctl(sockfd_, SIOCGIFINDEX, &ifreq_) == -1) {
    std::cout << "ioctl failed" << std::endl;
    return false;
  }

  can_addr_.can_family = AF_CAN;
  can_addr_.can_ifindex = ifreq_.ifr_ifindex;
  rc_ = bind(sockfd_, reinterpret_cast<struct sockaddr *>(&can_addr_), sizeof(can_addr_));
  if (rc_ == -1) {
    std::cout << "bind failed" << std::endl;
    return false;
  }
  return true;
}

int CanDevice::Read(uint8_t * buffer) {
  struct can_frame frame;
  read(sockfd_, &frame, CAN_MTU);
  memcpy(buffer, frame.data, 8);
  return frame.can_id;
}


int CanDevice::Read(unsigned int & can_id, uint8_t * buffer) {
  struct can_frame frame;
  read(sockfd_, &frame, CAN_MTU);
  can_id = frame.can_id;
  memcpy(buffer, frame.data, 8);
  return 0;
}

int CanDevice::Read(struct can_frame &frame) {
  return read(sockfd_, &frame, CAN_MTU);
}

int CanDevice::Send(unsigned int can_id, const uint8_t *buffer) {
  struct can_frame tx_frame;
  memcpy(tx_frame.data, buffer, 8);
  tx_frame.can_id = can_id;
  write(sockfd_, &tx_frame, CAN_MTU);
  return 0;
}

int CanDevice::Send(const struct can_frame &frame) {
  return write(sockfd_, &frame, CAN_MTU);;
}
