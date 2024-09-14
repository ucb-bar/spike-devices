#ifndef _SIM_UDP_H
#define _SIM_UDP_H

#include <riscv/abstract_device.h>
#include <riscv/dts.h>
#include <riscv/sim.h>
#include <fesvr/term.h>
#include <fdt/libfdt.h>



#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#define UDP_BASE          0x10001000
#define UDP_SIZE          0x1000
    
#define UDP_RXIP          0x00
#define UDP_TXIP          0x04
#define UDP_RXPORT        0x08
#define UDP_TXPORT        0x0C
#define UDP_CTRL          0x10
#define UDP_STATUS        0x14
#define UDP_RXSIZE        0x18
#define UDP_TXSIZE        0x1C

#define UDP_RXFIFO_DATA     0x20
#define UDP_RXFIFO_VALID    0x24
#define UDP_RXFIFO_READY    0x28

#define UDP_TXFIFO_DATA     0x30
#define UDP_TXFIFO_VALID    0x34
#define UDP_TXFIFO_READY    0x38

#define UDP_RX_STATUS       0x40
#define UDP_TX_STATUS       0x44


typedef struct {
  int sockfd;
  pthread_t thread_id;
  struct sockaddr_in tx_addr;
  struct sockaddr_in rx_addr;
} UDPSocket;

class sim_udp_t : public abstract_device_t {
public:

  sim_udp_t(abstract_interrupt_controller_t *intctrl, reg_t int_id);
  bool load(reg_t addr, size_t len, uint8_t* bytes) override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
  void tick(reg_t UNUSED rtc_ticks) override;

private:
  uint32_t reg_ctrl;
  uint32_t reg_status;
  uint32_t reg_rxsize;
  uint32_t reg_txsize;
  uint8_t rx_buffer[256];
  uint8_t tx_buffer[256];
  std::queue<uint8_t> rx_fifo;
  std::mutex rx_fifo_mutex;
  std::queue<uint8_t> tx_fifo;
  std::mutex tx_fifo_mutex;
  uint8_t rx_fifo_to_pop;
  uint8_t tx_fifo_to_push;
  
  uint32_t ie;
  uint32_t ip;
  
  reg_t interrupt_id;
  abstract_interrupt_controller_t *intctrl;

  UDPSocket udp;

  uint8_t reg_rx_status;
  uint8_t reg_tx_status;

  uint8_t enabled;
  uint8_t rx_flag;
  uint8_t tx_flag;

  void udp_create_socket();
  void udp_enable();
  void udp_set_rx_flag();
  void udp_set_tx_flag();
  void udp_receive();
  void udp_send();
};

#endif  // _SIM_UDP_H
