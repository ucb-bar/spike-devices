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



#define UDP_RXIP      0x00
#define UDP_TXIP      0x04
#define UDP_RXPORT    0x08
#define UDP_TXPORT    0x0C
#define UDP_CTRL      0x10
#define UDP_STATUS    0x14
#define UDP_RXSIZE    0x18
#define UDP_TXSIZE    0x1C
#define UDP_RXFIFO    0x30
#define UDP_TXFIFO    0x130


typedef struct {
  int sockfd;
  pthread_t thread_id;
  struct sockaddr_in tx_addr;
  struct sockaddr_in rx_addr;
} UDPSocket;

class sim_udp_t : public abstract_device_t {
public:
  sim_udp_t(abstract_interrupt_controller_t *intctrl, reg_t int_id) {
    this->interrupt_id = int_id;
    this->intctrl = intctrl;
  
    this->udp.rx_addr.sin_family = AF_INET;
    this->udp.tx_addr.sin_family = AF_INET;

    this->reg_ctrl = 0;
    this->reg_status = 0;
    this->reg_rxsize = 0;
    this->reg_txsize = 0;

    this->rx_enabled = 0;
    this->tx_enabled = 0;
    
  }

  bool load(reg_t addr, size_t len, uint8_t* bytes) override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
  void tick(reg_t UNUSED rtc_ticks) override;

private:
  uint32_t reg_ctrl;
  uint32_t reg_status;
  uint32_t reg_rxsize;
  uint32_t reg_txsize;

  // std::queue<uint8_t> rx_fifo;
  uint8_t rx_fifo[256];
  uint8_t tx_fifo[256];
  
  uint32_t ie;
  uint32_t ip;
  
  reg_t interrupt_id;
  abstract_interrupt_controller_t *intctrl;

  UDPSocket udp;
  // UDPSocket *udp_rx;

  uint8_t rx_enabled;
  uint8_t tx_enabled;

  void udp_create_socket();
  void udp_enable_rx();
  void udp_enable_tx();
  void udp_write();
  void udp_receive();

  // uint32_t read_rxfifo() {
  //   if (!rx_fifo.size()) return 0x80000000;
  //   uint8_t r = rx_fifo.front();
  //   rx_fifo.pop();
  //   update_interrupts();
  //   return r;
  // }

  // void update_interrupts() {
  //   int cond = 0;
  //   if ((ie) ||
  //       ((ie) && rx_fifo.size())) {
  //     cond = 1;
  //   }
  //   intctrl->set_interrupt_level(interrupt_id, (cond) ? 1 : 0);
  // }
};

#endif  // _SIM_UDP_H
