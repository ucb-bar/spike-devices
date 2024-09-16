#include "sim_udp.h"

// constructor
sim_udp_t::sim_udp_t(abstract_interrupt_controller_t *intctrl, reg_t int_id) {
  this->interrupt_id = int_id;
  this->intctrl = intctrl;

  this->udp.rx_addr.sin_family = AF_INET;
  this->udp.tx_addr.sin_family = AF_INET;

  this->reg_ctrl = 0;
  this->reg_status = 0;
  this->reg_rxsize = 0;
  this->reg_txsize = 0;

  this->enabled = 0;
  this->rx_flag = 0;
  this->tx_flag = 0;


  // Create socket file descriptor
  this->udp.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (this->udp.sockfd < 0) {
    printf("<SimUDP> [ERROR]: socket creation failed\n");
    exit(1);
  }

  std::thread rx_thread(&sim_udp_t::udp_receive_handler, this);
  std::thread tx_thread(&sim_udp_t::udp_send_handler, this);

  rx_thread.detach();
  tx_thread.detach();

  this->reg_rx_status = 0x00;
  this->reg_tx_status = 0x00;
}

void sim_udp_t::udp_receive_handler() {
  printf("<SimUDP> [INFO]: UDP Rx thread started\n");

  while (1) {
    if (this->rx_flag) {
      int n = recvfrom(this->udp.sockfd, (void *)this->rx_buffer, this->reg_rxsize, MSG_WAITALL, NULL, 0);
      
      if (n) {
        rx_fifo_mutex.lock();
        for (int i = 0; i < n; i += 1) {
          this->rx_fifo.push(this->rx_buffer[i]);
        }
        rx_fifo_mutex.unlock();

        this->reg_rxsize = n;
        this->reg_rx_status = 0x01;
        
        this->rx_flag = 0;
      }
    }
  }
}

void sim_udp_t::udp_send_handler() {

  printf("<SimUDP> [INFO]: UDP Tx thread started\n");

  while (1) {
    if (this->tx_flag && this->tx_fifo.size() >= this->reg_txsize) {
      printf("<SimUDP> [INFO]: UDP Tx to (%s, %d) with data size: %d\n", 
        inet_ntoa(this->udp.tx_addr.sin_addr),
        ntohs(this->udp.tx_addr.sin_port),
        this->reg_txsize
      );

      for (int i = 0; i < this->reg_txsize; i += 1) {
        this->tx_buffer[i] = this->tx_fifo.front();
        this->tx_fifo.pop();
      }

      sendto(
        this->udp.sockfd,
        (uint8_t *)this->tx_buffer, this->reg_txsize, 0,
        (const struct sockaddr *)&this->udp.tx_addr, sizeof(this->udp.tx_addr)
      );

      this->reg_tx_status = 0x01;
      this->tx_flag = 0;
    }
  }
}

void sim_udp_t::udp_enable() {
  if (this->enabled) {
    return;
  }

  ssize_t status = bind(this->udp.sockfd, (const struct sockaddr *)&this->udp.rx_addr, sizeof(this->udp.rx_addr));
  if (status < 0) {
    printf("<SimUDP> [ERROR]: bind failed\n");
  }
  printf("<SimUDP> [INFO]: bind success\n");
  this->enabled = 1;

}

bool sim_udp_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  // if illegal address or length, return false
  if (addr >= 0x1000 || len > 4) return false;

  uint32_t r = 0;
  switch (addr) {
    case UDP_RXFIFO_DATA:
      if (this->rx_fifo.size() == 0) {
        // set empty flag
        r = 0x80000000;
      }
      else {
        r = this->rx_fifo.front();
        this->rx_fifo.pop();
      }
      break;
    
    case UDP_RXFIFO_SIZE:
      r = this->rx_fifo.size();
      break;
    
    case UDP_TXFIFO_DATA:
      r = (this->tx_fifo.size() == FIFO_SIZE ? 0x80000000 : 0x00) | this->tx_fifo.back();
      break;
    
    case UDP_TXFIFO_SIZE:
      r = this->tx_fifo.size();
      break;
    
    case UDP_RX_STATUS:
      r = this->reg_rx_status;
      break;
    case UDP_TX_STATUS:
      r = this->reg_tx_status;
      break;
    default: printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
  memcpy(bytes, &r, len);
  return true;
}

