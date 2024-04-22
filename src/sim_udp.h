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



#define UART_TXFIFO (0x00)
#define UART_RXFIFO (0x04)
#define UART_TXCTRL (0x08)
#define UART_TXMARK (0x0a)
#define UART_RXCTRL (0x0c)
#define UART_RXMARK (0x0e)

#define ETH_ADDR_IP       0x50
#define ETH_ADDR_PORT     0x54


#define UART_IE     (0x10)
#define UART_IP     (0x14)
#define UART_DIV    (0x18)
#define UART_PARITY (0x1c)
#define UART_WIRE4  (0x20)
#define UART_EITHER8OR9 (0x24)

#define UART_GET_TXCNT(txctrl)   ((txctrl >> 16) & 0x7)
#define UART_GET_RXCNT(rxctrl)   ((rxctrl >> 16) & 0x7)
#define UART_RX_FIFO_SIZE (8)

#define UART_IE_TXWM       (1)
#define UART_IE_RXWM       (2)

#define UART_IP_TXWM       (1)
#define UART_IP_RXWM       (2)



typedef struct {
  int sockfd;
  pthread_t thread_id;
  struct sockaddr_in client_addr;
  struct sockaddr_in host_addr;
  float obs[1];
} UDPRx;

class sim_udp_t : public abstract_device_t {
public:
  sim_udp_t(abstract_interrupt_controller_t *intctrl, reg_t int_id) :
    ie(0), ip(0), txctrl(0), rxctrl(0), div(0), interrupt_id(int_id), intctrl(intctrl) {

  // host ip
  char *host_ip = "127.0.0.1";
  int host_port = 8000;


  udp = (UDPRx *)malloc(sizeof(UDPRx));

  memset(&udp->client_addr, 0, sizeof(udp->client_addr));
  memset(&udp->host_addr, 0, sizeof(udp->host_addr));
  memset(udp->obs, 0, 1 * sizeof(float));

  udp->host_addr.sin_family = AF_INET;
  udp->host_addr.sin_addr.s_addr = inet_addr(host_ip);
  udp->host_addr.sin_port = htons(host_port);


  // Create socket file descriptor
  if ((udp->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("[ERROR]: socket creation failed");
  }

  // if (bind(udp->sockfd, (const struct sockaddr *)&udp->host_addr, sizeof(udp->host_addr)) < 0 ) {
  //   printf("[ERROR]: bind failed");
  // }

  char tx_buffer[] = "Hello from the other side\n";

  sendto(udp->sockfd, tx_buffer, sizeof(tx_buffer), 0, (const struct sockaddr *)&udp->host_addr, sizeof(udp->host_addr));

  // Create a thread running the receive() function

  printf("[INFO]: Receive thread created, thread ID: %ld\n", (long)udp->thread_id);



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
  uint32_t div;
  reg_t interrupt_id;
  abstract_interrupt_controller_t *intctrl;

  uint32_t reg_addr_ip;
  uint32_t reg_addr_port;

  UDPRx *udp;

  uint64_t read_ip() {
    uint64_t ret = 0;
    uint64_t txcnt = UART_GET_TXCNT(txctrl);
    uint64_t rxcnt = UART_GET_RXCNT(rxctrl);
    if (txcnt != 0) ret |= UART_IP_TXWM;
    if (rx_fifo.size() > rxcnt) ret |= UART_IP_RXWM;
    return ret;
  }

  uint32_t read_rxfifo() {
    if (!rx_fifo.size()) return 0x80000000;
    uint8_t r = rx_fifo.front();
    rx_fifo.pop();
    update_interrupts();
    return r;
  }

  void update_interrupts() {
    int cond = 0;
    if ((ie & UART_IE_TXWM) ||
        ((ie & UART_IE_RXWM) && rx_fifo.size())) {
      cond = 1;
    }
    intctrl->set_interrupt_level(interrupt_id, (cond) ? 1 : 0);
  }
};

#endif  // _SIM_UDP_H
