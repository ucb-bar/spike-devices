#include "sim_udp.h"


void sim_udp_t::write_udp_tx() {
  printf("<SimUDP> [INFO]: Writing UDP TX with DMA pointer: 0x%lx, DMA size: %d\n", tx_fifo, reg_tx_size);
  printf("addr: %x, port: %x\n", reg_addr_ip, reg_addr_tx_port);
  sendto(udp->sockfd, (uint8_t *)tx_fifo, reg_tx_size, 0, (const struct sockaddr *)&udp->host_addr, sizeof(udp->host_addr));
}

void sim_udp_t::create_udp_tx() {
  udp = (UDPRx *)malloc(sizeof(UDPRx));

  memset(&udp->client_addr, 0, sizeof(udp->client_addr));
  memset(&udp->host_addr, 0, sizeof(udp->host_addr));

  // host ip
  udp->host_addr.sin_family = AF_INET;
  udp->host_addr.sin_addr.s_addr = htonl(reg_addr_ip);
  udp->host_addr.sin_port = htons(reg_addr_tx_port);


  // Create socket file descriptor
  if ((udp->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("<SimUDP> [ERROR]: socket creation failed\n");
  }

  // if (bind(udp->sockfd, (const struct sockaddr *)&udp->host_addr, sizeof(udp->host_addr)) < 0) {
  //   printf("<SimUDP> [ERROR]: bind failed\n");
  // }

  // Create a thread running the receive() function

  printf("<SimUDP> [INFO]: Tx thread created, thread ID: %ld\n", (long)udp->thread_id);

}


bool sim_udp_t::load(reg_t addr, size_t len, uint8_t* bytes) {
  if (addr >= 0x1000 || len > 4) return false;
  uint32_t r = 0;
  switch (addr) {
    case UDP_IP:      r = reg_addr_ip;      break;
    case UDP_RXPORT:  r = reg_addr_rx_port; break;
    case UDP_TXPORT:  r = reg_addr_tx_port; break;
    case UDP_RXSIZE:  r = reg_rx_size;      break;
    default: printf("LOAD -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
  memcpy(bytes, &r, len);
  return true;
}


bool sim_udp_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
  printf("STORE -- ADDR=0x%lx LEN=%lu\n", addr, len);
  
  if (addr >= 0x1000 || len > 4) return false;
  
  if (addr >= UDP_TXFIFO) {
    tx_fifo[addr - UDP_TXFIFO] = bytes[0];
    return true;
  }
  switch (addr) {
    case UDP_IP:      reg_addr_ip       = *((uint32_t *)bytes); return true;
    case UDP_RXPORT:  reg_addr_rx_port  = *((uint16_t *)bytes); return true;
    case UDP_TXPORT:  reg_addr_tx_port  = *((uint16_t *)bytes); return true;
    case UDP_TXCTRL:
      if (bytes[0] == 1) write_udp_tx();
      if (bytes[0] == 2) create_udp_tx();
      return true;
    case UDP_TXSIZE:  reg_tx_size       = *((uint32_t *)bytes); return true;
    default: printf("STORE -- ADDR=0x%lx LEN=%lu\n", addr, len); abort();
  }
}

void sim_udp_t::tick(reg_t UNUSED rtc_ticks) {
  if (rx_fifo.size() >= 1) return;
  int rc = canonical_terminal_t::read();
  if (rc < 0) return;
  rx_fifo.push((uint8_t)rc);
  update_interrupts();
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