bool sim_udp_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  // if illegal address or length, return false
  if (addr >= 0x1000 || len > 4) return false;
  
  switch (addr) {
    case UDP_RXIP:
      this->udp.rx_addr.sin_addr.s_addr = *((uint32_t *)bytes);
      return true;

    case UDP_TXIP:
      this->udp.tx_addr.sin_addr.s_addr = *((uint32_t *)bytes);
      return true;
    
    case UDP_RXPORT: 
      this->udp.rx_addr.sin_port = *((uint16_t *)bytes);
      return true;
    
    case UDP_TXPORT:
      this->udp.tx_addr.sin_port = *((uint16_t *)bytes);
      return true;
    
    case UDP_CTRL:
      if (READ_BITS(bytes[0], (1 << 0))) {
        this->udp_enable();
      }
      if (READ_BITS(bytes[0], (1 << 1))) {
        this->rx_flag = 1;
      }
      if (READ_BITS(bytes[0], (1 << 2))) {
        this->tx_flag = 1;
      }
      return true;
    
    case UDP_RXSIZE:  
      this->reg_rxsize       = *((uint32_t *)bytes); 
      return true;
    
    case UDP_TXSIZE: 
      this->reg_txsize       = *((uint32_t *)bytes); 
      return true;
    
    case UDP_TXFIFO_DATA:
      this->tx_fifo.push(bytes[0]);
      return true;
    
    case UDP_RX_STATUS:
      this->reg_rx_status = 0x00;
      return true;
    
    case UDP_TX_STATUS:
      this->reg_tx_status = 0x00;
      return true;

    default: printf("STORE -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
}

void sim_udp_t::tick(reg_t UNUSED rtc_ticks) {
  // if (rx_fifo.size() >= 1) return;
  // int rc = canonical_terminal_t::read();
  // if (rc < 0) return;
  // rx_fifo.push((uint8_t)rc);
  // update_interrupts();

}

int SimUDP_parseFDT(const void *fdt, reg_t *address,
			  const char *compatible) {
  int nodeoffset, len, rc;
  const fdt32_t *reg_p;

  nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
  if (nodeoffset < 0)
    return nodeoffset;

  rc = fdt_get_node_addr_size(fdt, nodeoffset, address, NULL, "reg");
  if (rc < 0 || !address)
    return -ENODEV;

  return 0;
}


// This function parses an instance of this device from the FDT
// An FDT node for a device should encode, at minimum, the base address for the device
sim_udp_t* SimUDP_parseFromFDT(const void* fdt, const sim_t* sim, reg_t* base, std::vector<std::string> sargs) {
  if (SimUDP_parseFDT(fdt, base, "ucbbar,sim_udp") == 0) {
    printf("Found SimUDP at 0x%lx\n", *base);
    return new sim_udp_t(sim->get_intctrl(), 1);
  } else {
    return nullptr;
  }
}

std::string SimUDP_generateDTS(const sim_t* sim, const std::vector<std::string>& args) {
  std::stringstream s;
  s << std::hex
    << "    udp: udp@" << UDP_BASE << " {\n"
       "      compatible = \"ucbbar,sim_udp\";\n"
       "      interrupt-parent = <&PLIC>;\n"
       "      interrupts = <" << std::dec << 2;
  reg_t blkdevbs = UDP_BASE;
  reg_t blkdevsz = UDP_SIZE;
  s << std::hex << ">;\n"
       "      reg = <0x" << (blkdevbs >> 32) << " 0x" << (blkdevbs & (uint32_t)-1) <<
                   " 0x" << (blkdevsz >> 32) << " 0x" << (blkdevsz & (uint32_t)-1) << ">;\n"
       "    };\n";
  return s.str();
}

REGISTER_DEVICE(sim_udp, SimUDP_parseFromFDT, SimUDP_generateDTS);
