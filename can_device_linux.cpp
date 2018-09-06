

#include "can_device_linux.h"

CanDevice::CanDevice() : can_interface_("can0"),
                         disable_filter_(false),
                         loopback_(true),
                         enable_timeout_(false){
  timeout_config_.tv_sec = 0;
  timeout_config_.tv_usec = 0;
}

CanDevice::CanDevice(std::string can_interface) :
                     can_interface_(can_interface),
                     disable_filter_(false),
                     loopback_(true) {}

CanDevice::~CanDevice() {
  close(sockfd_);
}

bool CanDevice::InitSocketCan() {

  if(!Init()){
    std::cout << "Init faild" << std::endl;
    return false;
  }

  if(!Config()){
    std::cout << "Configure failed" << std::endl;
    return false;
  }

  return Bind();
}


bool CanDevice::InitSocketCanWithFilter(struct can_filter * filter, unsigned int num) {

  if(!Init()){
    return false;
  }
  setsockopt(sockfd_, SOL_CAN_RAW, CAN_RAW_FILTER,
             filter, num * sizeof(struct can_filter));
  Config();
  return Bind();
}

bool CanDevice::Init() {
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
  if(!can_addr_.can_ifindex){
    std::cout << "invalid bridge interface" << std::endl;
    return false;
  }

  iov_.iov_base = &frame_;
  msg_.msg_name = &can_addr_;
  msg_.msg_iov = &iov_;
  msg_.msg_iovlen = 1;
  msg_.msg_control = &ctrlmsg_;

  return true;
}


bool CanDevice::Config(){
  if(loopback_ == 0){
    setsockopt(sockfd_, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback_, sizeof(loopback_));
  }
  if(disable_filter_){
    setsockopt(sockfd_, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  }
  return true;
}

bool CanDevice::Bind() {
  setsockopt(sockfd_, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));
  rc_ = bind(sockfd_, reinterpret_cast<struct sockaddr *>(&can_addr_), sizeof(can_addr_));
  if (rc_ < 0) {
    std::cout << "bind failed" << std::endl;
    return false;
  }
  return true;
}

int CanDevice::RecvMsg(struct canfd_frame * frame_ptr) {
  FD_ZERO(&rdfs_);
  FD_SET(sockfd_, &rdfs_);

  if (FD_ISSET(sockfd_, &rdfs_)) {
    int idx;
    iov_.iov_len = sizeof(frame_);
    msg_.msg_namelen = sizeof(can_addr_);
    msg_.msg_controllen = sizeof(ctrlmsg_);
    msg_.msg_flags = 0;

    int nbyte = recvmsg(sockfd_, &msg_, 0);

    if(nbyte < 0){
      if ((errno == ENETDOWN)) {
        std::cerr << can_interface_ << ": interface down" << std::endl;
        return 0;
      }
      return -1;
    }

    if((size_t)nbyte == CAN_MTU){
      max_dlen_ = CAN_MAX_DLEN;
    } else if((size_t)nbyte == CANFD_MTU){
      max_dlen_ = CANFD_MAX_DLEN;
    } else{
      std::cerr << "read: incomplete CAN Frame" << std::endl;
    }

    *frame_ptr = frame_;
    memcpy(frame_ptr->data, frame_.data, max_dlen_);
    return max_dlen_;
  }
  return -1;

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
