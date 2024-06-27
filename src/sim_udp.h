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
#include <pthread.h>



#define UDP_IP        0x00
#define UDP_RXPORT    0x04
#define UDP_TXPORT    0x08
#define UDP_RXCTRL    0x0C
#define UDP_TXCTRL    0x10
#define UDP_RXSTATUS  0x14
#define UDP_TXSTATUS  0x18
#define UDP_RXSIZE    0x1C
#define UDP_TXSIZE    0x20
#define UDP_RXFIFO    0x30
#define UDP_TXFIFO    0x130


typedef struct {
  int sockfd;
  pthread_t thread_id;
  struct sockaddr_in client_addr;
  struct sockaddr_in host_addr;
} UDPRx;

class sim_udp_t : public abstract_device_t {
public:
  sim_udp_t(abstract_interrupt_controller_t *intctrl, reg_t int_id) :
    ie(0), ip(0), txctrl(0), rxctrl(0), div(0), interrupt_id(int_id), intctrl(intctrl) {

  }

  bool load(reg_t addr, size_t len, uint8_t* bytes) override;
  bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
  void tick(reg_t UNUSED rtc_ticks) override;

private:
  std::queue<uint8_t> rx_fifo;
  
  uint32_t ie;
  uint32_t ip;
  uint32_t txctrl;
  uint32_t rxctrl;

  uint8_t tx_fifo[256];

  uint32_t reg_rx_size;
  uint32_t reg_tx_size;
  
  uint32_t div;
  reg_t interrupt_id;
  abstract_interrupt_controller_t *intctrl;

  uint32_t reg_addr_ip;
  uint32_t reg_addr_rx_port;
  uint32_t reg_addr_tx_port;

  UDPRx *udp;

  void write_udp_tx();
  void create_udp_tx();

  uint32_t read_rxfifo() {
    if (!rx_fifo.size()) return 0x80000000;
    uint8_t r = rx_fifo.front();
    rx_fifo.pop();
    update_interrupts();
    return r;
  }

  void update_interrupts() {
    int cond = 0;
    if ((ie) ||
        ((ie) && rx_fifo.size())) {
      cond = 1;
    }
    intctrl->set_interrupt_level(interrupt_id, (cond) ? 1 : 0);
  }
};

#endif  // _SIM_UDP_H
