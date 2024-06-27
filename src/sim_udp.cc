#include "sim_udp.h"

void sim_udp_t::udp_receive() {
  // UDPSocket *udp = (UDPSocket *)udp_ptr;

  printf("<SimUDP> [INFO]: UDP Rx thread started\n");

  socklen_t len = sizeof(this->udp.rx_addr);

  printf("here\n");

  // create performance counter
  // struct timespec start, end;

  // clock_gettime(CLOCK_MONOTONIC, &start);

  while (1) {
    // int n = recvfrom(udp->sockfd, (void *)rx_buffer, sizeof(float) * N_OBSERVATIONS, MSG_WAITALL, (struct sockaddr *)&udp->robot_addr, &len);
    
    // if (n == sizeof(float) * N_OBSERVATIONS) {
    //   for (int i = 0; i < N_OBSERVATIONS; i++) {
    //     udp->obs[i] = rx_buffer[i];
    //   }
    // }
    // else {
    //   // sleep for 1ms
    //   usleep(1000);
    // }

    
    if (this->rx_enabled) {
      socklen_t len = sizeof(this->udp.rx_addr);
      int n = recvfrom(this->udp.sockfd, (void *)this->rx_fifo, 512, MSG_WAITALL, (struct sockaddr *)&this->udp.rx_addr, &len);
      
      if (n) {
        printf("<SimUDP> [INFO]: UDP Rx from (%s, %d) with data size: %d\n", 
          inet_ntoa(this->udp.rx_addr.sin_addr),
          ntohs(this->udp.rx_addr.sin_port),
          n
          );
      }
    }
  }
  //   // perfromance
  //   clock_gettime(CLOCK_MONOTONIC, &end);


  //   long seconds = end.tv_sec - start.tv_sec;
  //   long ns = end.tv_nsec - start.tv_nsec;

  //   // Correct for rollover
  //   if (start.tv_nsec > end.tv_nsec) {
  //     --seconds;
  //     ns += 1000000000;
  //   }

  //   clock_gettime(CLOCK_MONOTONIC, &start);

  //   double freq = 1.0 / (seconds + ns / 1000000000.0);

  //   // printf("Frequency: %f\n", freq);
  // }
}

void sim_udp_t::udp_create_socket() {
  // Create socket file descriptor
  if ((this->udp.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("<SimUDP> [ERROR]: socket creation failed\n");
  }

  printf("<SimUDP> [INFO]: socket created\n");
}

void sim_udp_t::udp_enable_rx() {
  if (bind(udp.sockfd, (const struct sockaddr *)&udp.rx_addr, sizeof(udp.rx_addr)) < 0) {
    printf("<SimUDP> [ERROR]: bind failed\n");
  }

  std::thread t1(&sim_udp_t::udp_receive, this);
  t1.detach();

  // // Create a thread running the receive() function
  // if(pthread_create(&this->udp.thread_id, NULL, this->udp_receive, (void *)this->udp) != 0) {
  //   perror("[ERROR]: Failed to create receive thread");
  // }

  this->rx_enabled = 1;


  printf("<SimUDP> [INFO]: Rx thread created, thread ID: %ld\n", (long)this->udp.thread_id);
}

void sim_udp_t::udp_enable_tx() {


  this->tx_enabled = 1;
  printf("<SimUDP> [INFO]: Tx thread created, thread ID: %ld\n", (long)this->udp.thread_id);
}

void sim_udp_t::udp_write() {

  if (!this->tx_enabled) {
    return;
  }

  printf("<SimUDP> [INFO]: UDP Tx to (%s, %d) with data size: %d\n", 
      inet_ntoa(this->udp.tx_addr.sin_addr),
      ntohs(this->udp.tx_addr.sin_port),
      this->reg_txsize
      );
  
  sendto(
    this->udp.sockfd,
    (uint8_t *)this->tx_fifo, this->reg_txsize, 0,
    (const struct sockaddr *)&this->udp.tx_addr, sizeof(this->udp.tx_addr)
    );
}

bool sim_udp_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len);
  if (addr >= 0x1000 || len > 4) return false;
  uint32_t r = 0;
  switch (addr) {
    case UDP_RXIP:      r = this->udp.rx_addr.sin_addr.s_addr;  break;
    case UDP_TXIP:      r = this->udp.tx_addr.sin_addr.s_addr;  break;
    case UDP_RXPORT:    r = this->udp.rx_addr.sin_port;         break;
    case UDP_TXPORT:    r = this->udp.tx_addr.sin_port;         break;
    case UDP_CTRL:      r = this->reg_ctrl;         break;
    case UDP_STATUS:    r = this->reg_status;       break;
    case UDP_RXSIZE:    r = this->reg_rxsize;       break;
    case UDP_TXSIZE:    r = this->reg_txsize;       break;
    default: printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
  memcpy(bytes, &r, len);
  return true;
}


bool sim_udp_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  printf("STORE -- ADDR=0x%lx LEN=%lu DATA=%lx\n", addr, len, *(uint32_t *)bytes);
  
  if (addr >= 0x1000 || len > 4) return false;
  
  if (addr >= UDP_TXFIFO) {
    tx_fifo[addr - UDP_TXFIFO] = bytes[0];
    return true;
  }
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
      if (bytes[0] & (1 << 0)) {
        this->udp_create_socket();
      }
      if (bytes[0] & (1 << 1)) {
        this->udp_enable_rx();
      }
      if (bytes[0] & (1 << 2)) {
        this->udp_enable_tx();
      }
      if (bytes[0] & (1 << 3)) {
        this->udp_write();
      }
      return true;
    case UDP_TXSIZE:  this->reg_txsize       = *((uint32_t *)bytes); return true;
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
    printf("Found SimUDP at %lx\n", *base);
    return new sim_udp_t(sim->get_intctrl(), 1);
  } else {
    return nullptr;
  }
}

std::string SimUDP_generateDTS(const sim_t* sim) {
  std::stringstream s;
  s << std::hex
    << "    udp: udp@" << 0x10001000 << " {\n"
       "      compatible = \"ucbbar,sim_udp\";\n"
       "      interrupt-parent = <&PLIC>;\n"
       "      interrupts = <" << std::dec << 2;
  reg_t blkdevbs = 0x10001000;
  reg_t blkdevsz = 0x1000;
  s << std::hex << ">;\n"
       "      reg = <0x" << (blkdevbs >> 32) << " 0x" << (blkdevbs & (uint32_t)-1) <<
                   " 0x" << (blkdevsz >> 32) << " 0x" << (blkdevsz & (uint32_t)-1) << ">;\n"
       "    };\n";
  return s.str();
}

REGISTER_DEVICE(sim_udp, SimUDP_parseFromFDT, SimUDP_generateDTS);
